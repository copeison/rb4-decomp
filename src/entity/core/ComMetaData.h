#pragma once

#include <cstddef>

#include "entity/props/PropRegistry.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

// The registration of a component class (entity/ComMetaData.o,
// 0xE56C0-0xE7180): its description, the classes it depends on, the
// resources it may be created in and its property registry. Every class's
// static initializer fills one and calls Init, which adds it to
// sComMetaDataList. The object is 424 bytes. Field names are not in the
// reference map.
class ComMetaData {
public:
    // The editor category, named by GetCategorySym. The map names the type;
    // the enumerator names follow the category strings.
    enum Category : int {
        kCategoryRendering = 0,
        kCategoryCamera = 1,
        kCategoryLight = 2,
        kCategoryEffects = 3,
        kCategoryParticleSystem = 4,
        kCategoryPostProc = 5,
        kCategoryAnimation = 6,
        kCategoryCharacter = 7,
        kCategoryAudio = 8,
        kCategoryPhysics = 9,
        kCategoryVR = 10,
        kCategoryMiscellaneous = 11,
        kCategoryUI = 12,
        kCategoryEditor = 13,
        kCategoryDebug = 14,
    };

    // What the editor may change on a component of the class, combined by
    // InheritEditorRestrictions: bits 0 and 1 are values, and bits 2 and 3
    // mark them as set. The map names the type; the bit names are not
    // recovered.
    enum EditorRestriction : int {};

    // A message the class sends, with its parameters. The map names the
    // type; the field names are not in the reference map.
    struct ExportedEvent {
        Symbol mName;
        String mDescription;
        PropRegistry mParams;
    };

    // A class created alongside this one, with its interface: the object
    // gets it unless it already holds the class or the interface. Name not
    // in the reference map.
    struct DefaultComponent {
        Symbol mId;
        Symbol mInterface;
    };

    ComMetaData();  // 0xE56C0

    // The registered class with the id or one of its aliases, or null. The
    // map's signature is GetMetaData(Symbol, bool); this build has a copy
    // without the flag (0xE5900) and one taking a reference whose flag the
    // release build does not read.
    static ComMetaData* GetMetaData(Symbol id);                     // 0xE5900
    static ComMetaData* GetMetaData(const Symbol& id, bool fail);  // 0xE5980
    static Symbol GetCategorySym(Category category);               // 0xE5A00
    // Reserves room for 512 classes in the list.
    static void InitGlobals();  // 0xE5AF0

    // Registers the class: the ids, the property registry, the component's
    // size and whether it post-polls, the dependency lists of the classes it
    // names, and the class CRC. The map's signature is Init(Symbol,
    // PropRegistry const&, unsigned long, bool, bool); this build passes the
    // id by reference and adds mClassName. `unlisted` keeps the class out of
    // sComMetaDataList. Not reconstructed.
    void Init(
        const Symbol& id,
        Symbol className,
        const PropRegistry& registry,
        unsigned long size,
        bool hasPostPoll,
        bool unlisted);  // 0xE5B80
    // Makes the metadata the class's superclass unless it has one. The map's
    // signature is InitSuperclass(char const*, ComMetaData const&); the name
    // is not read.
    void InitSuperclass(const char* name, const ComMetaData& superclass);  // 0xE6850
    // Combines a class's restrictions with those it inherits: a set bit of
    // `restrictions` overrides the inherited value.
    static EditorRestriction InheritEditorRestrictions(
        EditorRestriction restrictions,
        EditorRestriction inherited);  // 0xE6870
    // Adds an exported event, reserving room for 32 the first time, and
    // returns its parameter registry.
    PropRegistry& AddExportedEvent(Symbol name, String description);  // 0xE68A0

