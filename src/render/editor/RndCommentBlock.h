#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "math/geometry/Rect.h"
#include "utl/text/Symbol.h"

// The comment block of the state and shader graph editors
// (render/RndCommentBlock.o; class id "RndCommentBlock", registry builder
// at 0x4AE420, vtable 0x19084A8): a rectangle with a comment, inside a
// folder. Only the class symbol and the fields the state graph reads are
// declared; the class is not reconstructed. The object is 56 bytes.
class RndCommentBlock : public Component {
public:
    static Symbol sId;  // 0x1A89B20
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1A89B28

    // Field names are not in the reference map; they follow the
    // properties.
    // "comments": the block's comment.
    Symbol mComments;
    // "rect": the block's rectangle in the editor.
    Hmx::Rect mRect;
    // "folder": the object of the StateNodeFolder the block sits in.
    GameObjectId mFolder;
};

static_assert(offsetof(RndCommentBlock, mComments) == 24);
static_assert(offsetof(RndCommentBlock, mRect) == 32);
static_assert(offsetof(RndCommentBlock, mFolder) == 48);
static_assert(sizeof(RndCommentBlock) == 56);
