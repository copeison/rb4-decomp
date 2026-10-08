#include "render/platform/orbis/system/orbis_render_factory.h"

#include "core/memory/engine_memory.h"
#include "render/platform/orbis/system/orbis_render_factory_adapters.h"

namespace rb4 {

OrbisRenderFactory* orbis_render_factory_create() {
    auto* storage = render_allocate(sizeof(OrbisRenderFactory));
    auto* factory = static_cast<OrbisRenderFactory*>(storage);
    orbis_render_factory_install_vtable(*factory);
    return factory;
}

}  // namespace rb4
