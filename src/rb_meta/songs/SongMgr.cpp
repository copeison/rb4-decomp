// rb_meta/SongMgr.o (0xF29E40 to 0xF33742). The object holds SongMgr, the
// song file resolvers it installs, and SongMountHandle (0xF32060-0xF32960).
// The compiler's EASTL helpers sit at the end: the subtree releases of the
// cache maps (0xF32B90, 0xF32C30, 0xF32C80), the key insertions of the maps
// (0xF32FD0-0xF33719, with the generic copies shared with other objects
// such as 0x4BA40 and 0x961F70) and SongMountCompletedMsg's constructor
// (0xF32CF0). The static initializer is at 0xF33720. IDA leaves most of the
// object undefined; it is read from the raw code.
#include "rb_meta/songs/SongMgr.h"

#include <cstring>
#include <new>

#include "audio/core/resources/MoggMusicResource.h"
#include "audio/core/resources/MoggResource.h"
#include "entity/core/GameObject.h"
#include "entity/resources/Resource.h"
#include "math/random/Rand.h"
#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "os/system/System.h"
#include "persistence/profiles/Profile.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_meta/songs/SongMetadata.h"
#include "rb_meta/songs/SongMetadataResource.h"
#include "utl/files/FileUtl.h"
#include "utl/streams/RevisionStream.h"
#include "utl/text/MakeString.h"
#include "utl/time/TimeMgr.h"

// --- Declared from another rb_meta object -------------------------------
// The entitlement manager: an rb_meta object after DialogMgrCom.o that the
// map does not have. Its factory (0xAE25D0) sets the pointer and installs
// a content filter with SongMgr::SetContentFilter. Only what the paygated
// folders need is declared; the names are not in the reference map and
// are weak.
class RBEntitlementMgr {
public:
    // Whether the entitlement is owned. Takes the manager's lock.
    bool IsEntitled(int entitlement);  // 0xAE40C0
};

extern RBEntitlementMgr* theRBEntitlementMgr;  // 0x1AF4E38
// -------------------------------------------------------------------------

namespace {

// The id of no object, at 0x1B43A00, which the static initializer sets
// first. Each object that includes the header has its own copy. Name not
// in the reference map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The name of the content in the game's own folders (0x1B43A08): its songs
// are never mounted or released, and GetSongFilePath does not put its root
// in front. Name not in the reference map.
Symbol sRootContent(".");

// The filter SetContentFilter installs (0x1B43A18). Name not in the
// reference map.
bool (*sContentFilter)(Symbol content);

// The song cache block's revision. Name not in the reference map.
constexpr int kSongCacheRevision = 8;
// SongMetadata revisions up to this one are skipped on load. Name not in
// the reference map.
constexpr int kOldestLoadedMetadataRev = 10;
// SongMountHandle::Poll asks again after this long (0x133CBC0). Name not in
// the reference map.
constexpr float kMountTimeoutMs = 6000.0f;

// The rank thresholds of each part's tiers (0x133CCF0); parts 3 and 5 have
// none. Name not in the reference map.
const float kRankTierThresholds[7][6] = {
    {138.0f, 175.0f, 220.0f, 266.0f, 332.0f, 408.0f},
    {134.0f, 180.0f, 227.0f, 292.0f, 363.0f, 435.0f},
    {123.0f, 150.0f, 177.0f, 241.0f, 344.0f, 447.0f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {131.0f, 174.0f, 217.0f, 278.0f, 352.0f, 426.0f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {164.0f, 214.0f, 242.0f, 266.0f, 291.0f, 344.0f},
};

// The resolvers Init installs. Names not in the reference map.
ResourcePath SongContentFilePath(const char* file);
ResourcePath SongMoggMusicFilePath(ResourcePath owner, const char* file);
ResourcePath SongMoggCompanionPath(ResourcePath mogg);

// Helpers for the subset of EASTL the containers model. Names not in the
// reference map.

template <typename Key>
bool Contains(const eastl::set<Key>& set, const Key& key) {
    return set.find(key) != set.end();
}

// Calls `func` with each key of the set, in order.
template <typename Key, typename Func>
void ForEach(const eastl::set<Key>& set, Func func) {
    for (const eastl::rbtree_node_base* node = set.mAnchor.mpNodeLeft; node != &set.mAnchor;
         node = eastl::RBTreeIncrement(node)) {
        func(static_cast<const typename eastl::set<Key>::node_type*>(node)->mValue);
    }
}

template <typename Key>
void Clear(eastl::set<Key>& set) {
    while (set.mnSize != 0) {
        set.erase(static_cast<typename eastl::set<Key>::node_type*>(set.mAnchor.mpNodeLeft)->mValue);
    }
}

// The lookups of the const methods.
template <typename Key, typename T>
typename eastl::map<Key, T>::iterator Find(const eastl::map<Key, T>& map, const Key& key) {
    return const_cast<eastl::map<Key, T>&>(map).find(key);
}
template <typename Key, typename T>
typename eastl::map<Key, T>::iterator End(const eastl::map<Key, T>& map) {
    return const_cast<eastl::map<Key, T>&>(map).end();
}

// The value of the key, inserted constructed in place when missing, for the
// maps whose values own storage (eastl::map's operator[] assigns over the
// raw node).
template <typename Key, typename T>
T& FindOrInsert(eastl::map<Key, T>& map, const Key& key) {
    using Node = typename eastl::map<Key, T>::node_type;
    const auto it = map.lower_bound(key);
    if (it != map.end() && !(key < it->first)) {
        return it->second;
    }
    eastl::rbtree_node_base* current = map.mAnchor.mpNodeParent;
    eastl::rbtree_node_base* parent = &map.mAnchor;
    bool less = true;
    while (current != nullptr) {
        parent = current;
        less = key < static_cast<Node*>(current)->mValue.first;
        current = less ? current->mpNodeLeft : current->mpNodeRight;
    }
    auto* node = static_cast<Node*>(map.mAllocator.allocate(sizeof(Node)));
    new (const_cast<Key*>(&node->mValue.first)) Key(key);
    new (&node->mValue.second) T();
    const auto side = parent == &map.mAnchor || less ? eastl::kRBTreeSideLeft : eastl::kRBTreeSideRight;
    eastl::RBTreeInsert(node, parent, &map.mAnchor, side);
    ++map.mnSize;
    return node->mValue.second;
}

// Releases the storage of the values before the map frees its nodes.
template <typename Key, typename T>
void ReleaseValues(eastl::map<Key, T>& map) {
    for (auto it = map.begin(); it != map.end(); ++it) {
        it->second.~T();
        new (&it->second) T();
    }
}

}  // namespace

// Writes the count, then each content and its songs. The map's build has
// operator<< <Symbol, eastl::vector<int>>; this build's songs are a list.
template <typename Key, typename T>
BinStream& operator<<(BinStream& stream, const eastl::map<Key, T>& map);

// Reconstructed from eboot.elf at 0xF2D0F0.
template <>
BinStream& operator<<(BinStream& stream, const eastl::map<Symbol, eastl::list<int>>& map) {
    int count = static_cast<int>(map.mnSize);
    stream.WriteEndian(&count, sizeof(count));
    auto& songs = const_cast<eastl::map<Symbol, eastl::list<int>>&>(map);
    for (auto it = songs.begin(); it != songs.end(); ++it) {
        BinStream& out = stream << it->first;
        int size = static_cast<int>(it->second.size());
        out.WriteEndian(&size, sizeof(size));
        for (auto song = it->second.begin(); song != it->second.end(); ++song) {
            int songID = *song;
            out.WriteEndian(&songID, sizeof(songID));
        }
    }
    return stream;
}

// Reconstructed from eboot.elf at 0xF2DA80.
template <>
BinStream& operator>>(BinStream& stream, eastl::map<Symbol, eastl::vector<int>>& map) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        Symbol key;
        stream >> key;
        eastl::vector<int>& songs = FindOrInsert(map, key);
        unsigned int size;
        stream.ReadEndian(&size, sizeof(size));
        songs.resize(size);
        for (int& songID : songs) {
            stream.ReadEndian(&songID, sizeof(songID));
        }
    }
    return stream;
}

