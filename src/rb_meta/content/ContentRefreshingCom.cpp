// rb_meta/ContentRefreshingCom.o (0xDB1A30 to 0xDB23A7). The property
// accessors are the lambdas after the identity slots (0xDB22C0-0xDB22E0).
// The static initializer is at 0xDB22F0. The class's registration and
// factory are emitted elsewhere (0xB63C50, 0xBA0B30).
#include "rb_meta/content/ContentRefreshingCom.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransEntityResource.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "os/content/ContentMgr.h"
#include "os/locale/Locale.h"
#include "rb_meta/input/RBUIInputCom.h"
#include "rb_meta/songs/SongMgr.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x1B25C08, which the static initializer sets.
// Each object that uses it has its own copy. Name not in the reference
// map.
[[maybe_unused]] GameObjectId gNullObjectId = {0xFFFFFFFFU};

}  // namespace

// The object's statics, in the order of its static initializer (0xDB22F0).
Symbol ContentRefreshingCom::sId("ContentRefreshingCom");
Symbol ContentRefreshingCom::sClassName("ContentRefreshingCom");
PropRegistry ContentRefreshingCom::sPropRegistry;
ComMetaData ContentRefreshingCom::sMetaData;

// Reconstructed from eboot.elf at 0xDB1A30.
ContentRefreshingCom::ContentRefreshingCom() : mState(kStateInactive), mCompletedStr() {}

// Reconstructed from eboot.elf at 0xDB1A70. The deleting destructor is at
// 0xDB1A80.
ContentRefreshingCom::~ContentRefreshingCom() {}

// Reconstructed from eboot.elf at 0xDB1AA0.
void ContentRefreshingCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    metadata.mDescription = "Brings up a blocking dialog whenever song content is refreshing.";
    metadata.mAuthor = "Chris McCarthy";
    metadata.mCategory = ComMetaData::kCategoryUI;
    metadata.mAllowedResources.push_back(TransEntityResource::Id());

    const ContentRefreshingCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    const int intType = 0;
    IntMetadata<int>& state = TypeSpecificMetadata(registry.RegisterProp("state", -1, kPropertyInt, 0), intType);
    static_cast<void>(Symbol("prop"));
    {
        eastl::vector<AllowedValue<int>> values;
        values.push_back(AllowedValue<int>{kStateInactive, String("kStateInactive"), String("")});
        values.push_back(AllowedValue<int>{kStateRefreshing, String("kStateRefreshing"), String("")});
        state.mAllowedValues = values;
    }
    state.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<int>(static_cast<ContentRefreshingCom*>(args.mComponent)->mState);
    };
    state.mSetter = [](const PropAccessorArgs& args) {
        static_cast<ContentRefreshingCom*>(args.mComponent)->mState =
            static_cast<State>(*static_cast<const int*>(args.mValue));
    };
    state.mHistory = false;

    const Symbol symbolType = prototype.mCompletedStr;
    SymbolMetadata& completedStr =
        TypeSpecificMetadata(registry.RegisterProp("completed_str", -1, kPropertySymbol, 0), symbolType);
    static_cast<void>(Symbol("prop"));
    completedStr.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<ContentRefreshingCom*>(args.mComponent)->mCompletedStr;
    };
    completedStr.mHistory = false;
}

// Reconstructed from eboot.elf at 0xDB1F10.
void ContentRefreshingCom::_Enter() {
    _UpdateState();
}

// Reconstructed from eboot.elf at 0xDB1F20. The input component is not
// checked for null.
void ContentRefreshingCom::_UpdateState() {
    if (TheContentMgr.IsRefreshInProgress()) {
        mState = kStateRefreshing;
        static Symbol progress;
        if (progress == Symbol()) {
            progress = Symbol("loading_additional_progress");
        }
        const int discovered = theSongMgr->mNumDiscoveredContent;
        if (discovered > 0) {
            FormatString format(Localize(progress, nullptr, 0, TheLocale));
            format << theSongMgr->mNumDiscoveredContentMounted << discovered;
            mCompletedStr = Symbol(format.Str());
        } else {
            mCompletedStr = Symbol(progress.Str());
        }
    } else {
        mState = kStateInactive;
    }
    RBUIInputCom* const input = mObject->mEntity->GetRoot()->GetCom<RBUIInputCom>();
    input->SetIsModal(mState == kStateRefreshing);
}

// Reconstructed from eboot.elf at 0xDB20E0.
void ContentRefreshingCom::_Poll() {
    _UpdateState();
}

// Reconstructed from eboot.elf at 0xDB20F0.
Symbol ContentRefreshingCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xDB2100.
Symbol ContentRefreshingCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xDB2110.
int ContentRefreshingCom::CurrentRev() const {
    return const_cast<ContentRefreshingCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xDB2130.
bool ContentRefreshingCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xDB2160.
Component* ContentRefreshingCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xDB2170. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* ContentRefreshingCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<ContentRefreshingCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) ContentRefreshingCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xDB22A0.
PropRegistry& ContentRefreshingCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xDB22B0.
ComMetaData& ContentRefreshingCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBA0B30. The binary emits the factory
// with the other rb_meta factories.
Component* ContentRefreshingCom::_Create() {
    return new ContentRefreshingCom();
}

// Reconstructed from eboot.elf at 0xB63C50. The binary emits the
// registration with the other rb_meta classes'.
void ContentRefreshingCom::Init() {
    InitComponentClass<ContentRefreshingCom>();
}
