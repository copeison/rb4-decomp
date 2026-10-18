// stategraph/StateGraphResource.o (0x706C40 to 0x70840B). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/resources/StateGraphResource.h"

#include <cstring>

#include "entity/core/Entity.h"
#include "entity/core/EntityConstants.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"
#include "stategraph/base/StateBase.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/nodes/StateNodeBlendTreeBase.h"
#include "stategraph/nodes/StateNodeRoot.h"
#include "stategraph/transitions/StateTransition.h"
#include "utl/containers/Vector.h"
#include "utl/streams/BinStream.h"
#include "utl/streams/MemStream.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x1AB2C58, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

}  // namespace

// The object's statics, in the order of its static initializer (0x7083D0).
ResourceMetaData StateGraphResource::sMetaData;

// Reconstructed from eboot.elf at 0x706C40.
void StateGraphResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("state"));
    metaData.mTypeFlags[0] = false;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x706D40.
StateGraphResource::StateGraphResource() : mRev(0) {}

// Reconstructed from eboot.elf at 0x706D70.
Entity* StateGraphResource::CreateEntity() {
    Entity* const entity = EntityResource::CreateEntity();
    entity->GetRoot()->CreateComponent(StateNodeRoot::sClassName, false);
    return entity;
}

// Reconstructed from eboot.elf at 0x706DB0.
void StateGraphResource::Save(BinStream& stream, bool cached) {
    EntityResource::Save(stream, cached);
    const int rev = 2;
    stream.WriteEndian(&rev, sizeof(rev));
}

// Reconstructed from eboot.elf at 0x706E10. The revision follows the
// entity from entity revision 25 on. The old root node's properties move
// through a memory stream into the new StateNodeRoot.
bool StateGraphResource::_LoadEntity(BinStream& stream, bool cached) {
    if (!EntityResource::_LoadEntity(stream, cached)) {
        return false;
    }
    mRev = 0;
    if (gEntityRev >= 25) {
        stream.ReadEndian(&mRev, sizeof(mRev));
        if (mRev > 0) {
            return true;
        }
    }
    GameObject* const root = mEntity->GetRoot();
    if (root->GetCom<StateNodeRoot>() != nullptr) {
        return true;
    }
    StateNode* const node = root->GetBaseCom<StateNode>();
    MemStream properties(false);
    _StorePropState(*node, PropPath(), properties, nullptr);
    root->DestroyComponent(StateNode::sId, false);
    Component* const replacement = root->CreateComponent(StateNodeRoot::sClassName, false);
    properties.Seek(0, kSeekBegin);
    _ApplyStoredPropState(*replacement, PropPath(), properties, true);
    return true;
}