// Reconstructed from eboot.elf at 0xF2DC20.
template <>
BinStream& operator>>(BinStream& stream, eastl::map<Symbol, Symbol>& map) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    for (; count != 0; --count) {
        Symbol key;
        stream >> key;
        stream >> map[key];
    }
    return stream;
}

SongMgr* theSongMgr;

// Reconstructed from eboot.elf at 0xF29E40.
void SongMgr::Init() {
    new SongMgr();
    SetContentFilePathFunc(SongContentFilePath);
    SetMoggMusicFilePathFunc(SongMoggMusicFilePath);
    SetMoggCompanionPathFunc(SongMoggCompanionPath);
}

namespace {

// Reconstructed from eboot.elf at 0xF29E80. A found .songdta records its
// song's folder and resolves to the song's file of that name.
ResourcePath SongContentFilePath(const char* file) {
    ResourcePath path(file);
    theSongMgr->_SetSongDirFromPath(path);
    ResourcePath source(file);
    const char* ext = FileGetExt(file, false);
    char dir[512];
    char base[512];
    const Symbol shortName(FileGetBase(FileGetPath(source.Str(), dir), base));
    StackString<512> name;
    name << shortName << "." << ext;
    return ResourcePath(SongMgr::GetSongFilePath(shortName, name.c_str(), source));
}

// Reconstructed from eboot.elf at 0xF2A100. A mogg resolves to its
// platform copy.
ResourcePath SongMoggMusicFilePath(ResourcePath owner, const char* file) {
    char dir[512];
    char base[512];
    const Symbol shortName(FileGetBase(FileGetPath(owner.Str(), dir), base));
    const char* path = SongMgr::GetSongFilePath(shortName, file, owner);
    if (std::strcmp(FileGetExt(file, false), "mogg") != 0) {
        return ResourcePath(path);
    }
    char platformPath[512];
    return ResourcePath(MakePlatformPath(path, true, platformPath));
}

// Reconstructed from eboot.elf at 0xF2A3B0. A mogg in a song folder has
// its companion "<short name>.mogg.dta" in the song's folder, keeping the
// subfolder after the song's; others use MakeResourceCompanionFilepath.
ResourcePath SongMoggCompanionPath(ResourcePath mogg) {
    char base[512];
    const Symbol shortName(FileGetBase(mogg.Str(), base));
    char dir[512];
    const char* folder = FileGetPath(mogg.Str(), dir);
    StackString<512> songFolder;
    songFolder << "/" << shortName;
    const char* found = std::strstr(folder, songFolder.c_str());
    if (found == nullptr) {
        return ResourcePath(MakeResourceCompanionFilepath(mogg));
    }
    // The binary loops without advancing while the character is neither a
    // separator nor the end; only those two cases are reachable.
    const char* rest = found + std::strlen(songFolder.c_str());
    FormatString format(*rest == '/' ? "%s/%s.mogg.dta" : "%s.mogg.dta");
    if (*rest == '/') {
        format << rest + 1;
    }
    format << shortName;
    const char* companion = format.Str();
    const char* path = SongMgr::GetSongFilePath(shortName, companion, mogg);
    char platformPath[512];
    return ResourcePath(MakePlatformPath(path, false, platformPath));
}

}  // namespace

// Reconstructed from eboot.elf at 0xF2A740.
void SongMgr::Terminate() {
    if (theSongMgr != nullptr) {
        delete theSongMgr;
    }
}

// Reconstructed from eboot.elf at 0xF2A760.
void SongMgr::SetContentFilter(bool (*filter)(Symbol content)) {
    if (sContentFilter != filter && sContentFilter == nullptr) {
        sContentFilter = filter;
    }
}

// Reconstructed from eboot.elf at 0xF2A780.
const char* SongMgr::GetSongFilePathWithExt(Symbol shortName, const char* ext, const ResourcePath& source) {
    StackString<512> name;
    name << shortName << "." << ext;
    return GetSongFilePath(shortName, name.c_str(), source);
}

