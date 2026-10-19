// stategraph/ClipBlendTreeCom.o (0x7FC520 to 0x80368F). The class's
// registration and factory are emitted in stategraph/StateGraph.o, and so
// are the copies of PropArray<ClipBlendTreeCom::Node>'s element functions
// (0x6FF040-0x6FF1F0). The object also holds BlendTreeBaseCom's
// superclass helper.
#include "stategraph/blendtrees/ClipBlendTreeCom.h"

#include <math.h>
#include <new>

#include "anim/char/CharClipCom.h"
#include "anim/char/CharClipResource.h"
#include "anim/core/AnimOptions.h"
#include "anim/drivers/AnimDriver.h"
#include "anim/drivers/AnimDriverCom.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "stategraph/conditions/StateConditionTimeElapsed.h"
#include "utl/containers/Sort.h"
#include "utl/time/TimeMgr.h"

namespace {

// The id of no object, at 0x1ABE6A8, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The tree whose property a tag function or an accessor describes. Name
// not in the reference map.
ClipBlendTreeCom& Tree(const TagArgs& args) {
    return const_cast<ClipBlendTreeCom&>(static_cast<const ClipBlendTreeCom&>(args.mComponent));
}
ClipBlendTreeCom& Tree(const PropAccessorArgs& args) {
    return *static_cast<ClipBlendTreeCom*>(args.mComponent);
}

// The child ("nodes" i ...) a tag function of a child's property
// describes. Name not in the reference map.
ClipBlendTreeCom::Node& TagNode(const TagArgs& args) {
    return Tree(args).mNodes[args.mPath.mNodes[1].mValue];
}

}  // namespace

// The object's statics, in the order of its static initializer (0x803550).
Symbol ClipBlendTreeCom::sId("ClipBlendTree");
Symbol ClipBlendTreeCom::sClassName("ClipBlendTree");
PropRegistry ClipBlendTreeCom::sPropRegistry;
ComMetaData ClipBlendTreeCom::sMetaData;

namespace {

// The edit values of the parameters that are set, shared by every tree
// that names the parameter: "parameter_value" writes them and the edit
// mode's instances read them. Its destructor is at 0x7FC520. Name not in
// the reference map.
eastl::map<PropRef, float> gEditParameterValues;  // 0x1ABE908

}  // namespace

// Reconstructed from eboot.elf at 0x7FC530.
ClipBlendTreeCom::ClipBlendTreeCom()
    : mParameter(),
      mTimeUnits(kNumTimeUnits),
      mNormalizePeriod(false),
      mAlignPeriod(false),
      mPeriod(1.0f),
      mNodes(),
      mEditParameterValue(0.0f) {}

// Reconstructed from eboot.elf at 0x7FC5D0.
ClipBlendTreeCom::Node::Node() : mType(kChildClip), mObject{0xFFFFFFFFU}, mValue(0.0f), mSpeed(1.0f) {}

