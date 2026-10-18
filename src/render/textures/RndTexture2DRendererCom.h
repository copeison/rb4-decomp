#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "utl/containers/LinkedList.h"

class RndContext;
class RndTexRendererGroup;
class RndTexture2D;

// The component that renders a 2D texture ("Texture2DRenderer": "Dynamically
// renders a 2d texture"), from a material or a scene's main camera, into
// textures that other objects use (0x6908B0-0x694F7C, its _Init at
// 0x691850). Renderers that can share their buffers join one
// RndTexRendererGroup of their entity's RndTexRendererMgr, which draws them.
// Only the members the manager uses are declared; the class is not
// reconstructed, and its virtual slots are not declared. Name not in the
// reference map; it follows the class id.
class RndTexture2DRendererCom : public Component {
public:
    // Reaches the link that keeps a renderer in its group. Name not in the
    // reference map.
    class GroupNode {
    public:
        static LinkedList::Node& ToNode(RndTexture2DRendererCom& renderer) {
            return renderer.mGroupNode;
        }
        static RndTexture2DRendererCom* FromNode(LinkedList::Node* node) {
            return reinterpret_cast<RndTexture2DRendererCom*>(
                reinterpret_cast<char*>(node) -
                offsetof(RndTexture2DRendererCom, mGroupNode));
        }
    };

    // The texture of the index's output. Name not in the reference map.
    RndTexture2D* GetTexture(unsigned long index) const;  // 0x691130
    // Whether the renderer can draw with the group's buffers: its format
    // and its first texture's size match those of the group's first
    // renderer. Name not in the reference map.
    bool CanJoinGroup(const RndTexRendererGroup& group) const;  // 0x691810
    // Draws the renderer's material or scene on the context when it is
    // enabled and its draw frequency allows this frame. Name not in the
    // reference map.
    void Draw(RndContext& context);  // 0x691140

    unsigned char mUnmodelled[176];
    // The group the renderer belongs to, or null.
    RndTexRendererGroup* mGroup;
    unsigned char mUnmodelled2[8];
    LinkedList::Node mGroupNode;
};

static_assert(offsetof(RndTexture2DRendererCom, mGroup) == 200);
static_assert(offsetof(RndTexture2DRendererCom, mGroupNode) == 216);
