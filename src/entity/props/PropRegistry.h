#pragma once

#include <cstddef>
#include <functional>

#include "entity/props/PropInfo.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;
class Component;
class PropPath;

// The properties of a component type (entity/PropRegistry.o): each property's
// name and PropInfo, the registries it inherits, and the layout of its
// dynamic storage. The object is 160 bytes, aligned for its std::function.
// Field names are not in the reference map.
class PropRegistry {
public:
    // One registered property. Name not in the reference map.
    struct PropEntry {
        PropEntry(const char* name, const PropInfo& info) : mName(name), mInfo(info) {}

        Symbol mName;
        PropInfo mInfo;
    };

    // A registry the type inherits, with that registry's revision when it
    // was added. Name not in the reference map.
    struct InheritedRegistry {
        Symbol mName;
        int mRev;
        const PropRegistry* mRegistry;
    };

    // What the revision callback receives; not modelled.
    struct RevisionArgs;

    PropRegistry();   // 0x1804C0
    // The copy constructor is the implicit one; the binary emits it with
    // ComMetaData's exported events at 0xE6E00.
    ~PropRegistry();  // 0x180540

    // The revision the type's saved properties have.
    void SetCurrentRev(int rev);  // 0x1805F0
    // Grows the property list ahead of more than 32 registrations. Name not
    // in the reference map.
    void ReserveProps(unsigned long count);  // 0x180600
    // Adds a property with a default PropInfo and returns the info. The list
    // starts with room for 32 properties. Name not in the reference map.
    PropInfo& _AddProp(const char* name);  // 0x180630
    // Adds a property and returns its info, whose metadata is created for
    // the type. The map does not give the return type.
    PropInfo& RegisterProp(
        const char* name,
        int offset,
        PropertyType type,
        unsigned long count);  // 0x1807F0
    // Adds an array property; its element type is registered with
    // RegisterPropArrayItem.
    PropInfo& RegisterPropArray(const char* name, int offset, unsigned long count);  // 0x180840
    // Sets the element type of the array property and returns the
    // elements' info. An array of arrays keeps the array flag on the outer
    // type.
    static PropInfo& RegisterPropArrayItem(
        PropInfo& array,
        PropertyType type,
        unsigned long count);  // 0x1808A0
    // Adds an action, a property without storage. Not reconstructed: its
    // metadata class (vtable 0x18E87D0) is not modelled.
    PropInfo& RegisterAction(const char* name);  // 0x180910
    // Adds a property to the dynamic storage, placed at the next offset its
    // alignment allows. `dynamicOffset` locates the storage pointer (see
    // PropInfo::mDynamicOffset). Not reconstructed: actions take the
    // RegisterAction path, which is not modelled.
    PropInfo& RegisterDynamicProp(
        const char* name,
        PropertyType type,
        int dynamicOffset);  // 0x180980
    // Places the last property, a struct, after the members registered so
    // far, using its registry's dynamic size and alignment.
    void AmendDynamicStructSize();  // 0x180AC0
    // Rounds the dynamic storage up to its alignment.
    void FinalizeDynamicStruct();  // 0x180B10
    // Marks the registry as describing dynamic storage. Name not in the
    // reference map; the flag's readers are not identified.
    void SetDynamic(bool dynamic);  // 0x180B30
    // The property the path names and, when `storage` is given, its address:
    // `*storage` starts as the component and ends as the property's storage,
    // or null when it is missing.
    const PropInfo* FindProp(const PropPath& path, void** storage) const;  // 0x180B40
    // Moves the property `name` to just before `before`. Not reconstructed.
    void MoveProp(Symbol name, Symbol before);  // 0x180EC0
    // Removes the property. Name not in the reference map.
    void RemoveProp(Symbol name);  // 0x1810C0
    void AddInheritedRegistry(Symbol name, const PropRegistry& registry);  // 0x181180
    // Writes the property infos, then the component's values.
    void Save(const Component& component, BinStream& stream);  // 0x1813D0
    // Not reconstructed.
    void _SavePropInfo(BinStream& stream);  // 0x181410
    void _SavePropValues(const Component& component, BinStream& stream);  // 0x1815B0
    // Reads the property values Save wrote into the component. The map's
    // Load(ObjPtr const&, Component*, BinStream&); this build drops the
    // object. Not reconstructed.
    static void Load(Component* component, BinStream& stream);  // 0x181880

    eastl::vector<PropEntry> mProps;
    bool mDynamic;
    // The size and alignment of the dynamic storage the registered dynamic
    // properties need.
    long mDynamicStructSize;
    int mDynamicStructAlign;
    // The map's SetRevisionFunc sets it.
    std::function<void(const RevisionArgs&)> mRevisionFunc;
    eastl::vector<InheritedRegistry> mInheritedRegistries;
    int mCurrentRev;
};

static_assert(sizeof(PropRegistry::PropEntry) == 40);
static_assert(sizeof(PropRegistry::InheritedRegistry) == 24);
static_assert(offsetof(PropRegistry, mDynamic) == 32);
static_assert(offsetof(PropRegistry, mDynamicStructSize) == 40);
static_assert(offsetof(PropRegistry, mDynamicStructAlign) == 48);
static_assert(offsetof(PropRegistry, mRevisionFunc) == 64);
static_assert(offsetof(PropRegistry, mInheritedRegistries) == 112);
static_assert(offsetof(PropRegistry, mCurrentRev) == 144);
static_assert(sizeof(PropRegistry) == 160);
