// anim/AnimResource.o (0x32C760 to 0x32CF92). The interface's vtable
// (0x18FA340) is emitted here.
#include "anim/core/AnimResource.h"

#include "entity/core/Component.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"

namespace {

// The path of the "frame" of the event whose member the accessor's path
// names: the last node is dropped and the one before it becomes "frame"
// (a local static of each accessor, 0x19FBA20 and 0x19FBA30). Inlined
// into both accessors. Name not in the reference map.
PropPath EventFramePath(const PropAccessorArgs& args, Symbol& frameName) {
    if (frameName == Symbol()) {
        frameName = Symbol("frame");
    }
    PropPath path = *static_cast<const PropPath*>(args.mPath);
    path.PopNode();
    path.mNodes[path.mSize - 1] = frameName;
    return path;
}

// Reports a "frame" that is missing or that does not convert: the
// component's error name and the path's text are made and dropped.
// Inlined into both accessors. Name not in the reference map.
void ReportEventFrame(const Component& component, const PropPath& path) {
    static_cast<void>(component.MakeErrorName());
    char text[512] = {};
    path.ToString(text, sizeof(text));
}

}  // namespace

// Reconstructed from eboot.elf at 0x32C760.
eastl::vector<AllowedValue<int>> AnimResource::GetBlendModeAllowedValues() {
    eastl::vector<AllowedValue<int>> values;
    values.push_back(AllowedValue<int>{
        kBlendAdditive,
        String("Additive"),
        String("These clips will add in their weight and joint data, all will be normalized at the end")});
    values.push_back(AllowedValue<int>{
        kBlendLayered,
        String("Layered"),
        String("These clips will blend themselves in OVER any previous data, superseding any older clips "
               "with same blend order, or any clip with a lower blend order")});
    return values;
}

// Reconstructed from eboot.elf at 0x32CF10.
float AnimResource::GetAuthoredStartFrame() const {
    return GetStartFrame();
}

// Reconstructed from eboot.elf at 0x32CF20.
float AnimResource::GetAuthoredEndFrame() const {
    return GetEndFrame();
}

// Reconstructed from eboot.elf at 0x32CF30.
float AnimResource::EventFrameToFrame(float frame) const {
    return frame - GetEventFrameOffset();
}

// Reconstructed from eboot.elf at 0x32CF60.
float AnimResource::FrameToEventFrame(float frame) const {
    return GetEventFrameOffset() + frame;
}

// Reconstructed from eboot.elf at 0x32CF90.
GameObject* AnimResource::GetObject(Entity* entity) const {
    static_cast<void>(entity);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x32C8D0.
float GetEventFrame(const PropAccessorArgs& args) {
    static Symbol frameName;
    const PropPath path = EventFramePath(args, frameName);
    Component* const component = args.mComponent;
    float frame = 0.0f;
    void* storage = component;
    const PropInfo* const info = component->_GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr || !_GetProp(*component, path, *info, storage, &frame, kPropertyFloat)) {
        ReportEventFrame(*component, path);
    }
    return frame;
}

// Reconstructed from eboot.elf at 0x32CBF0.
void SetEventFrame(const PropAccessorArgs& args, float frame) {
    static Symbol frameName;
    const PropPath path = EventFramePath(args, frameName);
    Component* const component = args.mComponent;
    void* storage = component;
    const PropInfo* const info = component->_GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr || !_SetProp(*component, path, *info, storage, &frame, kPropertyFloat, false)) {
        ReportEventFrame(*component, path);
    }
}
