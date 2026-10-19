#pragma once

#include <cstddef>
#include <functional>

#include "anim/char/CharBoneChannelMask.h"
#include "anim/core/AnimResource.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"
#include "utl/time/TimeMgr.h"

class BinStream;
class CharAnim;
class CharSkeletonResource;
class Entity;

// What a CharClipCom defines a clip as: a frame range of a source FBX
// animation, its alignment to the beat, what it plays relative to, how it
// blends and the bones it drives (136 bytes, no vtable). The component keeps
// the edited configuration and the one its resource was last built from;
// the resource keeps the one it was cached with. Newer than the reference
// map; the type's name comes from the struct registration
// "CharClipConfig::BeatTrackEntry". Field names follow the properties.
struct CharClipConfig {
    // One beat/frame pair of "beat_track": the (zero-based) beat a frame of
    // the clip falls on (8 bytes).
    struct BeatTrackEntry {
        BeatTrackEntry();  // 0x2F0030

        float mBeat;
        float mFrame;
    };

    // An empty clip: no source, frames 0 to 0, layered blending, and
    // `beatTrackSize` zeroed beat/frame pairs (none for the edited
    // configuration, two for the built ones).
    explicit CharClipConfig(unsigned long beatTrackSize);  // 0x2EFF80

    // Whether every field and every beat/frame pair and bone mask match.
    // Name not in the reference map.
    bool operator==(const CharClipConfig& other) const;  // 0x2F0080

    // "source_fbx": the FBX file of the source animation.
    ResourcePath mSourceFbx;
    // "start_frame" and "end_frame" (exclusive): the clip's range within the
    // source animation.
    int mStartFrame;
    int mEndFrame;
    // "beat_track": two beat/frame pairs that align a beat-synced clip.
    PropArray<BeatTrackEntry> mBeatTrack;
    // "relativity": a CharAnimCom::Relative (not relative, relative to the
    // bind pose, relative to "relative_source_fbx").
    int mRelativity;
    // "relative_source_fbx" and "relative_frame": the animation and frame
    // the clip plays relative to.
    ResourcePath mRelativeSourceFbx;
    int mRelativeFrame;
    // "blend_mode".
    AnimResource::BlendMode mBlendMode;
    // "subtree_index": the blend-tree slot the clip plays in.
    unsigned long mSubTreeIndex;
    // "blend_from_root": blend relative to the char_root.
    bool mBlendFromRoot;
    // "bones": the channels of each skeleton bone the clip drives.
    PropArray<CharBoneChannelMask> mBones;
};

static_assert(sizeof(CharClipConfig::BeatTrackEntry) == 8);
static_assert(offsetof(CharClipConfig, mStartFrame) == 8);
static_assert(offsetof(CharClipConfig, mBeatTrack) == 16);
static_assert(offsetof(CharClipConfig, mRelativity) == 56);
static_assert(offsetof(CharClipConfig, mRelativeSourceFbx) == 64);
static_assert(offsetof(CharClipConfig, mRelativeFrame) == 72);
static_assert(offsetof(CharClipConfig, mBlendMode) == 76);
static_assert(offsetof(CharClipConfig, mSubTreeIndex) == 80);
static_assert(offsetof(CharClipConfig, mBlendFromRoot) == 88);
static_assert(offsetof(CharClipConfig, mBones) == 96);
static_assert(sizeof(CharClipConfig) == 136);

// Write and read a configuration, revision 2: the paths, the frames, the
// first two beat/frame pairs, the relativity, the blend settings, the bone
// masks and the subtree index (revision 1 added it, revision 2 the beat
// track). Names not in the reference map.
BinStream& operator<<(BinStream& stream, const CharClipConfig& config);  // 0x2F01A0
BinStream& operator>>(BinStream& stream, CharClipConfig& config);        // 0x2F0380

// A blend-tree-compatible character clip, as a CharClipCom defines it
// (anim/CharClipResource.o, 0x2EFF80-0x2F135F; extension "charclip",
// category "Animations"): the configuration it was built from, the CharAnim
// it holds, the playback units and looping its component sets, the frame
// ranges of its source and relative animations, and the object of the
// component that defines it. The animation interface is the second base,
// at offset 48. The vtables are at 0x18F67D0 (30 slots: the resource's 13,
// then the animation overriders in declaration order) and 0x18F68D0 (the
// animation interface's 18). The object is 232 bytes. Newer than the
// reference map; field names are inferred.
class CharClipResource : public Resource, public AnimResource {
public:
    CharClipResource();  // 0x2F0670
    // Slots 10-11: 0x2F0720, 0x2F0810.
    ~CharClipResource() override;

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x2F11E0
    Symbol GetId() const override;                   // slot 1: 0x2F11F0
    bool IsA(Symbol type) const override;            // slot 2: 0x2F1290
    // Slot 3: reads the cached file alone; a clip is never built from a
    // source file here.
    void LoadFile() override;  // 0x2F0BD0
    // Slots 4-5: revision 1 of the cached file is the configuration, the
    // CharAnim when there is one and the four frame bounds.
    bool Load(BinStream& stream, bool cached) override;  // 0x2F0A70
    void Save(BinStream& stream, bool cached) override;  // 0x2F0950
    // Slot 6: without a CharAnim.
    bool Fail() const override;  // 0x2F0CB0