// Reconstructed from eboot.elf at 0x7FC5F0. The timing properties show only
// on the tree a node runs (IsRootBlendTree). A child's "object" is a clip in
// the context entity or a subtree in the graph; the "-" button beside a
// child removes it.
void ClipBlendTreeCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<Node>(Symbol("ClipBlendTree::Node"));
    ClipBlendTreeCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    PropRefMetadata& parameter = TypeSpecificMetadata(
        registry.RegisterProp(
            "parameter", offsetof(ClipBlendTreeCom, mParameter), kPropertyPropRef, sizeof(PropRef)),
        prototype.mParameter);
    parameter.mRefDefault = prototype.mParameter;
    static_cast<void>(Symbol("prop"));
    parameter.mHelp = String("The parameter controlling this animation blending.");
    parameter.mAllowedTypes = {kPropertyFloat};
    // At 0x8008F0.
    parameter.mEntity._Set([](const TagArgs& args) { return Tree(args)._GetContextEntityPrototype(); });

    const float valueType = 0.0f;
    FloatMetadata& value =
        TypeSpecificMetadata(registry.RegisterProp("parameter_value", -1, kPropertyFloat, 0), valueType);
    static_cast<void>(Symbol("prop"));
    value.mHelp = String("A value to use for the specified parameter while in edit mode.");
    value.mSlider = true;
    value.mHistory = false;
    // At 0x800640.
    value.mGetter = [](const PropAccessorArgs& args) {
        const ClipBlendTreeCom& tree = Tree(args);
        return tree.GetParameterValue(tree._GetContextEntityPrototype(), false);
    };
    // At 0x800670.
    value.mSetter = [](const PropAccessorArgs& args) {
        ClipBlendTreeCom& tree = Tree(args);
        const float newValue = *static_cast<const float*>(args.mValue);
        if (!tree.mParameter.IsComplete()) {
            tree.mEditParameterValue = newValue;
            return;
        }
        gEditParameterValues[tree.mParameter] = newValue;
    };
    // At 0x800D20.
    value.mReadOnly._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return true;
    });
    // The range of the children's values; at 0x800E20 and 0x800F30.
    value.mMin._Set([](const TagArgs& args) {
        const ClipBlendTreeCom& tree = Tree(args);
        return tree.mNodes.size() != 0 ? tree.mNodes[0].mValue : 0.0f;
    });
    value.mMax._Set([](const TagArgs& args) {
        const ClipBlendTreeCom& tree = Tree(args);
        return tree.mNodes.size() != 0 ? tree.mNodes[tree.mNodes.size() - 1].mValue : 0.0f;
    });
    // Fewer digits for a wider range; at 0x801050.
    value.mDecimalPlaces._Set([](const TagArgs& args) {
        const ClipBlendTreeCom& tree = Tree(args);
        const unsigned int count = tree.mNodes.size();
        if (count == 0) {
            return 4;
        }
        const float first = tree.mNodes[0].mValue;
        const float last = tree.mNodes[count - 1].mValue;
        if (first == last) {
            return 4;
        }
        return 4 - static_cast<int>(log10f(fabsf(last - first)));
    });

    IntMetadata<int>& units = static_cast<IntMetadata<int>&>(*registry
            .RegisterProp("time_units", offsetof(ClipBlendTreeCom, mTimeUnits), kPropertyInt, 4)
            .mMetadata.Get());
    units.mDefault = prototype.mTimeUnits;
    static_cast<void>(Symbol("prop"));
    units.mHelp = String("The time units this blend tree will run on.");
    units.mAllowedValues.mMode = TagBase<eastl::vector<AllowedValue<int>>>::kModeFunction;
    units.mAllowedValues.mFunction = gTimeUnitsAllowedValues;
    // At 0x8011A0.
    units.mIsHidden._Set([](const TagArgs& args) { return !Tree(args).IsRootBlendTree(); });

    BoolMetadata& normalize = TypeSpecificMetadata(
        registry.RegisterProp(
            "normalize_period", offsetof(ClipBlendTreeCom, mNormalizePeriod), kPropertyBool, 1),
        prototype.mNormalizePeriod);
    normalize.mDefault = prototype.mNormalizePeriod;
    static_cast<void>(Symbol("prop"));
    normalize.mHelp = String(
        "Should all animations in this blend tree be normalized to loop over the same period of time, "
        "regardless of their intrinsic durations and speed multipliers?");
    // At 0x8012A0.
    normalize.mIsHidden._Set([](const TagArgs& args) { return !Tree(args).IsRootBlendTree(); });

    FloatMetadata& period = TypeSpecificMetadata(
        registry.RegisterProp("period", offsetof(ClipBlendTreeCom, mPeriod), kPropertyFloat, 4),
        prototype.mPeriod);
    period.mDefault = prototype.mPeriod;
    static_cast<void>(Symbol("prop"));
    period.mHelp = String("The period over which this blend tree will loop all of its animations.");
    // At 0x8013A0 and 0x8014A0.
    period.mReadOnly._Set([](const TagArgs& args) { return !Tree(args).mNormalizePeriod; });
    period.mIsHidden._Set([](const TagArgs& args) { return !Tree(args).IsRootBlendTree(); });
    period.mMin = 0.0001f;
    // The time units' name after the number; at 0x8015A0.
    period.mUnits._Set([](const TagArgs& args) {
        const ClipBlendTreeCom& tree = Tree(args);
        const float length = tree.mPeriod;
        const char* name;
        switch (tree._GetTimeUnits()) {
        case kTimeUnitsSeconds:
            name = length != 1.0f ? " seconds" : " second";
            break;
        case kTimeUnitsBeats:
            name = length != 1.0f ? " beats" : " beat";
            break;
        case kTimeUnitsUISeconds:
            name = length != 1.0f ? " UI seconds" : " UI second";
            break;
        case kTimeUnitsTutorialSeconds:
            name = length != 1.0f ? " tutorial seconds" : " tutorial second";
            break;
        default:
            return Symbol();
        }
        return Symbol(name);
    });

    BoolMetadata& align = TypeSpecificMetadata(
        registry.RegisterProp("align_period", offsetof(ClipBlendTreeCom, mAlignPeriod), kPropertyBool, 1),
        prototype.mAlignPeriod);
    align.mDefault = prototype.mAlignPeriod;
    static_cast<void>(Symbol("prop"));
    align.mHelp = String(
        "Should the normalized animation cycle align to the specified timeline? For example, should a 4-beat "
        "normalized cycle start on the downbeat? Note that non-integral normalized periods will cause phase "
        "shifting relative to the timeline!");
    // At 0x801770 and 0x801870.
    align.mReadOnly._Set([](const TagArgs& args) { return !Tree(args).mNormalizePeriod; });
    align.mIsHidden._Set([](const TagArgs& args) { return !Tree(args).IsRootBlendTree(); });

    PropInfo& nodes = registry.RegisterPropArray(
        "nodes", offsetof(ClipBlendTreeCom, mNodes), sizeof(PropArray<Node>));
    ArrayMetadata& nodesMetadata = static_cast<ArrayMetadata&>(*nodes.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    nodesMetadata.mHelp = String(
        "The set of animations and subtrees controlled by this blend tree. Click the minus (-) button on an "
        "element to remove it.");
    nodesMetadata.mFixedSize = true;
    nodesMetadata.mExtraFlag1 = false;
    nodesMetadata.mExtraFlag2 = false;
    // The array's own insertion and removal do nothing: the actions add
    // children and the "-" button removes them. At 0x800780 and 0x800790.
    nodesMetadata.mInsert = [](const PropAccessorArgs& args) { static_cast<void>(args); };
    void (*const removeHook)(const PropAccessorArgs&) = [](const PropAccessorArgs& args) {
        static_cast<void>(args);
    };
    nodesMetadata.mHook2 = reinterpret_cast<void*>(removeHook);
    PropRegistry::RegisterPropArrayItem(nodes, kPropertyStruct, sizeof(Node));
    GroupMetadata& item = static_cast<GroupMetadata&>(*nodesMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    Node nodePrototype;
    item.mHelp = String(
        "An animation or subtree controlled by this blend tree. Click the minus (-) button to remove this "
        "element.");
    item.mExtraFlag1 = false;
    item.mExtraFlag2 = false;
    // At 0x801970.
    item.mDisplayName._Set(
        [](const TagArgs& args) { return String(TagNode(args).mType == kChildClip ? "Clip" : "Subtree"); });
    item.mInlineAction = new PropMetadata::InlineAction();
    item.mInlineAction->mLabel = "-";
    item.mInlineAction->mEnabled = true;
    // At 0x801AB0.
    item.mInlineAction->mAction = [](const TagArgs& args) {
        Tree(args).RemoveChild(args.mPath.mNodes[1].mValue);
    };
    item.mKind = 1;
    PropRegistry& members = item.mRegistry;

    IntMetadata<int>& type = static_cast<IntMetadata<int>&>(
        *members.RegisterProp("type", offsetof(Node, mType), kPropertyInt, 4).mMetadata.Get());
    type.mDefault = nodePrototype.mType;
    static_cast<void>(Symbol("prop"));
    type.mIsHidden = true;

    GameObjectIdMetadata& object = TypeSpecificMetadata(
        members.RegisterProp("object", offsetof(Node, mObject), kPropertyGameObjectId, 4), nodePrototype.mObject);
    object.mDefault = nodePrototype.mObject;
    static_cast<void>(Symbol("prop"));
    object.mHelp = String(
        "An element to be blended. This can be either a single CharClip or a BlendTree that acts as a subtree.");
    object.mAllowsThis = false;
    // At 0x801BB0.
    object.mDisplayName._Set(
        [](const TagArgs& args) { return String(TagNode(args).mType == kChildClip ? "Clip" : "Subtree"); });
    // At 0x801CF0 and 0x801E00.
    object.mExtraFlag1._Set([](const TagArgs& args) { return TagNode(args).mType == kChildClip; });
    object.mExtraFlag2._Set([](const TagArgs& args) { return TagNode(args).mType == kChildClip; });
    // A subtree's object is the one _AddChild created; at 0x801F10.
    object.mReadOnly._Set([](const TagArgs& args) { return TagNode(args).mType != kChildClip; });
    // At 0x802020.
    object.mRequiredComs._Set([](const TagArgs& args) {
        return eastl::vector<Symbol>{TagNode(args).mType == kChildClip ? CharClipCom::sId : sId};
    });
    // A clip is in the context entity, a subtree in the graph; at 0x8021D0.
    object.mEntity._Set([](const TagArgs& args) {
        const ClipBlendTreeCom& tree = Tree(args);
        if (TagNode(args).mType != kChildClip) {
            return tree.mObject->mEntity;
        }
        return tree._GetContextEntityPrototype();
    });
    // At 0x8007A0.
    object.mOnChanged = [](const PropAccessorArgs& args) { Tree(args)._SendModified(false); };

    FloatMetadata& nodeValue = TypeSpecificMetadata(
        members.RegisterProp("value", offsetof(Node, mValue), kPropertyFloat, 4), nodePrototype.mValue);
    nodeValue.mDefault = nodePrototype.mValue;
    static_cast<void>(Symbol("prop"));
    nodeValue.mHelp = String("The parameter value at which this node should take full effect.");

    FloatMetadata& speed = TypeSpecificMetadata(
        members.RegisterProp("speed", offsetof(Node, mSpeed), kPropertyFloat, 4), nodePrototype.mSpeed);
    speed.mDefault = nodePrototype.mSpeed;
    static_cast<void>(Symbol("prop"));
    speed.mHelp = String("The speed at which this animation should play back when in full effect.");
    // A normalized tree sets the speeds itself; at 0x8022F0.
    speed.mReadOnly._Set([](const TagArgs& args) { return Tree(args).mNormalizePeriod; });

    GroupMetadata& actions =
        static_cast<GroupMetadata&>(*registry.RegisterProp("actions", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    actions.mHelp = String("Various actions that can be performed on the blend tree structure.");
    actions.mKind = 1;

    ActionMetadata& addClip =
        static_cast<ActionMetadata&>(*actions.mRegistry.RegisterAction("add_clip").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    addClip.mHelp = String("Add a clip to the blend tree.");
    addClip.mDisplayName = String("+Clip");
    // At 0x8023F0.
    addClip.mAction = [](const TagArgs& args) { Tree(args).AddChild(kChildClip); };

    ActionMetadata& addSubtree =
        static_cast<ActionMetadata&>(*actions.mRegistry.RegisterAction("add_subtree").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    addSubtree.mHelp = String("Add a subtree to the blend tree.");
    addSubtree.mDisplayName = String("+Tree");
    // At 0x8024F0.
    addSubtree.mAction = [](const TagArgs& args) { Tree(args).AddChild(kChildBlendTree); };

    ActionMetadata& sort = static_cast<ActionMetadata&>(*actions.mRegistry.RegisterAction("sort").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    sort.mHelp = String("Sort the elements of the subtree by their parameter values in ascending order.");
    sort.mDisplayName = String("Sort");
    // At 0x8025F0, with eastl::sort's copies at 0x802820.
    sort.mAction = [](const TagArgs& args) {
        ClipBlendTreeCom& tree = Tree(args);
        eastl::sort(tree.mNodes.begin(), tree.mNodes.end(), [](const Node& a, const Node& b) {
            return a.mValue < b.mValue;
        });
        tree._SendModified(false);
    };
    // Unavailable once the values ascend; at 0x802CC0.
    sort.mReadOnly._Set([](const TagArgs& args) {
        const ClipBlendTreeCom& tree = Tree(args);
        const unsigned long count = tree.mNodes.size();
        if (count < 2) {
            return true;
        }
        for (unsigned long index = 1; index < count; ++index) {
            if (tree.mNodes[index - 1].mValue > tree.mNodes[index].mValue) {
                return false;
            }
        }
        return true;
    });

    ActionMetadata& span = static_cast<ActionMetadata&>(*actions.mRegistry.RegisterAction("span").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    span.mHelp = String(
        "Update the parameter values of each node so that they are distributed evenly between the the first "
        "node the last node.");
    span.mDisplayName = String("Span");
    // At 0x802E00.
    span.mAction = [](const TagArgs& args) {
        ClipBlendTreeCom& tree = Tree(args);
        const unsigned long count = tree.mNodes.size();
        if (count < 3) {
            return;
        }
        float value = tree.mNodes[0].mValue;
        const float step = (tree.mNodes[count - 1].mValue - value) / static_cast<float>(count - 1);
        if (!(step >= 0.0f)) {
            return;
        }
        for (unsigned long index = 1; index < count - 1; ++index) {
            value += step;
            tree.mNodes[index].mValue = value;
        }
        tree._SendModified(false);
    };
    // Unavailable once the values are evenly spaced; at 0x802F80.
    span.mReadOnly._Set([](const TagArgs& args) {
        const ClipBlendTreeCom& tree = Tree(args);
        const unsigned long count = tree.mNodes.size();
        if (count < 3) {
            return true;
        }
        float value = tree.mNodes[0].mValue;
        const float step = (tree.mNodes[count - 1].mValue - value) / static_cast<float>(count - 1);
        if (!(step >= 0.0f)) {
            return true;
        }
        for (unsigned long index = 1; index < count - 1; ++index) {
            value += step;
            if (fabsf(tree.mNodes[index].mValue - value) > 0.0001f) {
                return false;
            }
        }
        return true;
    });

    BlendTreeBaseCom::_InitAsSuperclass(registry, metadata);
    metadata.mDescription =
        "Manages blending between a set of animations or subtrees of animations based on some specified "
        "criteria.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mAliases.push_back(Symbol("CharBlendTree"));
    metadata.mAliases.push_back(Symbol("BlendTree"));
}

// Reconstructed from eboot.elf at 0x7FE810. The deleting destructor is at
// 0x8001D0.
ClipBlendTreeCom::~ClipBlendTreeCom() {}

// Reconstructed from eboot.elf at 0x7FE8A0.
bool ClipBlendTreeCom::IsClipNode(const TagArgs& args) const {
    return mNodes[args.mPath.mNodes[1].mValue].mType == kChildClip;
}

// Reconstructed from eboot.elf at 0x7FE8C0. An unreadable property gives 0;
// the report is compiled out.
float ClipBlendTreeCom::GetParameterValue(Entity* entity, bool editMode) const {
    if (editMode) {
        if (!mParameter.IsComplete()) {
            return mEditParameterValue;
        }
        return gEditParameterValues[mParameter];
    }
    float value = 0.0f;
    GameObject* const object = mParameter.Obj(entity);
    if (object == nullptr) {
        return value;
    }
    Component* const com = object->FindCom(mParameter.mCom);
    if (com == nullptr) {
        return value;
    }
    void* storage = com;
    const PropInfo* const info = com->_GetPropRegistry().FindProp(mParameter.mPath, &storage);
    if (info == nullptr || !_GetProp(*com, mParameter.mPath, *info, storage, &value, kPropertyFloat)) {
        static_cast<void>(com->MakeErrorName());
        char path[512] = {};
        mParameter.mPath.ToString(path, sizeof(path));
    }
    return value;
}

// Reconstructed from eboot.elf at 0x7FEC50.
int ClipBlendTreeCom::_GetTimeUnits() const {
    if (mTimeUnits == kNumTimeUnits) {
        const AnimDriverCom* const driver =
            _GetContextEntityPrototype()->GetRoot()->GetCom<AnimDriverCom>();
        if (driver != nullptr) {
            return driver->mDefaultTimeline;
        }
    }
    return mTimeUnits;
}

// Reconstructed from eboot.elf at 0x7FECD0.
void ClipBlendTreeCom::_Refresh() {
    for (const Node& node : mNodes) {
        if (node.mType == kChildBlendTree) {
            if (node.mObject.mId == gNullObjectId.mId) {
                continue;
            }
            const GameObject* const object = mObject->mEntity->GetObject(node.mObject);
            if (object == nullptr) {
                continue;
            }
            ClipBlendTreeCom* const child = object->GetCom<ClipBlendTreeCom>();
            if (child != nullptr) {
                child->Refresh();
            }
        } else {
            Entity* const entity = _GetContextEntityPrototype();
            if (node.mObject.mId == gNullObjectId.mId) {
                continue;
            }
            const GameObject* const object = entity->GetObject(node.mObject);
            if (object == nullptr) {
                continue;
            }
            CharClipCom* const clip = object->GetCom<CharClipCom>();
            if (clip != nullptr) {
                clip->Refresh();
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x7FEE50.
unsigned int ClipBlendTreeCom::_GetNumChildren() const {
    return mNodes.size();
}

// Reconstructed from eboot.elf at 0x7FEE60.
int ClipBlendTreeCom::_GetChildType(unsigned long index) const {
    return mNodes[index].mType;
}

// Reconstructed from eboot.elf at 0x7FEE70.
GameObjectId ClipBlendTreeCom::_GetChildObject(unsigned long index) const {
    return mNodes[index].mObject;
}

// Reconstructed from eboot.elf at 0x7FEE80. The subtree's object is named
// after the tree's.
void ClipBlendTreeCom::_AddChild(int type) {
    Node node;
    node.mType = type;
    node.mObject = gNullObjectId;
    node.mValue = 0.0f;
    node.mSpeed = 1.0f;
    const unsigned int count = mNodes.size();
    if (count != 0) {
        node.mValue = count == 1 ? mNodes[0].mValue + 1.0f
                                 : mNodes[count - 1].mValue * 2.0f - mNodes[count - 2].mValue;
    }
    if (type == kChildBlendTree) {
        Entity* const entity = mObject->mEntity;
        GameObject* const object = entity->CreateObject(0, 0);
        object->CreateComponent(sClassName, false);
        object->SetName(GenerateUniqueObjectName(mObject->mName, entity, false));
        object->_ReenterComponents();
        node.mObject = object->mId;
    }
    mNodes._Insert(mNodes.size(), &node);
}

// Reconstructed from eboot.elf at 0x7FEFA0.
void ClipBlendTreeCom::_RemoveChild(unsigned long index) {
    if (mNodes[index].mType == kChildBlendTree) {
        SafeDestroyObject(mObject->mEntity, mNodes[index].mObject);
    }
    mNodes.Destruct(1, mNodes.ElementAt(index));
    if (static_cast<unsigned int>(index) < mNodes.mSize - 1) {
        mNodes.Move(mNodes.mSize - 1 - index, mNodes.ElementAt(index), mNodes.ElementAt(index + 1));
    }
    --mNodes.mSize;
}

// Reconstructed from eboot.elf at 0x7FF060. The path is a guarded local
// static (0x1ABE940, guard 0x1ABEA08).
const PropPath& ClipBlendTreeCom::_GetChildObjectPath() const {
    static const PropPath path{
        PropPath::Node{} = Symbol("nodes"), PropPath::Node{} = 0UL, PropPath::Node{} = Symbol("object")};
    return path;
}

// Reconstructed from eboot.elf at 0x7FF180.
ClipBlendTreeCom::Instance* ClipBlendTreeCom::_CreateInstance(Entity* entity, bool editMode) {
    return new Instance(this, entity, editMode);
}

// Reconstructed from eboot.elf at 0x7FF1D0. Seconds and beats convert at
// the tempo (beats per minute).
float ClipBlendTreeCom::_GetScale(int units, float rate) const {
    float total = 0.0f;
    unsigned long count = 0;
    const int treeUnits = _GetTimeUnits();
    _AccumulateClipLengths(total, count, treeUnits, 1.0f);
    if (units != kNumTimeUnits && (units == kTimeUnitsBeats) != (treeUnits == kTimeUnitsBeats)) {
        const float beatsPerSecond = rate * (1.0f / 60.0f);
        total = treeUnits == kTimeUnitsBeats ? total / beatsPerSecond : total * beatsPerSecond;
    }
    if (count == 0) {
        return 0.0f;
    }
    return total / static_cast<float>(count);
}

// Reconstructed from eboot.elf at 0x7FF2D0.
void ClipBlendTreeCom::_AccumulateClipLengths(float& total, unsigned long& count, int units, float speed) const {
    for (const Node& node : mNodes) {
        const float nodeSpeed = speed * node.mSpeed;
        if (node.mType == kChildBlendTree) {
            if (node.mObject.mId == gNullObjectId.mId) {
                continue;
            }
            const GameObject* const object = mObject->mEntity->GetObject(node.mObject);
            if (object == nullptr) {
                continue;
            }
            const ClipBlendTreeCom* const child = object->GetCom<ClipBlendTreeCom>();
            if (child != nullptr) {
                child->_AccumulateClipLengths(total, count, units, nodeSpeed);
            }
        } else {
            Entity* const entity = _GetContextEntityPrototype();
            if (node.mObject.mId == gNullObjectId.mId) {
                continue;
            }
            const GameObject* const object = entity->GetObject(node.mObject);
            if (object == nullptr) {
                continue;
            }
            const CharClipCom* const clip = object->GetCom<CharClipCom>();
            if (clip == nullptr) {
                continue;
            }
            const CharClipResource* const resource = clip->mResource.Get();
            if (resource == nullptr || resource->Fail()) {
                continue;
            }
            const TimeUnits timeline = static_cast<TimeUnits>(units);
            const float end = resource->GetTimeFromFrame(resource->GetEndFrame(), timeline);
            const float start = resource->GetTimeFromFrame(resource->GetStartFrame(), timeline);
            total += (end - start) / nodeSpeed;
            ++count;
        }
    }
}

// Reconstructed from eboot.elf at 0x7FF530. The driver is the one on the
// entity's root object.
ClipBlendTreeCom::Instance::Instance(ClipBlendTreeCom* tree, Entity* entity, bool editMode)
    : mTree(tree),
      mEntity(entity),
      mDriver(entity->GetRoot()->GetCom<AnimDriverCom>()),
      mEditMode(editMode),
      mWeight(1.0f),
      mPhase(0.0f),
      mCycles(0),
      mTreeStates(),
      mClips() {}

// Reconstructed from eboot.elf at 0x7FF6A0. No caller in this build; the
// map's node insertion inlines it.
ClipBlendTreeCom::Instance::ClipState::ClipState() : mAnim(), mWeight(0.0f), mSpeed(0.0f) {}

// Reconstructed from eboot.elf at 0x7FF6C0. A normalized tree advances by
// 1/period per unit of time, or follows the clock's position in the period
// when aligned; otherwise each clip adds its speed over its length. Every
// clip with a weight plays at the tree's phase; the others stop.
void ClipBlendTreeCom::Instance::Poll(float weight) {
    mWeight = weight;
    for (auto it = mClips.begin(); it != mClips.end(); ++it) {
        it->second.mWeight = 0.0f;
        it->second.mSpeed = 0.0f;
    }
    _Evaluate(mTree, 1.0f, 1.0f);
    const TimeUnits units = static_cast<TimeUnits>(mTree->_GetTimeUnits());
    TimeMgr::Clock* const clock = mDriver->mRuntime.mClock;

    float phase;
    int wraps;
    if (mTree->mNormalizePeriod && mTree->mAlignPeriod) {
        const float period = mTree->mPeriod;
        const float time = clock->Time(units);
        float offset = 0.0f;
        wraps = 0;
        if (period != 0.0f) {
            const float passes = floorf(time / period);
            wraps = static_cast<int>(passes);
            offset = time - static_cast<float>(wraps) * period;
        }
        phase = offset / mTree->mPeriod;
    } else {
        float rate;
        if (mTree->mNormalizePeriod) {
            rate = 1.0f / mTree->mPeriod;
        } else {
            rate = 0.0f;
            for (auto it = mClips.begin(); it != mClips.end(); ++it) {
                const CharClipResource* const resource = it->first;
                const float end = resource->GetTimeFromFrame(resource->GetEndFrame(), units);
                const float start = resource->GetTimeFromFrame(resource->GetStartFrame(), units);
                const float length = end - start;
                rate += length > 0.0f ? 1.0f / length * it->second.mSpeed : 0.0f;
            }
        }
        const float position = clock->DeltaTime(units) * rate + mPhase;
        wraps = static_cast<int>(floorf(position));
        phase = position - static_cast<float>(wraps);
    }

    const float lastPhase = mPhase;
    mPhase = phase;
    mCycles += wraps;
    const bool wrapped = wraps != 0;
    for (auto it = mClips.begin(); it != mClips.end(); ++it) {
        ClipState& clip = it->second;
        if (clip.mWeight > 0.0f) {
            if (clip.mAnim.mDriver == nullptr) {
                const CharClipResource* const resource = it->first;
                AnimOptions options;
                options.mTimeline = units;
                options.mStartPaused = true;
                options.mManualTime = true;
                const float start = resource->GetTimeFromFrame(resource->GetStartFrame(), units);
                const float end = resource->GetTimeFromFrame(resource->GetEndFrame(), units);
                options.mStartTime = (end - start) * lastPhase + start;
                clip.mAnim = mDriver->StartAnim(resource->mPath, options);
            }
            PlaybackInfo* const info = mDriver->GetPlaybackInfo(clip.mAnim);
            info->mCurrentTime = (info->mEndTime - info->mBeginTime) * mPhase + info->mBeginTime;
            info->mWeight = mWeight * clip.mWeight;
            info->mWrapped = wrapped;
            info->mLoopCount = mCycles;
        } else if (clip.mAnim.mDriver != nullptr) {
            mDriver->StopAnim(clip.mAnim);
        }
    }
}

// Reconstructed from eboot.elf at 0x7FFAC0. Subtrees are found in the
// graph, clips in the instance's entity.
void ClipBlendTreeCom::Instance::_Evaluate(ClipBlendTreeCom* tree, float weight, float speed) {
    TreeState& state = mTreeStates[tree];
    state.mWeight = weight;
    _ComputeChildWeights(tree, mEditMode, state.mChildWeights);
    for (unsigned long index = 0; index < tree->mNodes.size(); ++index) {
        const Node& node = tree->mNodes[index];
        const float childWeight = state.mChildWeights[index] * weight;
        const float childSpeed = state.mChildWeights[index] * speed * node.mSpeed;
        if (node.mType == kChildBlendTree) {
            if (node.mObject.mId == gNullObjectId.mId) {
                continue;
            }
            const GameObject* const object = tree->mObject->mEntity->GetObject(node.mObject);
            if (object == nullptr) {
                continue;
            }
            ClipBlendTreeCom* const child = object->GetCom<ClipBlendTreeCom>();
            if (child != nullptr) {
                _Evaluate(child, childWeight, childSpeed);
            }
            continue;
        }
        if (node.mObject.mId == gNullObjectId.mId) {
            continue;
        }
        const GameObject* const object = mEntity->GetObject(node.mObject);
        if (object == nullptr) {
            continue;
        }
        const CharClipCom* const clipCom = object->GetCom<CharClipCom>();
        if (clipCom == nullptr) {
            continue;
        }
        CharClipResource* const resource = clipCom->mResource.Get();
        if (resource == nullptr || resource->Fail()) {
            continue;
        }
        ClipState& clip = mClips[resource];
        clip.mWeight += childWeight;
        clip.mSpeed += childSpeed;
    }
}

// Reconstructed from eboot.elf at 0x7FFEA0.
void ClipBlendTreeCom::Instance::Stop() {
    for (auto it = mClips.begin(); it != mClips.end(); ++it) {
        mDriver->StopAnim(it->second.mAnim);
    }
}

// Reconstructed from eboot.elf at 0x7FFEF0.
float ClipBlendTreeCom::Instance::GetWeight(const BlendTreeBaseCom* tree) {
    const auto it = mTreeStates.find(tree);
    if (it == mTreeStates.end()) {
        return 0.0f;
    }
    return mWeight * it->second.mWeight;
}

// Reconstructed from eboot.elf at 0x7FFF60. A tree not in the blend has no
// weights: an empty guarded local static (0x1ABEA10, guard 0x1ABEA30).
const eastl::vector<float>& ClipBlendTreeCom::Instance::GetChildWeights(const BlendTreeBaseCom* tree) {
    const auto it = mTreeStates.find(tree);
    if (it != mTreeStates.end()) {
        return it->second.mChildWeights;
    }
    static eastl::vector<float> none;
    return none;
}

// Reconstructed from eboot.elf at 0x800080.
void ClipBlendTreeCom::Instance::_ComputeChildWeights(
    const ClipBlendTreeCom* tree,
    bool editMode,
    eastl::vector<float>& weights) {
    const unsigned long count = tree->mNodes.size();
    if (count == 0) {
        return;
    }
    const float parameter = tree->GetParameterValue(mEntity, editMode);
    weights.clear();
    weights.resize(count);
    const ClipBlendTreeCom::Node* const nodes = tree->mNodes.data();
    if (!(parameter >= nodes[0].mValue)) {
        weights[0] = 1.0f;
        return;
    }
    if (parameter >= nodes[count - 1].mValue) {
        weights.back() = 1.0f;
        return;
    }
    for (unsigned long index = 1; index < count; ++index) {
        const float low = nodes[index - 1].mValue;
        const float high = nodes[index].mValue;
        if (low <= parameter && parameter < high) {
            const float blend = high == low ? 1.0f : (parameter - low) / (high - low);
            weights[index - 1] = 1.0f - blend;
            weights[index] = blend;
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x800260.
Symbol ClipBlendTreeCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x800270.
Symbol ClipBlendTreeCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x800280.
int ClipBlendTreeCom::CurrentRev() const {
    return const_cast<ClipBlendTreeCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8002A0.
bool ClipBlendTreeCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8002D0.
Component* ClipBlendTreeCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x8002E0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* ClipBlendTreeCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<ClipBlendTreeCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) ClipBlendTreeCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x8004D0.
PropRegistry& ClipBlendTreeCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x8004E0.
ComMetaData& ClipBlendTreeCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x6FEFF0. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* ClipBlendTreeCom::_Create() {
    return new ClipBlendTreeCom();
}

// Reconstructed from eboot.elf at 0x6F7C20. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void ClipBlendTreeCom::Init() {
    InitComponentClass<ClipBlendTreeCom>();
}
