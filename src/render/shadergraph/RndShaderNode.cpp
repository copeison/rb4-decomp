// render/RndShaderNode.o: only the class symbols are reconstructed.
#include "render/shadergraph/RndShaderNode.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x5185A0 and never read. Names not in the reference map.
[[maybe_unused]] int gShaderNodeGroupSizeUnused = -1;  // 0x1A8C520
[[maybe_unused]] int gShaderNodeGroupSize2D = 8;       // 0x1A8C524
[[maybe_unused]] int gShaderNodeGroupSize3D = 4;       // 0x1A8C528

}  // namespace

// The object's first statics, in the order of its static initializer
// (0x5185A0). sPropRegistry (0x1A8C540) and sMetaData (0x1A8C5E0) follow
// and are not modelled.
Symbol RndShaderNode::sId("RndShaderNode");
Symbol RndShaderNode::sClassName("RndShaderNode");