// Reconstructed from eboot.elf at 0xF2A860. A song the manager knows lives
// in "<song folder>/<short name>/" under its content's root; any other is
// placed beside the source, its folder taken without the drive. A copy in
// "patched_songs/<short name>/" wins when it exists.
const char* SongMgr::GetSongFilePath(Symbol shortName, const char* file, const ResourcePath& source) {
    StackString<512> root;
    Symbol songDir(gNullStr);
    SongMgr* mgr = theSongMgr;
    if (shortName != Symbol()) {
        const auto it = mgr->mShortNameToSongID.find(shortName);
        if (it != mgr->mShortNameToSongID.end() && it->second != 0
            && Contains(mgr->mAvailableSongs, it->second)) {
            const int songID = mgr->GetSongIDFromShortName(shortName, false);
            const StackString<512> contentRoot(TheContentMgr.ContentRoot(mgr->GetContentName(songID)));
            root = contentRoot.c_str();
            const auto dirIt = mgr->mSongDirs.find(shortName);
            songDir = dirIt != mgr->mSongDirs.end() ? dirIt->second : Symbol(gNullStr);
        }
    }
    if (songDir == Symbol() && source.mPath != Symbol()) {
        char dir[512];
        char parent[512];
        songDir = Symbol(FileGetPath(FileGetPath(source.Str(), dir), parent));
        char drive[512];
        const StackString<512> driveName(FileGetDrive(source.Str(), drive));
        root = driveName.c_str();
        if (root.c_str()[0] != '\0') {
            root << ":/";
            songDir = Symbol(songDir.Str() + std::strlen(root.c_str()));
        }
    }
    StackString<512> path;
    path << songDir << "/" << shortName << "/" << file;
    if (strcasecmp(root.c_str(), sRootContent.Str()) != 0 && strcasecmp(root.c_str(), gNullStr) != 0) {
        char madePath[512];
        const StackString<512> rooted(FileMakePath(root.c_str(), path.c_str(), madePath));
        path = rooted.c_str();
    }
    StackString<512> patched;
    patched << "patched_songs/" << shortName << "/" << file;
    char platformPath[512];
    if (FileExists(MakePlatformPath(patched.c_str(), true, platformPath), kReadNoBuffer)) {
        path = patched.c_str();
    }
    FormatString format("%s");
    format << static_cast<const FixedString&>(path);
    return format.Str();
}

// Reconstructed from eboot.elf at 0xF2B3B0.
const char* SongMgr::GetSongFilePath(int songID, const char* ext, const ResourcePath& source) {
    const Symbol shortName = theSongMgr->GetShortNameFromSongID(songID, false);
    StackString<512> name;
    name << shortName << "." << ext;
    return GetSongFilePath(shortName, name.c_str(), source);
}

// Reconstructed from eboot.elf at 0xF2B520.
Symbol SongMgr::GetShortNameFromSongID(int songID, bool unused) const {
    static_cast<void>(unused);
    const auto it = Find(mSongIDToShortName, songID);
    return it != End(mSongIDToShortName) ? it->second : Symbol(gNullStr);
}

// Reconstructed from eboot.elf at 0xF2B5B0.
const char* SongMgr::GetSongFilePathFromResourcePath(const ResourcePath& path, const char* ext) {
    char dir[512];
    char base[512];
    const Symbol shortName(FileGetBase(FileGetPath(path.Str(), dir), base));
    StackString<512> name;
    name << shortName << "." << ext;
    return GetSongFilePath(shortName, name.c_str(), path);
}

// Reconstructed from eboot.elf at 0xF2B7D0.
Symbol SongMgr::ResourcePathToSongShortName(const ResourcePath& path) {
    char dir[512];
    char base[512];
    return Symbol(FileGetBase(FileGetPath(path.Str(), dir), base));
}

// Reconstructed from eboot.elf at 0xF2B960.
bool SongMgr::HasSong(Symbol shortName) const {
    if (shortName == Symbol()) {
        return false;
    }
    const auto it = Find(mShortNameToSongID, shortName);
    if (it == End(mShortNameToSongID) || it->second == 0) {
        return false;
    }
    return Contains(mAvailableSongs, it->second);
}

// Reconstructed from eboot.elf at 0xF2BA70.
Symbol SongMgr::GetContentName(Symbol shortName, bool unused) const {
    static_cast<void>(unused);
    return GetContentName(GetSongIDFromShortName(shortName, false));
}

// Reconstructed from eboot.elf at 0xF2BB10.
Symbol SongMgr::GetSongDirFromShortName(Symbol shortName) {
    const auto it = mSongDirs.find(shortName);
    return it != mSongDirs.end() ? it->second : Symbol(gNullStr);
}

// Reconstructed from eboot.elf at 0xF2BBD0.
void SongMgr::_SetSongDirFromPath(ResourcePath path) {
    char dir[512];
    char base[512];
    const Symbol shortName(FileGetBase(FileGetPath(path.Str(), dir), base));
    Symbol songDir;
    if (FileIsLocal(path.Str())) {
        char songFolder[512];
        char parent[512];
        char parentBase[512];
        songDir = Symbol(FileGetBase(FileGetPath(FileGetPath(path.Str(), songFolder), parent), parentBase));
    } else {
        char songFolder[512];
        char parent[512];
        songDir = Symbol(FileGetPath(FileGetPath(path.Str(), songFolder), parent));
    }
    mSongDirs[shortName] = songDir;
}

// Reconstructed from eboot.elf at 0xF2C150.
SongMgr::SongMgr()
    : SyncMsgSource(&mCritSec),
      mSongCacheDirty(false),
      mSongCacheLoaded(false),
      mNumDiscoveredContent(0),
      mNumDiscoveredContentMounted(0),
      mNumOtherSongs(0),
      mNumDiscSongs(0),
      mNumRB4Songs(0),
      mNumRivalsSongs(0),
      mNumDiscSongsWithPart(0),
      mNumRB4SongsWithPart(0),
      mNumRivalsSongsWithPart(0) {
    theSongMgr = this;
    DataNode self;
    self.mType = kDataObject;
    self.mValue.object = static_cast<MsgSink*>(this);
    DataSetGlobal(Symbol("song_mgr"), self);
    TheContentMgr.RegisterCallback(this, false);

    static Symbol songMgr;
    if (songMgr == Symbol()) {
        songMgr = Symbol("song_mgr");
    }
    DataArray* config = SystemConfig(songMgr);
    static Symbol dirs;
    if (dirs == Symbol()) {
        dirs = Symbol("dirs");
    }
    DataArray* dirArray = config->FindArray(dirs, false);
    if (dirArray != nullptr) {
        for (int i = 1; i < dirArray->Size(); ++i) {
            const String dir(dirArray->Str(i));
            bool found = false;
            for (auto it = mContentDirs.begin(); it != mContentDirs.end(); ++it) {
                if (*it == dir) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                mContentDirs.push_back(dir);
            }
        }
    }
    static Symbol paygatedDirs;
    if (paygatedDirs == Symbol()) {
        paygatedDirs = Symbol("paygated_dirs");
    }
    DataArray* paygatedArray = config->FindArray(paygatedDirs, false);
    if (paygatedArray != nullptr) {
        for (int i = 1; i < paygatedArray->Size(); ++i) {
            const DataArrayPtr entry = paygatedArray->Node(i).Array(paygatedArray);
            const String dir(entry->Str(0));
            PaygatedDir paygated;
            paygated.mEntitlement = entry->Int(1);
            paygated.mDir = dir;
            mPaygatedDirs.push_back(paygated);
        }
    }
}

