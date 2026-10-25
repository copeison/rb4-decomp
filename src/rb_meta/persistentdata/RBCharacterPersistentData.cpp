// rb_meta/RBCharacterPersistentData.o (0xD4D520 to 0xD4FC1F). The object
// also holds eastl::vector<RBChar::RBCharDesc>::DoInsertValue (0xD4F9F0),
// which push_back reaches when the characters are full. The static
// initializer at 0xD4FBE0 sets the shared header globals every rb_meta
// object initializes; it is not modelled. UpdateCharacter, CheckCharUsage
// and _IsUserUsingChar are declared only (see the header).
#include "rb_meta/persistentdata/RBCharacterPersistentData.h"

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_world/characters/RBCharManager.h"
#include "utl/streams/RevisionStream.h"

// Reconstructed from eboot.elf at 0xD4D520.
RBCharacterPersistentData::RBCharacterPersistentData(RBProfile* profile)
    : mProfile(profile), mSavedCharIsPrefab(false) {
    mParent = profile;
}

// Reconstructed from eboot.elf at 0xD4D610. The deleting destructor is at
// 0xD4D6C0.
RBCharacterPersistentData::~RBCharacterPersistentData() {}

// Reconstructed from eboot.elf at 0xD4D6E0.
const char* RBCharacterPersistentData::_GetProfileName() const {
    if (mProfile != nullptr) {
        return mProfile->GetPlatformName();
    }
    return "NULL_PROFILE_POINTER!!!";
}

