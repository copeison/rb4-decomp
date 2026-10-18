#pragma once

#include "entity/core/Component.h"
#include "render/shaders/RndShaderEnums.h"
#include "utl/text/Symbol.h"

class EntityPtr;
class ObjPtr;
class RndContext;
class RndShaderGraph;
class TextStream;

// A node of a shader graph: a component on one object of the graph
// resource's entity (render/RndShaderNode.o). Only the vtable is modelled:
// RndShaderNode's own (0x1912100) has 61 slots, the Component's 41 and the
// 20 below, which no node class in this build reorders. The member layout
// and the method bodies are not reconstructed.
class RndShaderNode : public Component {
public:
    // Slot 41 at 0x512D80: "gTexSampler_" and the local variable name (or
    // "Node%x" of the object id). The map has it non-virtual.
    virtual const char* GetSamplerName() const;
    // Slot 42 at 0x517DB0, empty.
    virtual void _AddConnectionsImpl();
    // Slot 43 at 0x513550: the declaration of the local variable, then
    // slot 49 for the right-hand side.
    virtual void _PrintNodeImpl(
        RndShaderProgramType type,
        TextStream& stream,
        EntityPtr entity);
    // Slots 44-48 at 0x4DDC30-0x4DDC70: empty, 0, gNullStr, 0 and 0.
    virtual void _PrintDefinesImpl(TextStream& stream) const;
    virtual unsigned long _GetNumIncludesImpl() const;
    virtual const char* _GetIncludeImpl(unsigned long index) const;
    virtual unsigned int _GetInterpolantFlagsImpl() const;
    virtual unsigned int _GetComputedVariableFlagsImpl(
        RndShaderProgramType type) const;
    // Slot 49 at 0x517DC0, empty.
    virtual void _PrintRHS(
        RndShaderProgramType type,
        TextStream& stream,
        EntityPtr entity);
    // Slots 50-52 at 0x4DDC80-0x4DDCA0: false and two empty bodies.
    virtual bool _IsExposedImpl() const;
    virtual void _AddExposedPropertiesImpl(RndShaderGraph& graph) const;
    virtual void _AddGlobalsImpl(RndShaderGraph& graph) const;
    // Slot 53 at 0x513AD0.
    virtual void _AddTexturesImpl(
        RndShaderGraph& graph,
        RndShaderProgramType type) const;
    // Slots 54-56 at 0x4DDCB0-0x4DDCD0: 0.0, 0 and empty.
    virtual float _GetConversionPadValue() const;
    virtual int _GetDefaultVisMode() const;
    virtual void _SelectForNodePreviewsImpl(RndContext& context);
    // Slot 57 at 0x4DDCE0. The binary's body returns its second argument,
    // so this build's parameters differ from the map's signature, which
    // this follows.
    virtual unsigned long _FixupOutputConnectionImpl(unsigned long index) const;
    // Slots 58-59 at 0x4DDCF0, 0x4DDD00, empty.
    virtual void _PreRebuildGraphImpl(RndShaderGraph& graph);
    virtual void _PostPasteImpl(ObjPtr& object);
    // Slot 60 at 0x4DDD10, empty. The overrides found (0x4ED7F0, 0x4F0F10)
    // reset a cached index at +408 to -1. Name not in the reference map;
    // weak evidence.
    virtual void _ResetCachedStateImpl();

    // Both "RndShaderNode". Every node class registers under the class
    // name, which the graph's lookups compare with each component's base
    // id. The map has sId only.
    static Symbol sId;         // 0x1A8C530
    static Symbol sClassName;  // 0x1A8C538
};
