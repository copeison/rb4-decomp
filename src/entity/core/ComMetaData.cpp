#include "entity/core/ComMetaData.h"

#include "entity/props/PropMetadata.h"

// The registered classes at 0x19E28B8.
eastl::vector<ComMetaData*> ComMetaData::sComMetaDataList;

// Reconstructed from eboot.elf at 0xE56C0. The category and the size are
// left for the class's registration to set.
ComMetaData::ComMetaData()
    : mDescription(),
      mAuthor(),
      mInterface(),
      mAllowedResources(),
      mInstanceCom(false),
      mRootOnly(false),
      mLightweight(false),
      mReserved(false),
      mAliases(),
      mEditorRestrictions(3),
      mRequiredComponents(),
      mDefaultComponents(),
      mDefaultFor(),
      mEditorAttributes(),
      mExportedEvents(),
      mPropRegistry(nullptr),
      mDependentComponents(),
      mDefaultComponentIds(),
      mId(),
      mClassName(),
      mCRC(2166136261U),
      mSuperclass(nullptr),
      mHasFlaggedProps(false),
      mCreatable(true),
      mSuperclassRoot(false),
      mAllowMultiple(false) {}

// Reconstructed from eboot.elf at 0xE5900. Ids are matched before aliases.
ComMetaData* ComMetaData::GetMetaData(Symbol id) {
    for (ComMetaData* metaData : sComMetaDataList) {
        if (metaData->mId == id) {
            return metaData;
        }
    }
    for (ComMetaData* metaData : sComMetaDataList) {
        for (const Symbol& alias : metaData->mAliases) {
            if (alias == id) {
                return metaData;
            }
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xE5980.
ComMetaData* ComMetaData::GetMetaData(const Symbol& id, bool fail) {
    static_cast<void>(fail);
    for (ComMetaData* metaData : sComMetaDataList) {
        if (metaData->mId == id) {
            return metaData;
        }
    }
    for (ComMetaData* metaData : sComMetaDataList) {
        for (const Symbol& alias : metaData->mAliases) {
            if (alias == id) {
                return metaData;
            }
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0xE5A00.
Symbol ComMetaData::GetCategorySym(Category category) {
    const char* name;
    switch (category) {
    case kCategoryRendering:
        name = "Rendering";
        break;
    case kCategoryCamera:
        name = "Camera";
        break;
    case kCategoryLight:
        name = "Light";
        break;
    case kCategoryEffects:
        name = "Effects";
        break;
    case kCategoryParticleSystem:
        name = "Particle System";
        break;
    case kCategoryPostProc:
        name = "Post Proc";
        break;
    case kCategoryAnimation:
        name = "Animation";
        break;
    case kCategoryCharacter:
        name = "Character";
        break;
    case kCategoryAudio:
        name = "Audio";
        break;
    case kCategoryPhysics:
        name = "Physics";
        break;
    case kCategoryVR:
        name = "VR";
        break;
    case kCategoryMiscellaneous:
        name = "Miscellaneous";
        break;
    case kCategoryUI:
        name = "UI";
        break;
    case kCategoryEditor:
        name = "Editor";
        break;
    case kCategoryDebug:
        name = "Debug";
        break;
    default:
        name = "";
        break;
    }
    return Symbol(name);
}

// Reconstructed from eboot.elf at 0xE5AF0.
void ComMetaData::InitGlobals() {
    sComMetaDataList.reserve(512);
}

namespace {

// Whether the attribute is the constant true. Inlined into the property
// check of Init. Name not in the reference map.
bool IsConstantTrue(const TagBase<bool>& tag) {
    return tag.mMode == TagBase<bool>::kModeConstant && tag.mValue;
}

// FNV-1a over the bytes. Inlined into Init. Name not in the reference map.
unsigned int HashBytes(unsigned int hash, const char* bytes, unsigned long count) {
    for (unsigned long index = 0; index < count; ++index) {
        hash = (hash ^ static_cast<unsigned int>(static_cast<int>(bytes[index]))) * 16777619U;
    }
    return hash;
}

// FNV-1a over the text. Inlined into Init. Name not in the reference map.
unsigned int HashText(unsigned int hash, const char* text) {
    for (; *text != '\0'; ++text) {
        hash = (hash ^ static_cast<unsigned int>(static_cast<int>(*text))) * 16777619U;
    }
    return hash;
}

}  // namespace

// Reconstructed from eboot.elf at 0xE5B80. The release build keeps the
// duplicate-id and superclass walks of its stripped checks, which have no
// effect and are left out. The class's required classes learn that it
// depends on them; its default classes, and the classes it is a default
// for, record the interfaces. The CRC hashes the class and interface
// names, the post-poll flag, the order dependencies and the saved
// properties' layout. A property under a hidden or read-only one is not
// checked; a resource path without a resource class, extensions or entity
// relation only builds the stripped warning's text.
void ComMetaData::Init(
    const Symbol& id,
    Symbol className,
    const PropRegistry& registry,
    unsigned long size,
    bool hasPostPoll,
    bool unlisted) {
    mId = id;
    mClassName = className;
    if (mDescription.c_str()[0] == '\0') {
        mDescription = "<no description>";
    }
    mPropRegistry = &registry;
    static_cast<void>(GetCategorySym(mCategory));

    for (const Symbol* required = mRequiredComponents.begin(); required != mRequiredComponents.end();
         ++required) {
        static_cast<void>(Component::IsValid(*required));
        GetMetaData(*required, true)->mDependentComponents.push_back(mClassName);
    }

    mDefaultComponentIds.reserve(mDefaultComponentIds.size() + mDefaultComponents.size());
    for (const Symbol* component = mDefaultComponents.begin(); component != mDefaultComponents.end();
         ++component) {
        static_cast<void>(Component::IsValid(*component));
        const ComMetaData* const metadata = GetMetaData(*component, true);
        bool listed = false;
        for (const DefaultComponent* entry = mDefaultComponentIds.begin();
             entry != mDefaultComponentIds.end();
             ++entry) {
            if (entry->mId == *component) {
                listed = true;
                break;
            }
        }
        if (!listed) {
            mDefaultComponentIds.push_back(DefaultComponent{*component, metadata->mInterface});
        }
    }

    for (const Symbol* owner = mDefaultFor.begin(); owner != mDefaultFor.end(); ++owner) {
        static_cast<void>(Component::IsValid(*owner));
        ComMetaData* const metadata = GetMetaData(*owner, true);
        bool listed = false;
        for (const DefaultComponent* entry = metadata->mDefaultComponentIds.begin();
             entry != metadata->mDefaultComponentIds.end();
             ++entry) {
            if (entry->mId == mId) {
                listed = true;
                break;
            }
        }
        if (!listed) {
            metadata->mDefaultComponentIds.push_back(DefaultComponent{mId, mInterface});
        }
    }

    mSize = static_cast<int>(size);
    mCRC = 2166136261U;
    mCRC = HashText(mCRC, mClassName.Str());
    mCRC = HashText(mCRC, mInterface.Str());
    mCRC = (mCRC ^ static_cast<unsigned int>(hasPostPoll)) * 16777619U;
    const unsigned int orderCRC = Component::GetClassCRC(mId);
    mCRC = HashBytes(mCRC, reinterpret_cast<const char*>(&orderCRC), sizeof(orderCRC));
    registry.UpdateCRC(mCRC, false);

    unsigned long hiddenDepth = 0;
    unsigned long* const depth = &hiddenDepth;
    auto checkProp = [this, &hiddenDepth](const PropInfo& info, const PropPath& path) {
        const PropMetadata& metadata = *info.mMetadata.Get();
        if (!mHasFlaggedProps) {
            mHasFlaggedProps = metadata.mInstanceAccess != PropMetadata::kInstanceAccessNone;
        }
        if (hiddenDepth != 0 || IsConstantTrue(metadata.mIsHidden) || IsConstantTrue(metadata.mReadOnly)) {
            ++hiddenDepth;
        } else if (info.mType == kPropertyResourcePath) {
            const ResourcePathMetadata& file = static_cast<const ResourcePathMetadata&>(metadata);
            const bool noResourceClass =
                file.mResourceId.mMode == TagBase<Symbol>::kModeConstant && file.mResourceId.mValue == Symbol();
            if (noResourceClass && file.mCustomExtensions.mMode == TagBase<eastl::vector<Symbol>>::kModeUnset
                && !file.mIsRelativeToEntity) {
                char text[512] = {};
                path.ToString(text, sizeof(text));
            }
        }
        return true;
    };
    registry.VisitPropInfo(checkProp, depth);

    if (!unlisted) {
        sComMetaDataList.push_back(this);
    }
}

// Reconstructed from eboot.elf at 0xE6850.
void ComMetaData::InitSuperclass(const char* name, const ComMetaData& superclass) {
    static_cast<void>(name);
    if (mSuperclass == nullptr) {
        mSuperclass = const_cast<ComMetaData*>(&superclass);
    }
}

// Reconstructed from eboot.elf at 0xE6870.
ComMetaData::EditorRestriction ComMetaData::InheritEditorRestrictions(
    EditorRestriction restrictions,
    EditorRestriction inherited) {
    int combined = inherited & 5;
    if ((restrictions & 4) != 0) {
        combined = (restrictions & 1) | 4;
    }
    if ((restrictions & 8) != 0) {
        return static_cast<EditorRestriction>(combined | (restrictions & 2) | 8);
    }
    return static_cast<EditorRestriction>((inherited & 0xA) | combined);
}

// Reconstructed from eboot.elf at 0xE68A0. The event is built with a copy
// of the description and an empty registry, then copied in.
PropRegistry& ComMetaData::AddExportedEvent(Symbol name, String description) {
    if (mExportedEvents.capacity() < 32) {
        mExportedEvents.reserve(32);
    }
    const String text(description);
    const ExportedEvent event = {name, text, {}};
    mExportedEvents.push_back(event);
    return mExportedEvents.back().mParams;
}
