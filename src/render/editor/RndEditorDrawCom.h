#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "math/geometry/Sphere.h"
#include "math/transform/Transform.h"
#include "utl/containers/LinkedList.h"
#include "utl/containers/VectorAdapter.h"

class ComMetaData;
class Frustum;
class PropRegistry;
class RndCameraContext;
class RndContext;
class Segment;
class Vector3;

// A part of an editor-drawn object that the editor selects on its own, such
// as a spline's control point. The map names the type; its two words are
// not decoded, so the field names are not in the reference map.
struct RndSubEditableId {
    unsigned long mFirst;
    unsigned long mSecond;

    // The id of no part, which the base class's segment test returns. Name
    // not in the reference map.
    static const RndSubEditableId sNone;  // 0x1A86EB0
};

static_assert(sizeof(RndSubEditableId) == 16);

// The base of the editor companions of drawable components, the
// "*EditorData" classes (render/RndEditorDrawCom.o, 0x461330-0x4616BE; the
// static initializer at 0x4616A0). Each draws its component's editor
// representation and answers the editor's picking queries; it requires the
// root's RndEditorDrawSetCom. Slots 41-58 are this class's; slot 41 is
// pure. The vtable at 0x1903D98 has 59 slots. The object is 48 bytes.
//
// The map's older build passes an ObjPtr first to every slot; this build
// does not. The map names slots 41, 42, 45-50 and 53-58; slots 43, 44, 51
// and 52 are newer, and their names rest only on their default bodies, so
// the evidence for them is weak. The defaults the binary shares with other
// classes (0x44C780, 0x43D6D0, 0x3BC540-0x3BC620) are defined here.
class RndEditorDrawCom : public Component {
public:
    // The run-time state (constructor 0x461370).
    struct RuntimeData {
        RuntimeData();  // 0x461370

        // The capability bits GetWorldSphere tests: 0x20 when the class
        // gives its own sphere, 0x40 when the draw node's sphere counts.
        // Its writer is not identified, so the name is weakly supported.
        unsigned long mCapabilityFlags;
        // The link in the draw set's list. Name not in the reference map.
        LinkedList::Node mDrawSetNode;
    };

    RndEditorDrawCom();  // 0x461330
    // The imprint's copy: the run-time state starts afresh. Inlined into
    // the subclasses' _Imprint. Not in the reference map.
    RndEditorDrawCom(const RndEditorDrawCom& other)
        : Component(other), mRuntimeData() {}
    // Slots 0-1: 0x461390, 0x4613C0. Unlinks the component from its draw
    // set.
    ~RndEditorDrawCom() override;

    // Slot 20 at 0x461670: nothing. The map's _PreDestroy(ObjPtr const&,
    // DestroyType).
    void _PreDestroy(DestroyType type) override;
    // Slot 29 at 0x461680: true.
    bool _OnResourcesLoaded() override;