    // The animation interface (AnimResource slots 0 and 2-17); thunks at
    // 0x2F12E0, 0x2F0D20, 0x2F12F0, 0x2F0D70, 0x2F0D90, 0x2F0DB0,
    // 0x2F0DE0, 0x2F0E10, 0x2F0E30, 0x2F0E50, 0x2F11A0, 0x2F11C0,
    // 0x2F0EB0, 0x2F0F50, 0x2F0FF0, 0x2F10D0 and 0x2F1180.
    Resource* AsResource() override;  // slot 13: 0x2F12C0
    // The object of the defining CharClipCom in the entity, or null.
    GameObject* GetObject(Entity* entity) const override;  // slot 14: 0x2F0CE0
    // The CharAnim.
    void* GetCharData() const override;              // slot 15: 0x2F12D0
    bool IsLooping() const override;                 // slot 16: 0x2F0D60
    void SetLooping(bool looping) override;          // slot 17: 0x2F0D80
    // The CharAnim's range: 0 to its last frame.
    float GetStartFrame() const override;            // slot 18: 0x2F0DA0
    float GetEndFrame() const override;              // slot 19: 0x2F0DC0
    // The range in the source animation.
    float GetAuthoredStartFrame() const override;    // slot 20: 0x2F0E00
    float GetAuthoredEndFrame() const override;      // slot 21: 0x2F0E20
    // Beats for every playback unit but real time.
    TimeUnits GetUnits() const override;             // slot 22: 0x2F0E40
    // The CharAnim's blend mode and blend-tree slot.
    BlendMode GetBlendMode() const override;         // slot 23: 0x2F1190
    unsigned long GetSubTreeIndex() const override;  // slot 24: 0x2F11B0
    // Beats through the CharAnim's beat map, seconds at its frame rate.
    float GetTimeFromFrame(float frame, TimeUnits units) const override;  // slot 25: 0x2F0E60
    float GetFrameFromTime(float time, TimeUnits units) const override;   // slot 26: 0x2F0F00
    // On the beat timeline, the beats an aligned clip keeps in phase with:
    // 1, 4 or 2 for "Beat Aligned", "Downbeat Aligned" and "Two Beat
    // Aligned"; 0 otherwise.
    float GetTimeAlignment(TimeUnits units) const override;  // slot 27: 0x2F0FA0
    // The defining CharClipCom's events.
    const PropArray<Event>* GetEvents(Entity* entity) const override;  // slot 28: 0x2F1040
    // The clip's start frame in the source animation.
    float GetEventFrameOffset() const override;  // slot 29: 0x2F1170

    // The class id, created on first use and inlined into its users, such
    // as StateNodeAnimate::GetAnimResource (0x70F0F0) and GetId (0x2F11F0).
    // The local static is at 0x19F7D38.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("CharClipResource");
        }
        return id;
    }

    // Registers the class as a resource. Emitted in anim/AnimInit.o at
    // 0x2AFCC0, which Anim::Init calls.
    static void Init();
    // The factory. Emitted in anim/AnimInit.o at 0x2B7370.
    static Resource* _Create();
    static void _Init(ResourceMetaData& metaData);  // 0x2F0830

    // Builds the clip from the configuration on the skeleton. Empty in this
    // build: the tools build clips, and the game only loads them. Name not
    // in the reference map.
    void Create(const CharClipConfig& config, CharSkeletonResource* skeleton);  // 0x2F11D0
    // The configuration the clip was built from. No caller was found. Name
    // not in the reference map.
    CharClipConfig& GetConfig();  // 0x2F0CC0
    // Sets the playback units ("playback_units" of the CharClipCom). Name
    // not in the reference map.
    void SetUnits(int units);  // 0x2F0CD0

    // Constructed by the static initializer at 0x2F1300, which first sets a
    // shared header's null object id to -1 (0x19F8D90).
    static ResourceMetaData sMetaData;  // 0x19F8D98
    // Sets sBuildCallback. No caller was found. Name not in the reference
    // map.
    static void SetBuildCallback(const std::function<void(CharClipResource*)>& callback);  // 0x2F0600
    // A callback the static initializer constructs empty and only
    // SetBuildCallback writes; nothing calls it, so its type and name are
    // weakly supported. Name not in the reference map.
    static std::function<void(CharClipResource*)> sBuildCallback;  // 0x19F8DF0

    CharClipConfig mConfig;
    CharAnim* mAnim;
    // "playback_units" of the CharClipCom, one of CharAnim's units: 0
    // "Real-time", 1 "Beat", 2 "Beat Aligned", 3 "Downbeat Aligned", 4 "Two
    // Beat Aligned".
    int mUnits;
    bool mLooping;
    // The frame ranges of the source animation and of the relative
    // animation, which bound the component's frames.
    int mSourceStartFrame;
    int mSourceEndFrame;
    int mRelativeStartFrame;
    int mRelativeEndFrame;
    // The object whose CharClipCom defines the clip.
    GameObjectId mObjectId;
};

static_assert(offsetof(CharClipResource, mConfig) == 56);
static_assert(offsetof(CharClipResource, mAnim) == 192);
static_assert(offsetof(CharClipResource, mUnits) == 200);
static_assert(offsetof(CharClipResource, mLooping) == 204);
static_assert(offsetof(CharClipResource, mSourceStartFrame) == 208);
static_assert(offsetof(CharClipResource, mRelativeStartFrame) == 216);
static_assert(offsetof(CharClipResource, mObjectId) == 224);
static_assert(sizeof(CharClipResource) == 232);
