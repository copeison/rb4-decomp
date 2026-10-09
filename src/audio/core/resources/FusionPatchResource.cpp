#include "audio/core/resources/FusionPatchResource.h"

#include <cstring>
#include <utility>

#include "audio/core/fusion/FusionGenerator.h"
#include "audio/core/fusion/FusionPatchCom.h"
#include "entity/core/Component.h"
#include "entity/core/EditorCom.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "os/memory/MemMgr.h"
#include "utl/data/DataFile.h"
#include "utl/files/FileUtl.h"
#include "utl/streams/BinStream.h"
#include "utl/text/Str.h"

// The class's metadata at 0x19C87D0.
ResourceMetaData FusionPatchResource::sMetaData;

// Reconstructed from eboot.elf at 0x5B590.
FusionPatchResource::FusionPatchResource() : mLoaded(true), mName("") {}

// Reconstructed from eboot.elf at 0x5B5E0.
FusionPatchResource::~FusionPatchResource() {
    FusionGeneratorManager::RemovePatch(this);
}

// Reconstructed from eboot.elf at 0x5B650.
void FusionPatchResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("fusion"));
    metaData.mCategory = Symbol("Fusion Patches");
    metaData.mTypeOption = 2;
}

// Reconstructed from eboot.elf at 0x5B770.
bool FusionPatchResource::NeedsReload() {
    const FusionPatchCom* const patch = GetPatch();
    if (patch == nullptr) {
        return true;
    }
    for (unsigned int index = 0; index < patch->mNumKeyzones; ++index) {
        AudioSampleResource* const sample = patch->mKeyzones[index].mSample;
        if (sample != nullptr && (sample->mFileChangedOnDisk || sample->NeedsReload())) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x5B830.
FusionPatchCom* FusionPatchResource::GetPatch() const {
    return mEntity->GetRoot()->GetCom<FusionPatchCom>();
}

// Reconstructed from eboot.elf at 0x5B890. The binary inlines GetPatch.
void FusionPatchResource::GetDependencies(eastl::vector<ResourcePtr<Resource>>& dependencies) const {
    const FusionPatchCom* const patch = GetPatch();
    if (patch == nullptr) {
        return;
    }
    for (unsigned int index = 0; index < patch->mNumKeyzones; ++index) {
        AudioSampleResource* const sample = patch->mKeyzones[index].mSample;
        if (sample == nullptr) {
            continue;
        }
        sample->GetDependencies(dependencies);
        ResourcePtr<Resource> dependency(sample);
        dependencies.emplace_back(std::move(dependency));
    }
}

// Reconstructed from eboot.elf at 0x5B9D0. A patch that loads registers
// under its file name.
void FusionPatchResource::LoadFile() {
    mLoaded = false;
    if (std::strcmp(FileGetExt(mPath.Str(), false), "sxt") == 0) {
        CreateEntity();
        mLoaded = GetPatch()->_LoadFromSXTFile(mPath);
        if (mLoaded) {
            LoadResources();
        }
    } else if (std::strcmp(FileGetExt(mPath.Str(), false), "fusion") == 0) {
        CreateEntity();
        mLoaded = GetPatch()->_LoadFromDTAFile(mPath);
        if (mLoaded) {
            LoadResources();
        }
    } else {
        Resource::LoadFile();
        mLoaded = mEntity != nullptr;
    }
    if (mLoaded) {
        mName = Symbol(FileGetName(mPath.Str()));
        FusionGeneratorManager::AddPatch(Symbol(FileGetName(mPath.Str())), this);
    }
}

// Reconstructed from eboot.elf at 0x5BBA0.
const char* FusionPatchResource::_GetExt() const {
    return FileGetExt(mPath.Str(), false);
}

// Reconstructed from eboot.elf at 0x5BBB0.
void FusionPatchResource::Save(BinStream& stream, bool cached) {
    if (std::strcmp(FileGetExt(mPath.Str(), false), "fusion") != 0 || Component::sRegressionTesting) {
        EntityResource::Save(stream, cached);
        return;
    }
    const DataArrayPtr data = GetPatch()->_SaveToDataArray();
    if (data != nullptr) {
        String text;
        DataWriteStream(&text, data, 0);
        const char* const str = text.c_str();
        stream.Write(str, std::strlen(str));
    }
}

// Reconstructed from eboot.elf at 0x5BD20. The text is read into a
// temporary allocation that is not freed.
bool FusionPatchResource::_LoadEntity(BinStream& stream, bool cached) {
    if (std::strcmp(FileGetExt(mPath.Str(), false), "fusion") != 0 || Component::sRegressionTesting) {
        mLoaded = EntityResource::_LoadEntity(stream, cached);
    } else {
        CreateEntity();
        FusionPatchCom* const patch = GetPatch();
        unsigned int temp;
        MemPushTemp(temp, true, true);
        char* const text = static_cast<char*>(MemAllocTemp(stream.Size() + 1, "FusionDta", 0));
        stream.Read(text, stream.Size());
        text[stream.Size()] = '\0';
        if (stream.Fail()) {
            MemPopTemp(temp);
            return false;
        }
        {
            const DataArrayPtr data = DataReadString(text);
            mLoaded = patch->_LoadFromDataArray(data, ResourcePath(stream.Name()));
        }
        MemPopTemp(temp);
    }
    if (mLoaded) {
        mName = Symbol(FileGetName(mPath.Str()));
        FusionGeneratorManager::AddPatch(Symbol(FileGetName(mPath.Str())), this);
    }
    return mLoaded;
}

// Reconstructed from eboot.elf at 0x5BF80.
bool FusionPatchResource::PrepareEmpty() {
    return LoadResources();
}

// Reconstructed from eboot.elf at 0x5BF90.
EditorCom* FusionPatchResource::GetEditorCom() const {
    if (mEntity == nullptr) {
        return nullptr;
    }
    return mEntity->GetRoot()->GetCom<EditorCom>();
}

// Reconstructed from eboot.elf at 0x5C060.
Entity* FusionPatchResource::CreateEntity() {
    Entity* const entity = EntityResource::CreateEntity();
    GameObject* const root = entity->GetRoot();
    root->SetName(Symbol("fusion_patch"));
    reinterpret_cast<FusionPatchCom*>(root->CreateComponent(FusionPatchCom::sClassName, false))
        ->mResource = this;
    EditorCom* const editor = root->GetCom<EditorCom>();
    if (editor != nullptr) {
        editor->RevokeEditorCapability(root, EditorCom::kCanChangeLayer);
        editor->RevokeEditorCapability(root, EditorCom::kCanDelete);
        editor->RevokeEditorCapability(root, EditorCom::kCanChangeProperties);
    }
    return entity;
}

// Reconstructed from eboot.elf at 0x5C180.
ResourceMetaData* FusionPatchResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x5C190.
Symbol FusionPatchResource::GetId() const {
    return Id();
}

// Reconstructed from eboot.elf at 0x5C230.
bool FusionPatchResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x5C260.
bool FusionPatchResource::Fail() const {
    return !mLoaded;
}

// Reconstructed from eboot.elf at 0x5C280.
bool FusionPatchResource::HasDependencies() const {
    return true;
}
