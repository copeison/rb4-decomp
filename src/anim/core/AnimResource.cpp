// anim/AnimResource.o (0x32C760 to 0x32CF92). The interface's vtable
// (0x18FA340) is emitted here. Not reconstructed: the "frame" accessors of
// the event properties at 0x32C8D0 and 0x32CBF0, which read and write an
// event's frame through the component's registry for the resources'
// "frame_editor_view".
#include "anim/core/AnimResource.h"

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
