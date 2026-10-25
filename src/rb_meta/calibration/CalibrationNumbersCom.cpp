// rb_meta/CalibrationNumbersCom.o (0xDB0280 to 0xDB1A2F). The static
// initializer is at 0xDB1930; of the shared header values it sets first
// (0x1B25928-0x1B25978) only the null object id is kept below. The class's
// registration and factory are emitted elsewhere (0xB638A0, 0xBA0AE0).
#include "rb_meta/calibration/CalibrationNumbersCom.h"

#include <new>

#include "audio/core/components/AudioListenerCom.h"
#include "entity/core/Entity.h"
#include "entity/core/InstanceCom.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "persistence/options/SystemOptions.h"
#include "rb_meta/input/RBUIInputCom.h"
#include "rb_meta/persistentdata/RBAchPersistentData.h"
#include "rb_meta/profiles/RBProfile.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_meta/state/RBMetaStateCom.h"
#include "render/scene/RndSceneResource.h"
#include "ui/input/UIMsg.h"
#include "ui/widgets/UIComboBoxEnumCom.h"
#include "utl/data/DataArray.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

namespace {

// The id of no object, at 0x1B25928, which the static initializer sets.
// Each object that uses it has its own copy. Name not in the reference
// map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The range of the offsets the combo boxes offer, in milliseconds, and the
// step between them. Names not in the reference map.
const float kMinAudioCalibration = -300.0f;  // 0x1377018
const float kMaxAudioCalibration = 300.0f;   // 0x137701C
const float kMinVideoCalibration = -300.0f;  // 0x1377020
const float kMaxVideoCalibration = 300.0f;   // 0x1377024
constexpr int kCalibrationStep = 5;

// Appends the offsets from `min` to `max` to the combo box held at
// `comboBox`, each with its "%dms" text, through the box's "entry_list" and
// "display_list" properties, then selects the entry of `current`. The
// binary inlines it twice and reads the box anew for each entry. Name not
// in the reference map.
void FillCalibrationComboBox(UIComboBoxEnumCom* const& comboBox, float min, float max, float current) {
    const int last = static_cast<int>(max);
    for (int value = static_cast<int>(min); value <= last; value += kCalibrationStep) {
        PropPath::Node entryList;
        entryList = Symbol("entry_list");
        PropPath::Node entryIndex;
        entryIndex = static_cast<unsigned long>(comboBox->Size(PropPath(Symbol("entry_list"))));
        comboBox->_Insert(PropPath(entryList, entryIndex), &value, kPropertyInt);

        UIComboBoxEnumCom* const displayBox = comboBox;
        PropPath::Node displayList;
        displayList = Symbol("display_list");
        PropPath::Node displayIndex;
        displayIndex = static_cast<unsigned long>(displayBox->Size(PropPath(Symbol("display_list"))));
        const PropPath displayPath(displayList, displayIndex);
        FormatString format("%dms");
        format << value;
        const Symbol text(format.Str());
        displayBox->_Insert(displayPath, &text, kPropertySymbol);
    }
    const int selected = CalibrationNumbersCom::_ValueForCalibration(current);
    for (unsigned long i = 0; i < comboBox->mEntries.mSize; i++) {
        if (comboBox->mEntries[i] == selected) {
            comboBox->_SetCurrentIndex(i);
            break;
        }
    }
}

// Stores the offset of the combo box's selected entry, when one is
// selected. Inlined into OnMsg. Name not in the reference map.
void StoreSelectedCalibration(
    const UIComboBoxEnumCom* comboBox,
    void (SystemOptions::*set)(float offset, bool force)) {
    if (comboBox->mCurrentIndex != UIComboBoxEnumCom::kNoIndex && comboBox->mCurrentIndex < comboBox->mEntries.mSize) {
        const float calibration = CalibrationNumbersCom::_CalibrationForValue(comboBox->mEntries[comboBox->mCurrentIndex]);
        (theRBProfileMgr->GetSystemOptions()->*set)(-calibration, false);
    }
}

}  // namespace

// The object's statics, in the order of its static initializer (0xDB1930).
Symbol CalibrationNumbersCom::sId("CalibrationNumbers");
Symbol CalibrationNumbersCom::sClassName("CalibrationNumbers");
PropRegistry CalibrationNumbersCom::sPropRegistry;
ComMetaData CalibrationNumbersCom::sMetaData;

// Reconstructed from eboot.elf at 0xDB0280.
CalibrationNumbersCom::CalibrationNumbersCom()
    : mAudio(GameObjectId{0xFFFFFFFFU}), mVideo(GameObjectId{0xFFFFFFFFU}), mRuntime() {}

// Reconstructed from eboot.elf at 0xDB02C0.
void CalibrationNumbersCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    CalibrationNumbersCom prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    metadata.mDescription = "Sets up the combo boxes for entering calibration numbers manually.";
    metadata.mAuthor = "Don Wurster";
    metadata.mCategory = ComMetaData::kCategoryMiscellaneous;
    metadata.mAllowedResources.push_back(RndSceneResource::Id());
    metadata.mRootOnly = true;

    GameObjectIdMetadata& audio = TypeSpecificMetadata(
        registry.RegisterProp(
            "audio", offsetof(CalibrationNumbersCom, mAudio), kPropertyGameObjectId, sizeof(GameObjectId)),
        prototype.mAudio);
    audio.mDefault = prototype.mAudio;
    static_cast<void>(Symbol("prop"));
    audio.mHelp = String("Audio combo box.");
    audio.mRequiredComs = {InstanceCom::sId};

    GameObjectIdMetadata& video = TypeSpecificMetadata(
        registry.RegisterProp(
            "video", offsetof(CalibrationNumbersCom, mVideo), kPropertyGameObjectId, sizeof(GameObjectId)),
        prototype.mVideo);
    video.mDefault = prototype.mVideo;
    static_cast<void>(Symbol("prop"));
    video.mHelp = String("Video combo box");
    video.mRequiredComs = {InstanceCom::sId};
}

