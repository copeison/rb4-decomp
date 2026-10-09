#include "render/system/RndFactory.h"

#include "render/system/RndDevice.h"

RndFactory* TheRndFactory() {
    return TheRndDevice()->mFactory;
}
