#include "render/meshes/RndVertex.h"

#include <cstring>

RndVertexType VertexTypeFromName(const char* name) {
    if (std::strcmp(name, "Color") == 0) {
        return kVertexColor;
    }
    if (std::strcmp(name, "ColorTex") == 0) {
        return kVertexColorTex;
    }
    if (std::strcmp(name, "Unskinned") == 0) {
        return kVertexUnskinned;
    }
    if (std::strcmp(name, "Skinned") == 0) {
        return kVertexSkinned;
    }
    if (std::strcmp(name, "PosOnly") == 0) {
        return kVertexPosOnly;
    }
    if (std::strcmp(name, "Particle") == 0) {
        return kVertexParticle;
    }
    if (std::strcmp(name, "UnskinnedCompressed") == 0) {
        return kVertexUnskinnedCompressed;
    }
    if (std::strcmp(name, "SkinnedCompressed") == 0) {
        return kVertexSkinnedCompressed;
    }
    return kVertexInvalid;
}