// Reconstructed from eboot.elf at 0xDB0800. The deleting destructor is at
// 0xDB1740.
CalibrationNumbersCom::~CalibrationNumbersCom() {}

// Reconstructed from eboot.elf at 0xDB0810.
bool CalibrationNumbersCom::_OnResourcesLoaded() {
    return true;
}

// Reconstructed from eboot.elf at 0xDB0820.
float CalibrationNumbersCom::_CalibrationForValue(int value) {
    return static_cast<float>(value);
}

// Reconstructed from eboot.elf at 0xDB0830.
int CalibrationNumbersCom::_ValueForCalibration(float calibration) {
    return static_cast<int>(calibration) / kCalibrationStep * kCalibrationStep;
}

// Reconstructed from eboot.elf at 0xDB0850. The input component and the
// combo boxes are not checked for null, and the two calibration number
// symbols are built but not used. The stored offsets are negated.
void CalibrationNumbersCom::_Enter() {
    Entity* const scene = FindSceneEntity(mObject->mEntity);
    if (scene != nullptr) {
        scene->GetRoot()->GetCom<RBUIInputCom>()->RegisterCom(this);
    }

    if (mAudio.mId != gNullObjectId.mId) {
        static Symbol audioNumber;
        if (audioNumber == Symbol()) {
            audioNumber = Symbol("audio_calibration_number");
        }
        const InstanceCom* const instance = mObject->mEntity->GetObject(mAudio)->GetExistingBaseCom<InstanceCom>();
        mRuntime.mAudioComboBox = instance->mRuntime.mEntity->GetRoot()->GetCom<UIComboBoxEnumCom>();
        FillCalibrationComboBox(
            mRuntime.mAudioComboBox,
            kMinAudioCalibration,
            kMaxAudioCalibration,
            -theRBProfileMgr->GetSystemOptions()->mAudioOffset);
    }

    if (mVideo.mId != gNullObjectId.mId) {
        static Symbol videoNumber;
        if (videoNumber == Symbol()) {
            videoNumber = Symbol("video_calibration_number");
        }
        const InstanceCom* const instance = mObject->mEntity->GetObject(mVideo)->GetExistingBaseCom<InstanceCom>();
        mRuntime.mVideoComboBox = instance->mRuntime.mEntity->GetRoot()->GetCom<UIComboBoxEnumCom>();
        FillCalibrationComboBox(
            mRuntime.mVideoComboBox,
            kMinVideoCalibration,
            kMaxVideoCalibration,
            -theRBProfileMgr->GetSystemOptions()->mVideoOffset);
    }
}

// Reconstructed from eboot.elf at 0xDB1410. Node 3 of the message is the
// selected object's name.
DataNode CalibrationNumbersCom::OnMsg(const UISelectMsg& msg) {
    static Symbol confirmButton;
    if (confirmButton == Symbol()) {
        confirmButton = Symbol("confirm_button");
    }
    if (msg.mData->Sym(3) == confirmButton) {
        StoreSelectedCalibration(mRuntime.mAudioComboBox, &SystemOptions::SetAudioOffset);
        StoreSelectedCalibration(mRuntime.mVideoComboBox, &SystemOptions::SetVideoOffset);
        theRBProfileMgr->GetSystemOptions()->SetHasCalibrated(true);
        const eastl::vector<RBProfile*> profiles = theRBProfileMgr->GetParticipatingProfiles();
        for (RBProfile* const profile : profiles) {
            profile->mAchData->LogCalibrationPerformed();
        }
        theRBMetaState.Goto(kLayoutCalibrationAuto, false);
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xDB1620.
DataNode CalibrationNumbersCom::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    if (msg->Sym(1) == UISelectMsg::Event()) {
        DataNode result = OnMsg(UISelectMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xDB1760.
Symbol CalibrationNumbersCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0xDB1770.
Symbol CalibrationNumbersCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0xDB1780.
int CalibrationNumbersCom::CurrentRev() const {
    return const_cast<CalibrationNumbersCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0xDB17A0.
bool CalibrationNumbersCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xDB17D0.
Component* CalibrationNumbersCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0xDB17E0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* CalibrationNumbersCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<CalibrationNumbersCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) CalibrationNumbersCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0xDB1910.
PropRegistry& CalibrationNumbersCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0xDB1920.
ComMetaData& CalibrationNumbersCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xBA0AE0. The binary emits the factory
// with the other rb_meta factories.
Component* CalibrationNumbersCom::_Create() {
    return new CalibrationNumbersCom();
}

// Reconstructed from eboot.elf at 0xB638A0. The binary emits the
// registration with the other rb_meta classes'.
void CalibrationNumbersCom::Init() {
    InitComponentClass<CalibrationNumbersCom>();
}