    // The class's description and author, shown in the editor.
    String mDescription;
    String mAuthor;
    // The interface the class implements, which GameObject::ComIndex keeps
    // as mBaseId: GetBaseCom<RndLightCom> finds a light by its "Light"
    // interface. An object holds one component per interface.
    Symbol mInterface;
    Category mCategory;
    // The entity resource classes the class may be created in
    // (EntityResource::IsAllowedComponent).
    eastl::vector<Symbol> mAllowedResources;
    // Set for classes that serve as a TransEntityResource's instance
    // component (TransEntityResource::_PostLoad at 0x1BB0E0).
    bool mInstanceCom;
    // Set for classes created only on an entity's root object.
    bool mRootOnly;
    // Set for the camera, mesh, drawable, particle, transform, instance and
    // audio clip classes, among others. _SortComponents records whether
    // every component's class sets it (GameObject::mAllComsFlagged); no
    // reader is identified, so the name is weakly supported.
    bool mLightweight;
    // Set only by the registration at 0x2E6360; no reader is identified.
    bool mReserved;
    // Other ids the class is found by.
    eastl::vector<Symbol> mAliases;
    // 3 by default and 1 for TransCom; presumably the class's
    // EditorRestriction bits. Weak evidence.
    int mEditorRestrictions;
    // The classes the object must hold (EntityResource::
    // CreateRequiredComponents). Init adds this class to their
    // mDependentComponents.
    eastl::vector<Symbol> mRequiredComponents;
    // The classes created with this one; Init turns them into
    // mDefaultComponentIds.
    eastl::vector<Symbol> mDefaultComponents;
    // The classes that create this one by default; Init adds this class to
    // their mDefaultComponentIds.
    eastl::vector<Symbol> mDefaultFor;
    // Editor attributes by name, such as "group". The value type is not
    // confirmed.
    eastl::map<Symbol, Symbol> mEditorAttributes;
    eastl::vector<ExportedEvent> mExportedEvents;
    const PropRegistry* mPropRegistry;
    // The classes that require this one (EntityResource::
    // DestroyDependentComponents).
    eastl::vector<Symbol> mDependentComponents;
    eastl::vector<DefaultComponent> mDefaultComponentIds;
    // The id GameObject::ComIndex keeps as mId, and the name Init receives
    // beside it; they are the same for the registrations seen.
    Symbol mId;
    Symbol mClassName;
    // An FNV-1a hash of the names, the post-poll flag and the class's
    // poll-order dependencies.
    unsigned int mCRC;
    // The component's size in bytes.
    int mSize;
    ComMetaData* mSuperclass;
    // Set by Init when a property's metadata has the flag at +1512.
    bool mHasFlaggedProps;
    // Cleared for classes that cannot be created; set by default.
    bool mCreatable;
    // Ends Init's walk up the superclasses.
    bool mSuperclassRoot;
    // Lets an object hold several components of the class, each named by
    // its address (0x118900).
    bool mAllowMultiple;

    // Every registered class, in registration order.
    static eastl::vector<ComMetaData*> sComMetaDataList;  // 0x19E28B8
};

static_assert(sizeof(ComMetaData::ExportedEvent) == 192);
static_assert(offsetof(ComMetaData, mInterface) == 32);
static_assert(offsetof(ComMetaData, mAllowedResources) == 48);
static_assert(offsetof(ComMetaData, mInstanceCom) == 80);
static_assert(offsetof(ComMetaData, mAliases) == 88);
static_assert(offsetof(ComMetaData, mEditorRestrictions) == 120);
static_assert(offsetof(ComMetaData, mRequiredComponents) == 128);
static_assert(offsetof(ComMetaData, mDefaultComponents) == 160);
static_assert(offsetof(ComMetaData, mDefaultFor) == 192);
static_assert(offsetof(ComMetaData, mEditorAttributes) == 224);
static_assert(offsetof(ComMetaData, mExportedEvents) == 280);
static_assert(offsetof(ComMetaData, mPropRegistry) == 312);
static_assert(offsetof(ComMetaData, mDependentComponents) == 320);
static_assert(offsetof(ComMetaData, mDefaultComponentIds) == 352);
static_assert(offsetof(ComMetaData, mId) == 384);
static_assert(offsetof(ComMetaData, mCRC) == 400);
static_assert(offsetof(ComMetaData, mSuperclass) == 408);
static_assert(offsetof(ComMetaData, mHasFlaggedProps) == 416);
static_assert(offsetof(ComMetaData, mAllowMultiple) == 419);
static_assert(sizeof(ComMetaData) == 424);