// Reconstructed from eboot.elf at 0xD4D700.
void RBCharacterPersistentData::Clear() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        ScopedCritSec lock(mCritSec);
        mCharacters.clear();
        mCharacters.reserve(kMaxCharacters);
        mSavedCharName = Symbol();
        mSavedCharIsPrefab = false;
        mCharUseInfo.clear();
        mCharUseInfo.reserve(kMaxCharacters);
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4D890. Save takes no lock.
void RBCharacterPersistentData::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 4, 0);
        revisionStream.BeginWrite();
        int count = static_cast<int>(mCharacters.size());
        revisionStream.WriteEndian(&count, sizeof(count));
        revisionStream << mSavedCharName;
        bool isPrefab = mSavedCharIsPrefab;
        revisionStream.Write(&isPrefab, sizeof(isPrefab));
        for (const RBChar::RBCharDesc* desc = mCharacters.begin(); desc != mCharacters.end(); ++desc) {
            revisionStream << *desc;
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4D9E0. A block that cannot be read
// rebuilds the use entries; one from before revision 3 or with more than
// kMaxCharacters characters leaves them as they were. The failures are
// logged with the profile's name.
void RBCharacterPersistentData::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        ScopedCritSec lock(mCritSec);
        RevisionStream revisionStream(stream, 4, 0);
        bool rebuildUses = true;
        if (revisionStream.BeginRead()) {
            mCharacters.clear();
            mCharacters.reserve(kMaxCharacters);
            if (revisionStream.mReadRevision < 3) {
                rebuildUses = false;
            } else {
                unsigned int count;
                revisionStream.ReadEndian(&count, sizeof(count));
                revisionStream >> Revision::AsOfOp<Symbol>(&mSavedCharName, 2);
                revisionStream >> Revision::AsOfOp<bool>(&mSavedCharIsPrefab, 2);
                if (count > kMaxCharacters) {
                    static_cast<void>(_GetProfileName());
                    rebuildUses = false;
                } else {
                    for (unsigned int i = 0; i < count; i++) {
                        RBChar::RBCharDesc desc;
                        revisionStream >> desc;
                        mCharacters.push_back(desc);
                    }
                }
            }
        } else {
            static_cast<void>(_GetProfileName());
        }
        if (rebuildUses) {
            mCharUseInfo.clear();
            mCharUseInfo.reserve(kMaxCharacters);
            _CreateCharUseVecAfterLoad();
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4DDD0.
void RBCharacterPersistentData::_CreateCharUseVecAfterLoad() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCharUseInfo.clear();
    mCharUseInfo.reserve(kMaxCharacters);
    for (RBChar::RBCharDesc* desc = mCharacters.begin(); desc != mCharacters.end(); ++desc) {
        _CreateCharUseEntryForChar(desc->mName);
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4DEE0.
void RBCharacterPersistentData::GetCharacterNames(eastl::vector<Symbol>& names) {
    ScopedCritSec lock(mCritSec);
    names.clear();
    names.reserve(kMaxCharacters);
    for (RBChar::RBCharDesc* desc = mCharacters.begin(); desc != mCharacters.end(); ++desc) {
        names.push_back(desc->mName);
    }
}

// Reconstructed from eboot.elf at 0xD4E0D0.
void RBCharacterPersistentData::GetCharacters(eastl::vector<RBChar::RBCharDesc>& characters) {
    ScopedCritSec lock(mCritSec);
    characters = mCharacters;
}

// Reconstructed from eboot.elf at 0xD4E220.
bool RBCharacterPersistentData::HasRoomForMoreCharacters() {
    ScopedCritSec lock(mCritSec);
    return mCharacters.size() < kMaxCharacters;
}

// Reconstructed from eboot.elf at 0xD4E280.
unsigned long RBCharacterPersistentData::GetCharacterCount() {
    ScopedCritSec lock(mCritSec);
    return mCharacters.size();
}

// Reconstructed from eboot.elf at 0xD4E2D0.
const RBChar::RBCharDesc* RBCharacterPersistentData::_FindCharacterByName(Symbol name) const {
    const RBChar::RBCharDesc* desc = mCharacters.begin();
    for (; desc != mCharacters.end(); ++desc) {
        if (desc->mName == name) {
            break;
        }
    }
    return desc;
}

// Reconstructed from eboot.elf at 0xD4E300.
RBChar::RBCharDesc* RBCharacterPersistentData::_FindCharacterByNameMutable(Symbol name) {
    RBChar::RBCharDesc* desc = mCharacters.begin();
    for (; desc != mCharacters.end(); ++desc) {
        if (desc->mName == name) {
            break;
        }
    }
    return desc;
}

// Reconstructed from eboot.elf at 0xD4E330.
bool RBCharacterPersistentData::CharacterExists(Symbol name) {
    ScopedCritSec lock(mCritSec);
    return _FindCharacterByName(name) != mCharacters.end();
}

// Reconstructed from eboot.elf at 0xD4E3B0.
bool RBCharacterPersistentData::GetCharacterByName(Symbol name, RBChar::RBCharDesc& desc) {
    ScopedCritSec lock(mCritSec);
    const RBChar::RBCharDesc* found = _FindCharacterByName(name);
    if (found != mCharacters.end()) {
        desc = *found;
        return true;
    }
    desc = RBChar::RBCharDesc();
    return false;
}

// Reconstructed from eboot.elf at 0xD4E520.
bool RBCharacterPersistentData::CreateCharacter(Symbol name, RBChar::RBCharDesc* desc) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    bool created = false;
    {
        ScopedCritSec lock(mCritSec);
        if (_FindCharacterByName(name) != mCharacters.end()) {
            static_cast<void>(_GetProfileName());
        } else if (mCharacters.size() == kMaxCharacters) {
            static_cast<void>(_GetProfileName());
        } else {
            if (desc != nullptr) {
                _CheckForDefaultPieces(*desc);
                mCharacters.push_back(*desc);
            } else {
                RBChar::RBCharDesc newDesc;
                newDesc.mName = name;
                mCharacters.push_back(newDesc);
            }
            _CreateCharUseEntryForChar(name);
            MakeDirty();
            created = true;
        }
    }
    MemPopHeap();
    return created;
}

// Reconstructed from eboot.elf at 0xD4E790. Unlike _GetProfileName, the log
// reads the profile without checking it.
void RBCharacterPersistentData::_CheckForDefaultPieces(const RBChar::RBCharDesc& desc) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    for (RBChar::RBCharOutfitSlot* slot = RBChar::gOutfitSlotVec.begin();
         slot != RBChar::gOutfitSlotVec.end(); ++slot) {
        if (RBChar::IsInstrumentOutfitSlot(*slot)) {
            continue;
        }
        const RBChar::RBCharOutfitPieceDesc& piece = desc.mOutfit.mPieces[*slot];
        if (piece.mName != Symbol() && piece.mIsDefault) {
            static_cast<void>(mProfile->GetPlatformName());
            static_cast<void>(RBChar::GetOutfitSlotName(*slot));
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4E870. An existing entry is logged and
// destroyed (_DestroyCharUseEntryForChar is inlined).
void RBCharacterPersistentData::_CreateCharUseEntryForChar(const Symbol& name) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    if (_GetCharUseInfoForChar(name) != nullptr) {
        static_cast<void>(_GetProfileName());
        _DestroyCharUseEntryForChar(name);
    }
    CharUseInfo info;
    info.mCharName = name;
    info.mInUse = false;
    info.mUserGuid = UserGuid();
    mCharUseInfo.push_back(info);
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4EBF0. The old character is looked up
// again before it is replaced; _DestroyCharUseEntryForChar is inlined.
bool RBCharacterPersistentData::RenameCharacter(Symbol oldName, Symbol newName,
                                                const RBChar::RBCharDesc* desc) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    bool renamed = false;
    {
        ScopedCritSec lock(mCritSec);
        RBChar::RBCharDesc* existing = _FindCharacterByNameMutable(oldName);
        if (existing == mCharacters.end()) {
            static_cast<void>(_GetProfileName());
        } else if (_FindCharacterByName(newName) == mCharacters.end()) {
            const RBChar::RBCharDesc* source = desc != nullptr ? desc : existing;
            _CheckForDefaultPieces(*source);
            RBChar::RBCharDesc renamedDesc = *source;
            renamedDesc.mName = newName;
            existing = _FindCharacterByNameMutable(oldName);
            if (existing == mCharacters.end()) {
                static_cast<void>(_GetProfileName());
            } else {
                *existing = renamedDesc;
                _DestroyCharUseEntryForChar(oldName);
                _CreateCharUseEntryForChar(renamedDesc.mName);
                MakeDirty();
                renamed = true;
            }
        }
    }
    MemPopHeap();
    return renamed;
}

// Reconstructed from eboot.elf at 0xD4EE60. Only the first entry of the
// name is erased.
void RBCharacterPersistentData::_DestroyCharUseEntryForChar(const Symbol& name) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    for (CharUseInfo* info = mCharUseInfo.begin(); info != mCharUseInfo.end(); ++info) {
        if (info->mCharName == name) {
            mCharUseInfo.erase(info);
            break;
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4EF00. _DestroyCharUseEntryForChar is
// inlined.
bool RBCharacterPersistentData::DeleteCharacter(Symbol name) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    bool deleted = false;
    {
        ScopedCritSec lock(mCritSec);
        RBChar::RBCharDesc* desc = _FindCharacterByNameMutable(name);
        if (desc != mCharacters.end()) {
            mCharacters.erase(desc);
            _DestroyCharUseEntryForChar(name);
            MakeDirty();
            deleted = true;
        } else {
            static_cast<void>(_GetProfileName());
        }
    }
    MemPopHeap();
    return deleted;
}

// Reconstructed from eboot.elf at 0xD4F080.
Symbol RBCharacterPersistentData::GetSavedCharName(bool& isPrefab) const {
    isPrefab = mSavedCharIsPrefab;
    return mSavedCharName;
}

// Reconstructed from eboot.elf at 0xD4F090.
Symbol RBCharacterPersistentData::GetSavedCharName() const {
    return mSavedCharName;
}

// Reconstructed from eboot.elf at 0xD4F0A0. Only the lookup among the
// profile's characters takes the lock; an unknown profile character is
// logged.
bool RBCharacterPersistentData::SetSavedCharName(Symbol name, bool isPrefab) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    bool saved = false;
    if (name == Symbol()) {
        mSavedCharName = name;
        mSavedCharIsPrefab = true;
        MakeDirty();
        saved = true;
    } else {
        bool found;
        if (isPrefab) {
            found = theCharManager->GetPrefabCharDesc(name) != nullptr;
        } else {
            {
                ScopedCritSec lock(mCritSec);
                found = _FindCharacterByName(name) != mCharacters.end();
            }
            if (!found) {
                static_cast<void>(_GetProfileName());
            }
        }
        if (found) {
            mSavedCharIsPrefab = isPrefab;
            mSavedCharName = name;
            MakeDirty();
            saved = true;
        }
    }
    MemPopHeap();
    return saved;
}

// Reconstructed from eboot.elf at 0xD4F1D0.
bool RBCharacterPersistentData::CharIsInUse(const Symbol& name) {
    ScopedCritSec lock(mCritSec);
    CharUseInfo* info = _GetCharUseInfoForChar(name);
    if (info != nullptr) {
        return info->mInUse;
    }
    static_cast<void>(_GetProfileName());
    return true;
}

// Reconstructed from eboot.elf at 0xD4F260.
RBCharacterPersistentData::CharUseInfo* RBCharacterPersistentData::_GetCharUseInfoForChar(
    const Symbol& name) {
    for (CharUseInfo* info = mCharUseInfo.begin(); info != mCharUseInfo.end(); ++info) {
        if (info->mCharName == name) {
            return info;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xD4F290. An unknown character is logged;
// one already in use fails quietly.
bool RBCharacterPersistentData::TryToMarkCharInUse(const Symbol& name, const UserGuid& user) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    bool marked = false;
    {
        ScopedCritSec lock(mCritSec);
        CharUseInfo* info = _GetCharUseInfoForChar(name);
        if (info == nullptr) {
            static_cast<void>(_GetProfileName());
        } else if (!info->mInUse) {
            info->mInUse = true;
            info->mUserGuid = user;
            marked = true;
        }
    }
    MemPopHeap();
    return marked;
}

// Reconstructed from eboot.elf at 0xD4F370.
bool RBCharacterPersistentData::TryToGetFirstNotInUseCharAndMarkIt(const UserGuid& user,
                                                                   Symbol& name) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    bool marked = false;
    {
        ScopedCritSec lock(mCritSec);
        for (CharUseInfo* info = mCharUseInfo.begin(); info != mCharUseInfo.end(); ++info) {
            if (!info->mInUse) {
                info->mInUse = true;
                info->mUserGuid = user;
                name = info->mCharName;
                marked = true;
                break;
            }
        }
    }
    MemPopHeap();
    return marked;
}

// Reconstructed from eboot.elf at 0xD4F440. An unknown character, a free one
// and one used by another user are logged and left alone.
void RBCharacterPersistentData::ClearCharInUse(const Symbol& name, const UserGuid& user) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        ScopedCritSec lock(mCritSec);
        CharUseInfo* info = _GetCharUseInfoForChar(name);
        if (info == nullptr) {
            static_cast<void>(_GetProfileName());
        } else if (!info->mInUse) {
            static_cast<void>(_GetProfileName());
            static_cast<void>(user.ToShortString());
        } else if (info->mUserGuid != user) {
            static_cast<void>(_GetProfileName());
            static_cast<void>(user.ToShortString());
            static_cast<void>(info->mUserGuid.ToShortString());
        } else {
            info->mInUse = false;
            info->mUserGuid = UserGuid();
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4F5B0.
void RBCharacterPersistentData::ClearAllCharsInUseForUser(const UserGuid& user) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        ScopedCritSec lock(mCritSec);
        for (CharUseInfo* info = mCharUseInfo.begin(); info != mCharUseInfo.end(); ++info) {
            if (info->mInUse && info->mUserGuid == user) {
                info->mInUse = false;
                info->mUserGuid = UserGuid();
            }
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD4F6D0.
bool RBCharacterPersistentData::UserIsUsingChar(const UserGuid& user, const Symbol& name) {
    ScopedCritSec lock(mCritSec);
    CharUseInfo* info = _GetCharUseInfoForChar(name);
    if (info != nullptr && info->mInUse) {
        return info->mUserGuid == user;
    }
    return false;
}
