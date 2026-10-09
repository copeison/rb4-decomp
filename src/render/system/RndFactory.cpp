#include "render/system/RndFactory.h"

#include "render/core/system/render_system_globals.h"

RndFactory* TheRndFactory() {
    return rb4::render_system_factory(*rb4::render_system_instance());
}