// Reconstructed from eboot.elf at 0xF2CB60. The cached metadata is not
// freed.
SongMgr::~SongMgr() {
    TheContentMgr.UnregisterCallback(this, false);
    DataNode none;
    none.mType = kDataObject;
    none.mValue.object = nullptr;
    DataSetGlobal(Symbol("song_mgr"), none);
    theSongMgr = nullptr;
    ReleaseValues(mContentSongs);
}

// Reconstructed from eboot.elf at 0xF2CE50.
void SongMgr::Poll() {
    if (!mSongCacheLoaded && HasSongCacheLoaded()) {
        mSongCacheLoaded = true;
    }
}

// Reconstructed from eboot.elf at 0xF2CEA0.
bool SongMgr::HasSongCacheLoaded() const {
    for (Profile* profile : theRBProfileMgr->mProfiles) {
        if (profile->mReadyToSave) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xF2CEE0.
void SongMgr::SaveSongCache(BinStream& stream) {
    RevisionStream revisionStream(stream, kSongCacheRevision, 0);
    revisionStream.BeginWrite();
    int value = SongMetadata::kRev;
    revisionStream.WriteEndian(&value, sizeof(value));
    revisionStream << mContentSongs;
    value = static_cast<int>(mSongDirs.size());
    revisionStream.WriteEndian(&value, sizeof(value));
    for (auto it = mSongDirs.begin(); it != mSongDirs.end(); ++it) {
        revisionStream << it->first << it->second;
    }
    value = sizeof(SongMetadata);
    revisionStream.WriteEndian(&value, sizeof(value));
    value = static_cast<int>(mSongCache.size());
    revisionStream.WriteEndian(&value, sizeof(value));
    for (auto it = mSongCache.begin(); it != mSongCache.end(); ++it) {
        value = it->first;
        revisionStream.WriteEndian(&value, sizeof(value));
        revisionStream.Write(it->second, sizeof(SongMetadata));
    }
    value = static_cast<int>(mRecentlyAcquired.size());
    revisionStream.WriteEndian(&value, sizeof(value));
    for (auto it = mRecentlyAcquired.begin(); it != mRecentlyAcquired.end(); ++it) {
        value = *it;
        revisionStream.WriteEndian(&value, sizeof(value));
    }
}

// Reconstructed from eboot.elf at 0xF2D1E0. Metadata of revision 10 or
// older is skipped. A block of revision 0 or 1 takes each song's patched
// .songdta when there is one. The recently acquired songs are read only
// when the list is empty.
void SongMgr::LoadSongCache(BinStream& stream) {
    unsigned int savedTemp;
    MemPushTemp(savedTemp, true, true);
    {
        RevisionStream revisionStream(stream, kSongCacheRevision, 0);
        if (revisionStream.BeginRead()) {
            int metadataRev;
            revisionStream.ReadEndian(&metadataRev, sizeof(metadataRev));
            eastl::map<Symbol, eastl::vector<int>> contentSongs;
            revisionStream >> contentSongs;
            eastl::map<Symbol, Symbol> songDirs;
            revisionStream >> songDirs;
            eastl::map<int, SongMetadata*> metadata;
            unsigned int size;
            revisionStream.ReadEndian(&size, sizeof(size));
            unsigned int count;
            revisionStream.ReadEndian(&count, sizeof(count));
            for (unsigned int i = 0; i < count; ++i) {
                int songID;
                revisionStream.ReadEndian(&songID, sizeof(songID));
                if (metadataRev <= kOldestLoadedMetadataRev) {
                    unsigned int savedSkip;
                    MemPushTemp(savedSkip, true, true);
                    void* skipped = MemAlloc(size, "SongMgr Temp Alloc", 0);
                    revisionStream.Read(skipped, size);
                    MemFree(skipped);
                    MemPopTemp(savedSkip);
                } else {
                    auto* song = new (MemAlloc(sizeof(SongMetadata), "SongMetadata", 0)) SongMetadata();
                    revisionStream.Read(song, size);
                    metadata[songID] = song;
                }
            }
            if (metadataRev > kOldestLoadedMetadataRev) {
                if (revisionStream.mReadRevision <= 1) {
                    for (auto it = metadata.begin(); it != metadata.end(); ++it) {
                        SongMetadata* song = it->second;
                        const Symbol shortName(song->mShortName);
                        StackString<512> patched;
                        patched << "patched_songs/" << shortName << "/" << shortName << ".songdta";
                        char platformPath[512];
                        if (FileExists(MakePlatformPath(patched.c_str(), true, platformPath), kReadNoBuffer)) {
                            const ResourcePath path(patched.c_str());
                            const ResourcePtr<SongMetadataResource> resource =
                                Resource::GetOrLoad<SongMetadataResource>(path, false);
                            if (resource != nullptr && !resource->Fail()) {
                                std::memcpy(song, resource->mMetadata, sizeof(SongMetadata));
                            }
                        }
                    }
                }
                _UpdateFromLoadedCache(contentSongs, metadata, songDirs);
            }
            if (mRecentlyAcquired.size() != 0) {
                unsigned int savedDropped;
                MemPushTemp(savedDropped, true, true);
                {
                    eastl::vector<int> dropped;
                    unsigned int recentCount;
                    revisionStream.ReadEndian(&recentCount, sizeof(recentCount));
                    dropped.resize(recentCount);
                    for (int& songID : dropped) {
                        revisionStream.ReadEndian(&songID, sizeof(songID));
                    }
                }
                MemPopTemp(savedDropped);
            } else {
                unsigned int recentCount;
                revisionStream.ReadEndian(&recentCount, sizeof(recentCount));
                for (unsigned int i = 0; i < recentCount; ++i) {
                    mRecentlyAcquired.push_back(0);
                }
                for (auto it = mRecentlyAcquired.begin(); it != mRecentlyAcquired.end(); ++it) {
                    revisionStream.ReadEndian(&*it, sizeof(int));
                }
            }
            mSongCacheLoaded = true;
            for (auto it = metadata.begin(); it != metadata.end(); ++it) {
                if (it->second != nullptr) {
                    MemFree(it->second);
                }
            }
            metadata.clear();
            ReleaseValues(contentSongs);
        }
    }
    MemPopTemp(savedTemp);
}

// Reconstructed from eboot.elf at 0xF2DE90. A song takes the last content
// listing it; songs of no content are added with the empty content.
void SongMgr::_UpdateFromLoadedCache(
    const eastl::map<Symbol, eastl::vector<int>>& contentSongs,
    const eastl::map<int, SongMetadata*>& metadata,
    const eastl::map<Symbol, Symbol>& songDirs) {
    auto& songs = const_cast<eastl::map<int, SongMetadata*>&>(metadata);
    auto& contents = const_cast<eastl::map<Symbol, eastl::vector<int>>&>(contentSongs);
    for (auto it = songs.begin(); it != songs.end(); ++it) {
        const int songID = it->first;
        if (mSongCache.find(songID) != mSongCache.end()) {
            continue;
        }
        Symbol content(gNullStr);
        for (auto contentIt = contents.begin(); contentIt != contents.end(); ++contentIt) {
            for (const int contentSong : contentIt->second) {
                if (songID == contentSong) {
                    content = contentIt->first;
                    break;
                }
            }
        }
        if (content != sRootContent) {
            _AddSongData(it->second, content);
        }
    }
    auto& dirs = const_cast<eastl::map<Symbol, Symbol>&>(songDirs);
    for (auto it = dirs.begin(); it != dirs.end(); ++it) {
        if (mSongDirs.find(it->first) == mSongDirs.end()) {
            mSongDirs[it->first] = it->second;
        }
    }
}

// Reconstructed from eboot.elf at 0xF2E160.
void SongMgr::_SetSongCacheDirty() {
    mSongCacheDirty = true;
    static Symbol songCacheDirty;
    if (songCacheDirty == Symbol()) {
        songCacheDirty = Symbol("song_cache_dirty");
    }
    Message<0> msg(songCacheDirty);
    Export(msg.mData);
}

// Reconstructed from eboot.elf at 0xF2E230.
int SongMgr::GetPosInRecentList(int songID) {
    int pos = 0;
    for (auto it = mRecentlyAcquired.begin(); it != mRecentlyAcquired.end(); ++it, ++pos) {
        if (*it == songID) {
            return pos;
        }
    }
    return -1;
}

// Reconstructed from eboot.elf at 0xF2E290.
void SongMgr::ClearSongCache() {
    ReleaseValues(mContentSongs);
    mContentSongs.clear();
    for (auto it = mSongCache.begin(); it != mSongCache.end(); ++it) {
        if (it->second != nullptr) {
            MemFree(it->second);
        }
    }
    mSongCache.clear();
}

// Reconstructed from eboot.elf at 0xF2E370.
int SongMgr::NumSongs(bool includeDiscSongs, bool includeTutorials) const {
    int count = 0;
    ForEach(mAvailableSongs, [&](int songID) {
        const SongMetadata* metadata = GetMetadata(songID, false);
        if (!includeDiscSongs && !metadata->IsDownload()) {
            return;
        }
        if (includeTutorials || !metadata->IsTutorial()) {
            ++count;
        }
    });
    return count;
}

// Reconstructed from eboot.elf at 0xF2E650.
SongMetadata* SongMgr::GetMetadata(int songID, bool unused) const {
    static_cast<void>(unused);
    if (!Contains(mAvailableSongs, songID)) {
        return nullptr;
    }
    const auto it = Find(mSongCache, songID);
    return it != End(mSongCache) ? it->second : nullptr;
}

// Reconstructed from eboot.elf at 0xF2E740.
bool SongMgr::HasSong(int songID) const {
    return Contains(mAvailableSongs, songID);
}

// Reconstructed from eboot.elf at 0xF2E7B0.
int SongMgr::GetSongIDFromShortName(Symbol shortName, bool unused) const {
    static_cast<void>(unused);
    if (shortName == Symbol()) {
        return 0;
    }
    const auto it = Find(mShortNameToSongID, shortName);
    return it != End(mShortNameToSongID) ? it->second : 0;
}

// Reconstructed from eboot.elf at 0xF2E840.
SongMetadata* SongMgr::GetMetadata(Symbol shortName, bool unused) const {
    static_cast<void>(unused);
    return GetMetadata(GetSongIDFromShortName(shortName, false), false);
}

// Reconstructed from eboot.elf at 0xF2E9D0.
eastl::vector<SongMetadata*> SongMgr::GetMetadataList(const eastl::vector<Symbol>& shortNames, bool unused) const {
    eastl::vector<SongMetadata*> metadata;
    metadata.reserve(shortNames.size());
    for (const Symbol& shortName : shortNames) {
        metadata.push_back(GetMetadata(shortName, unused));
    }
    return metadata;
}

// Reconstructed from eboot.elf at 0xF2EBD0.
void SongMgr::ContentRefreshStarted(int refreshType) {
    if (refreshType == 2) {
        Clear(mDiscoveredSongs);
        mNumDiscoveredContent = 0;
    }
}

// Reconstructed from eboot.elf at 0xF2EC90.
void SongMgr::ContentRefreshDone(int refreshType) {
    if (refreshType == 2) {
        _ReplaceAvailableSongs();
    } else if (refreshType == 1) {
        _AddDiscoveredSongs();
    }
}

// Reconstructed from eboot.elf at 0xF2ED60.
void SongMgr::_AddDiscoveredSongs() {
    ForEach(mDiscoveredSongs, [this](int songID) {
        mAvailableSongs.insert(songID);
    });
    _SendSongContentRefreshedMsg();
    _UpdateSongCounts();
}

// Reconstructed from eboot.elf at 0xF2EE70.
void SongMgr::_ReplaceAvailableSongs() {
    Clear(mAvailableSongs);
    ForEach(mDiscoveredSongs, [this](int songID) {
        mAvailableSongs.insert(songID);
    });
    _SendSongContentRefreshedMsg();
    _UpdateSongCounts();
}

// Reconstructed from eboot.elf at 0xF2EF30. Location 1 is always mounted.
// Content the cache knows is not mounted; its cached songs are discovered.
void SongMgr::ContentDiscovered(int location, Symbol content, bool& mount) {
    if (sContentFilter != nullptr && sContentFilter(content)) {
        return;
    }
    const auto it = mContentSongs.find(content);
    if (location == 1) {
        mount = true;
    } else {
        mount = it == mContentSongs.end();
        if (it != mContentSongs.end()) {
            for (auto song = it->second.begin(); song != it->second.end(); ++song) {
                mDiscoveredSongs.insert(*song);
            }
            _IncDiscoveredContentMounted(content);
        }
    }
    if (sRootContent != content) {
        ++mNumDiscoveredContent;
    }
}

// Reconstructed from eboot.elf at 0xF2F130.
void SongMgr::_IncDiscoveredContentMounted(Symbol content) {
    if (TheContentMgr.IsRefreshInProgress() && sRootContent != content) {
        ++mNumDiscoveredContentMounted;
    }
}

// Reconstructed from eboot.elf at 0xF2F180.
void SongMgr::ContentMounted(Symbol content, const char* root) {
    static_cast<void>(root);
    if (sContentFilter != nullptr && sContentFilter(content)) {
        return;
    }
    _IncDiscoveredContentMounted(content);
    _SendSongMountCompletedMsg(content, true);
}

// Reconstructed from eboot.elf at 0xF2F1E0.
void SongMgr::_SendSongMountCompletedMsg(Symbol content, bool success) {
    const auto it = mContentSongs.find(content);
    if (it == mContentSongs.end()) {
        return;
    }
    for (auto song = it->second.begin(); song != it->second.end(); ++song) {
        const int songID = *song;
        SongMountCompletedMsg msg(success, GetShortNameFromSongID(songID, false), songID);
        Export(msg.mData);
    }
}

// Reconstructed from eboot.elf at 0xF2F410.
void SongMgr::ContentFailed(Symbol content) {
    if (sContentFilter != nullptr && sContentFilter(content)) {
        return;
    }
    _IncDiscoveredContentMounted(content);
    _SendSongMountCompletedMsg(content, false);
}

// Reconstructed from eboot.elf at 0xF2F4D0.
void SongMgr::ContentLoaded(Symbol content, Resource* resource) {
    if (sContentFilter != nullptr && sContentFilter(content)) {
        return;
    }
    if (resource->Fail()) {
        return;
    }
    _AddSongData(static_cast<SongMetadataResource*>(resource)->mMetadata, content);
}

// Reconstructed from eboot.elf at 0xF2F530. A new song in a loaded cache
// that is a download counts as recently acquired. The binary also searches
// the content's songs for the song first and drops the result.
void SongMgr::_AddSongData(SongMetadata* metadata, Symbol content) {
    const Symbol shortName(metadata->mShortName);
    int songID = metadata->GetID();
    if (Contains(mDiscoveredSongs, songID)) {
        return;
    }
    mDiscoveredSongs.insert(songID);
    mSongIDToShortName[songID] = shortName;
    mShortNameToSongID[shortName] = songID;
    if (mSongCache.find(songID) != mSongCache.end()) {
        return;
    }
    auto* song = static_cast<SongMetadata*>(MemAlloc(sizeof(SongMetadata), "SongMetadata", 0));
    std::memcpy(song, metadata, sizeof(SongMetadata));
    mSongCache[songID] = song;
    mSongIDToContent[songID] = content;
    FindOrInsert(mContentSongs, content).push_back(songID);
    if (mSongCacheLoaded && metadata->IsDownload()) {
        _AddSongAsRecentlyAcquired(songID, mSongCache[songID]);
    }
    _SetSongCacheDirty();
}

// Reconstructed from eboot.elf at 0xF2FC00.
const char* SongMgr::ContentPattern() {
    return "*/*.songdta*";
}

// Reconstructed from eboot.elf at 0xF2FC20.
eastl::list<String> SongMgr::ContentDirs() {
    eastl::list<String> dirs;
    for (auto it = mContentDirs.begin(); it != mContentDirs.end(); ++it) {
        dirs.push_back(*it);
    }
    for (auto it = mPaygatedDirs.begin(); it != mPaygatedDirs.end(); ++it) {
        if (theRBEntitlementMgr->IsEntitled(it->mEntitlement)) {
            dirs.push_back(it->mDir);
        }
    }
    return dirs;
}

// Reconstructed from eboot.elf at 0xF2FDB0. The result is the text of a
// local string, as in the binary; the caller copies it at once.
const char* SongMgr::ContentFileFound(Symbol content, const char* file) {
    static_cast<void>(content);
    StackString<512> path(file);
    const ResourcePath resourcePath(file);
    const bool local = FileIsLocal(resourcePath.Str());
    static const unsigned long sSongDtaLength = std::strlen(".songdta");
    const unsigned long end = path.find(".songdta") + sSongDtaLength;
    if (end < std::strlen(path.c_str())) {
        path.erase(end);
    }
    if (!local) {
        const StackString<512> rest(path.c_str() + path.find("/") + 1);
        path = rest.c_str();
    }
    return path.c_str();
}

// Reconstructed from eboot.elf at 0xF30010.
Symbol SongMgr::GetContentName(int songID) const {
    if (!Contains(mAvailableSongs, songID)) {
        return Symbol(gNullStr);
    }
    const auto it = Find(mSongIDToContent, songID);
    return Symbol(it != End(mSongIDToContent) ? it->second.Str() : nullptr);
}

// Reconstructed from eboot.elf at 0xF30150. The binary reads the anchor of
// the folder map for a song without a folder.
bool SongMgr::IsSongEntitled(Symbol shortName) const {
    const auto dirIt = Find(mSongDirs, shortName);
    const Symbol dir = dirIt != End(mSongDirs) ? dirIt->second : Symbol();
    auto& paygated = const_cast<eastl::list<PaygatedDir>&>(mPaygatedDirs);
    for (auto it = paygated.begin(); it != paygated.end(); ++it) {
        if (it->mDir == dir) {
            return theRBEntitlementMgr->IsEntitled(it->mEntitlement);
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0xF30240.
bool SongMgr::IsSongContentAvailable(int songID) const {
    if (!Contains(mAvailableSongs, songID)) {
        return false;
    }
    if (!IsSongEntitled(GetShortNameFromSongID(songID, false))) {
        return false;
    }
    return TheContentMgr.IsContentAvailable(GetContentName(songID), true);
}

// Reconstructed from eboot.elf at 0xF30470.
void SongMgr::MountSong(int songID, bool& mounted) {
    MountSong(GetShortNameFromSongID(songID, false), mounted);
}

// Reconstructed from eboot.elf at 0xF30500.
void SongMgr::MountSong(Symbol shortName, bool& mounted) {
    mounted = false;
    const Symbol content = GetContentName(GetSongIDFromShortName(shortName, false));
    if (content == Symbol()) {
        SongMountCompletedMsg msg(false, shortName, 0);
        Export(msg.mData);
        return;
    }
    if (IsSongEntitled(shortName) && TheContentMgr.MountContent(content)) {
        mounted = true;
        return;
    }
    _SendSongMountCompletedMsg(content, false);
}

// Reconstructed from eboot.elf at 0xF30760.
bool SongMgr::IsSongMounted(int songID) const {
    return IsSongMounted(GetShortNameFromSongID(songID, false));
}

// Reconstructed from eboot.elf at 0xF308B0.
bool SongMgr::IsSongMounted(Symbol shortName) const {
    const Symbol content = GetContentName(GetSongIDFromShortName(shortName, false));
    if (content == Symbol()) {
        return true;
    }
    return TheContentMgr.IsContentMounted(content);
}

// Reconstructed from eboot.elf at 0xF30980.
void SongMgr::ReleaseSongMountRef(int songID) const {
    ReleaseSongMountRef(GetShortNameFromSongID(songID, false));
}

// Reconstructed from eboot.elf at 0xF30AE0.
void SongMgr::ReleaseSongMountRef(Symbol shortName) const {
    const Symbol content = GetContentName(GetSongIDFromShortName(shortName, false));
    if (content != Symbol() && content != sRootContent) {
        TheContentMgr.ReleaseMountRef(content);
    }
}

// Reconstructed from eboot.elf at 0xF30BB0.
void SongMgr::_AddSongAsRecentlyAcquired(int songID, const SongMetadata* metadata) {
    static_cast<void>(metadata);
    mRecentlyAcquired.push_back(songID);
    while (mRecentlyAcquired.size() > kMaxRecentSongs) {
        mRecentlyAcquired.erase(mRecentlyAcquired.begin());
    }
    _SetSongCacheDirty();
}

// Reconstructed from eboot.elf at 0xF30C70.
const eastl::set<int>& SongMgr::GetAvailableSongSet() const {
    return mAvailableSongs;
}

// Reconstructed from eboot.elf at 0xF30C80.
eastl::set<int> SongMgr::GetPlaybleSongSet() const {
    eastl::vector<int> songs;
    ForEach(mAvailableSongs, [&](int songID) {
        const SongMetadata* metadata = GetMetadata(songID, false);
        if (metadata->IsPrivate()) {
            return;
        }
        if (metadata->IsDownload() || ThePlatformMgr->IsMarkerInstalled(Symbol(metadata->mShortName))) {
            songs.push_back(songID);
        }
    });
    return eastl::set<int>(songs.begin(), songs.end());
}

// Reconstructed from eboot.elf at 0xF30F50.
eastl::set<int> SongMgr::GetDownloadedSongSet() const {
    eastl::vector<int> songs;
    ForEach(mAvailableSongs, [&](int songID) {
        const SongMetadata* metadata = GetMetadata(songID, false);
        if (!metadata->IsPrivate() && metadata->IsDownload()) {
            songs.push_back(songID);
        }
    });
    return eastl::set<int>(songs.begin(), songs.end());
}

// Reconstructed from eboot.elf at 0xF31200.
eastl::set<int> SongMgr::GetAvailableSongsIn(const eastl::set<int>& songs) const {
    eastl::vector<int> available;
    ForEach(mAvailableSongs, [&](int songID) {
        const SongMetadata* metadata = GetMetadata(songID, false);
        if (!metadata->IsPrivate() && Contains(songs, songID)) {
            available.push_back(songID);
        }
    });
    return eastl::set<int>(available.begin(), available.end());
}

// Reconstructed from eboot.elf at 0xF31550.
eastl::set<int> SongMgr::FilterAvailableSongs(const eastl::set<int>& songs) const {
    eastl::vector<int> available;
    ForEach(songs, [&](int songID) {
        if (!Contains(mAvailableSongs, songID)) {
            return;
        }
        const SongMetadata* metadata = GetMetadata(songID, false);
        if (!metadata->IsPrivate()) {
            available.push_back(songID);
        }
    });
    return eastl::set<int>(available.begin(), available.end());
}

// Reconstructed from eboot.elf at 0xF31930.
eastl::set<int> SongMgr::GetInstalledDiscSongSet() const {
    eastl::vector<int> songs;
    ForEach(mAvailableSongs, [&](int songID) {
        const SongMetadata* metadata = GetMetadata(songID, false);
        if (metadata->IsPrivate() || metadata->IsDownload()) {
            return;
        }
        if (ThePlatformMgr->IsMarkerInstalled(Symbol(metadata->mShortName))) {
            songs.push_back(songID);
        }
    });
    return eastl::set<int>(songs.begin(), songs.end());
}

// Reconstructed from eboot.elf at 0xF31C20.
int SongMgr::GetRandomPlayableSong() const {
    const eastl::set<int> songs = GetPlaybleSongSet();
    int songID = 0;
    if (songs.mnSize != 0) {
        unsigned long index = gRand.Int() % songs.mnSize;
        const eastl::rbtree_node_base* node = songs.mAnchor.mpNodeLeft;
        for (; index != 0; --index) {
            node = eastl::RBTreeIncrement(node);
        }
        songID = static_cast<const eastl::set<int>::node_type*>(node)->mValue;
    }
    return songID;
}

// Reconstructed from eboot.elf at 0xF31CD0.
int SongMgr::GetRankTier(const SongMetadata* metadata, int part) const {
    return GetRankTier(metadata->GetRank(part), part);
}

// Reconstructed from eboot.elf at 0xF31D20.
int SongMgr::GetRankTier(float rank, int part) const {
    const float* thresholds = kRankTierThresholds[part];
    int tier = 0;
    while (tier < 6 && rank > thresholds[tier]) {
        ++tier;
    }
    return tier;
}

// Reconstructed from eboot.elf at 0xF31D60. The message's argument is left
// unset.
void SongMgr::_SendSongContentRefreshedMsg() {
    Clear(mDiscoveredSongs);
    mNumDiscoveredContent = 0;
    static Symbol songContentRefreshed;
    if (songContentRefreshed == Symbol()) {
        songContentRefreshed = Symbol("song_content_refreshed");
    }
    Message<1> msg(songContentRefreshed);
    Export(msg.mData);
}

// Reconstructed from eboot.elf at 0xF31E70.
void SongMgr::_UpdateSongCounts() {
    mNumOtherSongs = 0;
    mNumDiscSongs = 0;
    mNumRB4Songs = 0;
    mNumRivalsSongs = 0;
    mNumDiscSongsWithPart = 0;
    mNumRB4SongsWithPart = 0;
    mNumRivalsSongsWithPart = 0;
    ForEach(mAvailableSongs, [this](int songID) {
        const SongMetadata* metadata = mSongCache[songID];
        if (metadata->IsPrivate()) {
            return;
        }
        const bool hasPart = metadata->HasPart(1);
        if (metadata->IsRB4Song()) {
            ++mNumRB4Songs;
            if (hasPart) {
                ++mNumRB4SongsWithPart;
            }
        } else if (metadata->IsRivalsSong()) {
            ++mNumRivalsSongs;
            if (hasPart) {
                ++mNumRivalsSongsWithPart;
            }
        } else {
            ++mNumOtherSongs;
        }
    });
    mNumDiscSongs = mNumRB4Songs + mNumRivalsSongs;
    mNumDiscSongsWithPart = mNumRB4SongsWithPart + mNumRivalsSongsWithPart;
}

// Reconstructed from eboot.elf at 0xF32060.
SongMountHandle::SongMountHandle()
    : mSong(gNullStr), mState(kMountIdle), mHoldsMountRef(false), mRequestMs(-1.0f) {
    mSinkElem.mSink = nullptr;
    mSinkElem.mSource = nullptr;
}

// Reconstructed from eboot.elf at 0xF320B0.
SongMountHandle::SongMountHandle(Symbol song) : SongMountHandle() {
    SetSong(song);
}

// Reconstructed from eboot.elf at 0xF32100.
void SongMountHandle::SetSong(Symbol song) {
    if (mSong == song) {
        return;
    }
    if (mSong != Symbol()) {
        if (mHoldsMountRef) {
            theSongMgr->ReleaseSongMountRef(mSong);
        }
        if (mSinkElem.mSource != nullptr) {
            mSinkElem.mSource->RemoveSink(&mSinkElem);
        }
        mRequestMs = -1.0f;
    }
    mSong = song;
    mHoldsMountRef = false;
    mState = kMountIdle;
    if (song == Symbol()) {
        return;
    }
    mState = kMountPending;
    mRequestMs = TheTimeMgr->mRealTime.SplitMs();
    theSongMgr->AddSink(&mSinkElem, this, SongMountCompletedMsg::Type(), Symbol(gNullStr));
    theSongMgr->MountSong(mSong, mHoldsMountRef);
    if (!mHoldsMountRef) {
        mState = kMountIdle;
    }
}

// Reconstructed from eboot.elf at 0xF32380.
SongMountHandle::SongMountHandle(const SongMountHandle& other) : SongMountHandle() {
    SetSong(other.mSong);
}

// Reconstructed from eboot.elf at 0xF323D0.
SongMountHandle& SongMountHandle::operator=(const SongMountHandle& other) {
    SetSong(other.mSong);
    return *this;
}

// Reconstructed from eboot.elf at 0xF323F0.
SongMountHandle::~SongMountHandle() {
    if (mHoldsMountRef) {
        theSongMgr->ReleaseSongMountRef(mSong);
    }
    LinkedList::Node& link = mSinkElem.mLink;
    if (link.mNext != &link && link.mPrev != &link) {
        mSinkElem.mSource->RemoveSink(&mSinkElem);
    }
}

// Reconstructed from eboot.elf at 0xF32550.
void SongMountHandle::Clear() {
    SetSong(Symbol(gNullStr));
}

// Reconstructed from eboot.elf at 0xF325B0.
void SongMountHandle::Poll() {
    if (mSong == Symbol() || mState != kMountPending || !(mRequestMs > 0.0f)) {
        return;
    }
    if (TheTimeMgr->mRealTime.SplitMs() - mRequestMs > kMountTimeoutMs) {
        const Symbol song = mSong;
        SetSong(Symbol(gNullStr));
        SetSong(song);
    }
}

// Reconstructed from eboot.elf at 0xF326A0. The time a successful mount
// took is measured and dropped.
DataNode SongMountHandle::OnMsg(const SongMountCompletedMsg& msg) {
    if (msg.Song() != mSong) {
        return DataNode();
    }
    const bool success = msg.Success();
    const State state = success ? kMounted : kMountIdle;
    const bool changed = state != mState;
    mState = state;
    if (changed && success && mRequestMs > 0.0f) {
        static_cast<void>(TheTimeMgr->mRealTime.SplitMs());
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xF32780.
DataNode SongMountHandle::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    if (msg->Sym(1) == SongMountCompletedMsg::Type()) {
        const SongMountCompletedMsg completed(msg);
        return OnMsg(completed);
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xF32960.
DataNode SongMgr::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol hasSong;
    if (hasSong == Symbol()) {
        hasSong = Symbol("has_song");
    }
    if (type != hasSong) {
        return DataNode();
    }
    return DataNode(static_cast<int>(HasSong(msg->Sym(2))));
}
