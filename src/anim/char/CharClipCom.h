#pragma once

#include <cstddef>

#include "anim/char/CharClipResource.h"
#include "anim/core/AnimInstID.h"
#include "anim/core/AnimResource.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class AnimDriverCom;
class DataArray;
class RndSkeletonPoseCom;

// The component that defines a character clip on its object
// (anim/CharClipCom.o, 0x2E5C90-0x2EFF7F; class id "CharClip"): a range of
// a source FBX animation with its beat alignment, relativity, blending and
// bone masks, the clip's playback units, looping and events. The clip
// itself is the CharClipResource "<layer>_clips/<entity>_<serial>.charclip"
// next to the entity; the component loads it, rebuilds it when its
// configuration changes in the editor (an empty step in this build) and can
// preview it on the entity's skeleton. Clip blend trees, jukeboxes and the
// state graph's animation nodes play it. The vtable at 0x18F5A48 has 41
// slots. The object is 432 bytes. Newer than the reference map: the class
// and its members are named from the binary's strings and uses.
class CharClipCom : public Component {
public:
    CharClipCom();  // 0x2E5C90
    // Copies the configuration, the units, the looping and the events for an
    // imprint; the rest starts empty. Inlined into _Imprint (0x2EACE0).
    CharClipCom(const CharClipCom& other);
    // Slots 0-1: 0x2E5E50, 0x2E6020.
    ~CharClipCom() override;

    // Slot 2: records the components "component_created" reports while
    // the clip is rebuilt; every other message is unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x2EAB70
    Symbol GetId() const override;          // slot 4: 0x2EAC60
    Symbol GetClassName() const override;   // slot 5: 0x2EAC70
    int CurrentRev() const override;        // slot 7: 0x2EAC80
    bool IsA(Symbol type) const override;   // slot 8: 0x2EACA0
    Component* AsComponent() override;      // slot 9: 0x2EACD0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x2EACE0
    // Slot 19: a new clip has two beat/frame pairs.
    void _PostCreate() override;  // 0x2E6040
    // Slot 20: stops the preview when the object or the component goes
    // away, and forgets it otherwise.
    void _PreDestroy(DestroyType type) override;  // 0x2E6050
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x2EAFB0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x2EAFC0
    // Slot 29: loads the clip resource, hands it the object, units and
    // looping, matches the bone masks to the skeleton, takes the source and
    // relative frame ranges after a file change and sorts the events.
    bool _OnResourcesLoaded() override;  // 0x2E9AE0
    // Slot 33: empty.
    void _Poll() override;  // 0x2EA310
    // Slot 38: rebuilds the clip after a frame change or while it plays.
    void _EditPoll() override;  // 0x2EA320

    // Rebuilds the clip when the configuration differs from the one it was
    // built from: the resource's components created meanwhile are loaded
    // and entered again, and the preview takes the new range. StateNodeAnimate
    // and the blend trees call it on the clips they play. Name not in the
    // reference map; the evidence is weak.
    void Refresh();  // 0x2EA450
    // Gives the preview playback the clip's units, alignment and range.
    // Inlined into Refresh and the "playback_units" callback. Name not in
    // the reference map.
    void _UpdatePreviewRange();  // 0x2EA870
    // The actions "disable_all_bones" and "enable_all_bones": every bone
    // mask masks or keeps every channel. Names not in the reference map.
    void DisableAllBones();  // 0x2EA940
    void EnableAllBones();   // 0x2EA990
    // The AnimDriverCom and the RndSkeletonPoseCom of the entity's root.
    // Names not in the reference map.
    AnimDriverCom* _GetDriver() const;               // 0x2EA3F0
    RndSkeletonPoseCom* _GetSkeletonPose() const;    // 0x2EA9D0
    // The "component_created" handler: records the class (node 3) of the
    // created component. Handle inlines the dispatch. Name not in the
    // reference map.
    DataNode _OnComponentCreated(DataArray* msg);  // 0x2EAA30

    // Registers the class (InitComponentClass). Emitted in anim/AnimInit.o
    // at 0x2AFE30, which Anim::Init calls.
    static void Init();
    // The class factory. Emitted in anim/AnimInit.o at 0x2B73A0; the
    // placement constructor InitComponentClass installs is at 0x2B73D0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x2E6360

    // "CharClip", constructed by the static initializer at 0x2EFEC0, which
    // first sets a shared header's null object id to -1 (0x19F8A98).
    static Symbol sId;  // 0x19F8AA0
    // Also "CharClip". Name not in the reference map.
    static Symbol sClassName;           // 0x19F8AA8
    static PropRegistry sPropRegistry;  // 0x19F8AB0
    static ComMetaData sMetaData;       // 0x19F8B50
    // The clip previewing on its entity, at most one at a time: turning on
    // "preview" turns it off on the previous one. Name not in the reference
    // map.
    static CharClipCom* sPreviewCom;  // 0x19F8CF8

    // The edited configuration, its properties registered at their own
    // names.
    CharClipConfig mConfig;
    // "playback_units": a CharAnim unit (real time, beats, beat aligned).
    int mPlaybackUnits;
    // "is_looping".
    bool mIsLooping;
    // "events": what the clip's playbacks send, sorted by frame when the
    // resources load.
    PropArray<AnimResource::Event> mEvents;
    // Constructed empty and never read or copied. The name is weak.
    ResourcePath mClipPath;
    // The clip resource the component builds and plays. Name not in the
    // reference map.
    ResourcePtr<CharClipResource> mResource;
    // "hide_disabled_bones".
    bool mHideDisabledBones;
    // Set when "source_fbx" or "relative_source_fbx" changes; the next
    // resource load takes the new animation's frame range.
    bool mSourceChanged;
    bool mRelativeSourceChanged;
    // The configuration the resource was last built from.
    CharClipConfig mBuiltConfig;
    // "preview", "pause" and "frame": the editor's preview playback on the
    // entity, which is not saved.
    bool mPreview;
    bool mPause;
    float mPreviewFrame;
    AnimInstID mPreviewID;
    // The component classes "component_created" reported during a rebuild.
    eastl::vector<Symbol> mCreatedComponents;
    // Set when "start_frame" or "end_frame" changes; the next edit poll
    // rebuilds the clip.
    bool mNeedsRefresh;
};

static_assert(offsetof(CharClipCom, mConfig) == 24);
static_assert(offsetof(CharClipCom, mPlaybackUnits) == 160);
static_assert(offsetof(CharClipCom, mIsLooping) == 164);
static_assert(offsetof(CharClipCom, mEvents) == 168);
static_assert(offsetof(CharClipCom, mClipPath) == 208);
static_assert(offsetof(CharClipCom, mResource) == 216);
static_assert(offsetof(CharClipCom, mHideDisabledBones) == 224);
static_assert(offsetof(CharClipCom, mBuiltConfig) == 232);
static_assert(offsetof(CharClipCom, mPreview) == 368);
static_assert(offsetof(CharClipCom, mPreviewFrame) == 372);
static_assert(offsetof(CharClipCom, mPreviewID) == 376);
static_assert(offsetof(CharClipCom, mCreatedComponents) == 392);
static_assert(offsetof(CharClipCom, mNeedsRefresh) == 424);
static_assert(sizeof(CharClipCom) == 432);