// Reconstructed from eboot.elf at 0x7070C0.
bool StateGraphResource::_PostLoad(BinStream& stream, bool cached) {
    static_cast<void>(stream);
    static_cast<void>(cached);
    if (mRev > 0) {
        return true;
    }
    Entity* const entity = mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        StateNode* const node = object->GetBaseCom<StateNode>();
        if (node != nullptr) {
            const unsigned int count = node->mTransitions.size();
            for (unsigned long index = 0; index < count; ++index) {
                node->mRuntime.mTransitions[index]->SetOrigin(object->mId);
            }
        }
        StateTransition* const transition = object->GetBaseCom<StateTransition>();
        if (transition != nullptr) {
            transition->_LoadBlendCurve();
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x707200.
StateNodeRoot* StateGraphResource::GetRootNode() const {
    return mEntity->GetRoot()->GetCom<StateNodeRoot>();
}

// Reconstructed from eboot.elf at 0x707260.
bool StateGraphResource::IsRootNode(const StateNode* node) const {
    return GetRootNode() == node;
}

// Reconstructed from eboot.elf at 0x7072C0.
bool StateGraphResource::StartsOnEnter() const {
    return mEntity->GetRoot()->GetExistingCom<StateNodeRoot>()->mStartOnEnter;
}

// Reconstructed from eboot.elf at 0x707310.
void StateGraphResource::SetStartsOnEnter(bool startOnEnter) {
    GetRootNode()->SetStartOnEnter(startOnEnter);
}

// Reconstructed from eboot.elf at 0x707380.
GameObject* StateGraphResource::CreateGraphObject(Symbol className, const char* prefix) {
    GameObject* const object = mEntity->CreateObject(0, 0);
    Component* const component = object->CreateComponent(className, false);
    _PostCreateObject(static_cast<StateBase*>(component), object, prefix);
    return object;
}

// Reconstructed from eboot.elf at 0x7073F0.
void StateGraphResource::_PostCreateObject(StateBase* component, GameObject* object, const char* prefix) {
    object->SetName(GetBaseName(component->GetId(), prefix));
    object->_ReenterComponents();
}

// Reconstructed from eboot.elf at 0x707430.
GameObject* StateGraphResource::CreateGraphObject(StateCondition::Type type, const char* prefix) {
    GameObject* const object = mEntity->CreateObject(0, 0);
    StateCondition* const condition = StateCondition::Create(object, type);
    _PostCreateObject(condition, object, prefix);
    return object;
}

// Reconstructed from eboot.elf at 0x707490.
GameObject* StateGraphResource::FindGraphObject(GameObjectId id) const {
    if (id.mId == gNullObjectId.mId) {
        return nullptr;
    }
    return mEntity->GetObject(id);
}

// Reconstructed from eboot.elf at 0x7074D0.
Symbol StateGraphResource::GetBaseName(Symbol className, const char* prefix) {
    if (className == StateNode::sId) {
        return Symbol("No-Op");
    }
    Symbol base = className;
    String name(className.Str());
    const unsigned long found = name.find(prefix);
    if (found != FixedString::npos) {
        base = Symbol(name.c_str() + strlen(prefix) + found);
    }
    String trimmed(base.Str());
    const unsigned long underscore = trimmed.find("_");
    if (underscore != FixedString::npos) {
        const String head = trimmed.substr(0, underscore);
        base = Symbol(head.c_str());
    }
    return base;
}

// Reconstructed from eboot.elf at 0x7075F0.
void StateGraphResource::DestroyGraphObject(GameObjectId id) {
    SafeDestroyObject(mEntity, id);
}

// Reconstructed from eboot.elf at 0x707630. Every transition object must
// hold a transition.
StateTransition* StateGraphResource::FindTransitionWithCondition(GameObjectId condition) const {
    Entity* const entity = mEntity;
    eastl::vector<GameObjectId> descendants;
    for (GameObject* object = entity->NextObject(nullptr, StateTransition::sId); object != nullptr;
         object = entity->NextObject(object, StateTransition::sId)) {
        StateTransition* const transition = object->GetBaseCom<StateTransition>();
        if (transition->mCondition.mId == condition.mId) {
            return transition;
        }
        const StateCondition* const root = transition->mRuntime.mCondition;
        if (root == nullptr) {
            continue;
        }
        descendants.clear();
        root->GetDescendants(descendants);
        for (const GameObjectId& descendant : descendants) {
            if (descendant.mId == condition.mId) {
                return transition;
            }
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x7077C0. Every node object must hold a
// node.
StateNode* StateGraphResource::FindParentBlendTree(GameObjectId node) const {
    Entity* const entity = mEntity;
    eastl::vector<GameObjectId> children;
    for (GameObject* object = entity->NextObject(nullptr, StateNode::sId); object != nullptr;
         object = entity->NextObject(object, StateNode::sId)) {
        StateNode* const tree = object->GetExistingBaseCom<StateNode>();
        if (!tree->IsA(StateNodeBlendTreeBase::sClassName)) {
            continue;
        }
        if (tree->mObject->mId.mId == node.mId) {
            return tree;
        }
        children.clear();
        tree->GetChildNodes(children);
        for (const GameObjectId& child : children) {
            if (child.mId == node.mId) {
                return tree;
            }
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x707920.
Entity* StateGraphResource::GetContextEntityPrototype() const {
    return mInlineOwner != nullptr ? mInlineOwner->mEntity : nullptr;
}

// Reconstructed from eboot.elf at 0x707940.
void StateGraphResource::FixupAll() {
    Entity* const entity = mEntity;
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        for (unsigned int index = 0; index < object->mComs.size(); ++index) {
            Component* const component = object->mComs[index].mCom;
            if (component != nullptr && component->IsA(StateBase::sClassName)) {
                static_cast<StateBase*>(component)->Fixup();
                break;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x707A00. A transition counts as used
// when a node lists it and its target holds a node; its condition and
// every descendant of the condition count with it.
bool StateGraphResource::GetUnusedObjects(eastl::set<unsigned int>& unused) const {
    Entity* const entity = mEntity;
    eastl::set<unsigned int> used;
    for (GameObject* object = entity->NextObject(nullptr, StateNode::sId); object != nullptr;
         object = entity->NextObject(object, StateNode::sId)) {
        const StateNode* const node = object->GetBaseCom<StateNode>();
        for (unsigned int index = 0; index < node->mTransitions.size(); ++index) {
            const GameObjectId id = node->mTransitions[index];
            if (id.mId == gNullObjectId.mId) {
                continue;
            }
            const StateTransition* const transition =
                entity->GetObject(id)->GetBaseCom<StateTransition>();
            const GameObjectId target = transition->mTargetState;
            if (target.mId == gNullObjectId.mId) {
                continue;
            }
            const GameObject* const targetObject = entity->GetObject(target);
            if (targetObject == nullptr || targetObject->GetBaseCom<StateNode>() == nullptr) {
                continue;
            }
            used.insert(id.mId);
            const GameObjectId condition = transition->mCondition;
            if (condition.mId == gNullObjectId.mId) {
                continue;
            }
            used.insert(condition.mId);
            eastl::vector<GameObjectId> descendants;
            entity->GetObject(condition)->GetBaseCom<StateCondition>()->GetDescendants(descendants);
            for (const GameObjectId& descendant : descendants) {
                if (descendant.mId != gNullObjectId.mId) {
                    used.insert(descendant.mId);
                }
            }
        }
    }
    for (GameObject* object = entity->BeginObject(); object != nullptr;
         object = entity->NextObject(object, Symbol())) {
        const unsigned int id = object->mId.mId;
        if (object->GetBaseCom<StateTransition>() == nullptr &&
            object->GetBaseCom<StateCondition>() == nullptr) {
            continue;
        }
        if (used.find(id) != used.end()) {
            continue;
        }
        unused.insert(id);
    }
    return unused.mnSize != 0;
}

// Reconstructed from eboot.elf at 0x708270.
ResourceMetaData* StateGraphResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x708280.
Symbol StateGraphResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x708320.
bool StateGraphResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr; metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}