    // Slot 41: the capability bits of the class (RuntimeData).
    virtual unsigned int _GetCapabilityFlagsImpl() const = 0;
    // Slot 42 at 0x44C780: whether the component draws for the camera when
    // `selected` says whether its object is selected. True here.
    virtual bool _FilterShowingImpl(const RndCameraContext* camera, bool selected) const {
        static_cast<void>(camera);
        static_cast<void>(selected);
        return true;
    }
    // Slot 43 at 0x43D6D0: whether the component draws while its object is
    // not selected; one subclass answers from an options component. False
    // here.
    virtual bool _IsShownUnselectedImpl() const {
        return false;
    }
    // Slot 44 at 0x3BC540: whether the editor may select the component.
    // True here.
    virtual bool _IsSelectableImpl() const {
        return true;
    }
    // Slot 45 at 0x3BC550: the component's own world sphere; false when it
    // has none. False here.
    virtual bool _GetWorldSphereImpl(const RndCameraContext* camera, Sphere& sphere) const {
        static_cast<void>(camera);
        static_cast<void>(sphere);
        return false;
    }
    // Slot 46: draws the editor representation; `subEditables` are the
    // selected parts.
    virtual void _EditorDrawImpl(
        RndContext& context,
        bool selected,
        const VectorAdapter<unsigned long>& subEditables);  // 0x461690
    // Slots 47-49 at 0x3BC560, 0x3BC570 and 0x3BC580: picking by a segment
    // (at `t`, and at `point` when it is given) and by a selection frustum.
    // Nothing intersects or is contained, and everything is excluded,
    // here.
    virtual bool _IntersectSegmentImpl(
        const RndCameraContext& camera,
        const Segment& segment,
        float& t,
        Vector3* point) const {
        static_cast<void>(camera);
        static_cast<void>(segment);
        static_cast<void>(t);
        static_cast<void>(point);
        return false;
    }
    virtual bool _FrustumContainsImpl(
        const RndCameraContext& camera,
        const Frustum& frustum) const {
        static_cast<void>(camera);
        static_cast<void>(frustum);
        return false;
    }
    virtual bool _FrustumExcludesImpl(
        const RndCameraContext& camera,
        const Frustum& frustum) const {
        static_cast<void>(camera);
        static_cast<void>(frustum);
        return true;
    }
    // Slot 50 at 0x3BC590: the number of parts. None here.
    virtual unsigned long _GetNumSubEditablesImpl() const {
        return 0;
    }
    // Slot 51 at 0x3BC5A0: whether the parts can be edited. False here.
    virtual bool _CanEditSubEditablesImpl() const {
        return false;
    }
    // Slot 52 at 0x3BC5B0: told that the parts changed. Nothing here.
    virtual void _OnSubEditablesChangedImpl() {}
    // Slots 53-55 at 0x3BC5C0, 0x3BC5E0 and 0x3BC5F0: a part's world
    // transform (the identity here), and moving and deleting parts
    // (nothing here).
    virtual Transform _GetSubEditableWorldXfmImpl(unsigned long index) const {
        static_cast<void>(index);
        return Transform::sID;
    }
    virtual void _SetSubEditableLocalXfmImpl(unsigned long index, const Transform& xfm) {
        static_cast<void>(index);
        static_cast<void>(xfm);
    }
    virtual void _DeleteSubEditablesImpl(const VectorAdapter<unsigned long>& indices) {
        static_cast<void>(indices);
    }
    // Slot 56 at 0x3BC600: the part a segment hits first, at `t`. None
    // here. The map's version returns an unsigned long.
    virtual RndSubEditableId _IntersectSegmentSubEditableImpl(
        const RndCameraContext& camera,
        const Segment& segment,
        float& t) const {
        static_cast<void>(camera);
        static_cast<void>(segment);
        static_cast<void>(t);
        return RndSubEditableId::sNone;
    }
    // Slots 57-58 at 0x3BC610 and 0x3BC620: a part against a selection
    // frustum. Not contained and excluded here.
    virtual bool _FrustumContainsSubEditable(
        unsigned long index,
        const RndCameraContext& camera,
        const Frustum& frustum) const {
        static_cast<void>(index);
        static_cast<void>(camera);
        static_cast<void>(frustum);
        return false;
    }
    virtual bool _FrustumExcludesSubEditable(
        unsigned long index,
        const RndCameraContext& camera,
        const Frustum& frustum) const {
        static_cast<void>(index);
        static_cast<void>(camera);
        static_cast<void>(frustum);
        return true;
    }

    // The world sphere the editor frames the component by: the class's
    // own sphere, the object's draw node sphere, or both combined, by the
    // capability bits. False when neither applies. The map's
    // GetWorldSphere(ObjPtr const&, RndCameraContext const*, Sphere&)
    // const.
    bool GetWorldSphere(const RndCameraContext* camera, Sphere& sphere) const;  // 0x461400

    // Describes the class for its subclasses: a debug class allowed in
    // drawable entities that requires the root's RndEditorDrawSetCom.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x4614C0

    RuntimeData mRuntimeData;
};

static_assert(offsetof(RndEditorDrawCom::RuntimeData, mDrawSetNode) == 8);
static_assert(offsetof(RndEditorDrawCom, mRuntimeData) == 24);
static_assert(sizeof(RndEditorDrawCom) == 48);
