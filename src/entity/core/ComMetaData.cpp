#include "entity/core/ComMetaData.h"

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
