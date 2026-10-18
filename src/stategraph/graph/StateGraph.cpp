// stategraph/StateGraph.o (0x6F6090 to 0x7013A0). The registrations of the
// reconstructed classes, which the binary emits here, are defined with
// their classes.
#include "stategraph/graph/StateGraph.h"

#include "stategraph/base/StateBase.h"
#include "stategraph/blendtrees/BlendTreeBaseCom.h"
#include "stategraph/blendtrees/ClipBlendTreeCom.h"
#include "stategraph/blendtrees/ClipJukeboxCom.h"
#include "stategraph/conditions/StateCondition.h"
#include "stategraph/conditions/StateConditionComparison.h"
#include "stategraph/conditions/StateConditionEvent.h"
#include "stategraph/conditions/StateConditionEventParameter.h"
#include "stategraph/conditions/StateConditionLogic.h"
#include "stategraph/conditions/StateConditionNodeFinished.h"
#include "stategraph/conditions/StateConditionNot.h"
#include "stategraph/conditions/StateConditionObjectDistance.h"
#include "stategraph/conditions/StateConditionPropertyComparison.h"
#include "stategraph/conditions/StateConditionRandom.h"
#include "stategraph/conditions/StateConditionSoundProgress.h"
#include "stategraph/conditions/StateConditionTimeElapsed.h"
#include "stategraph/drivers/StateGraphDriverCom.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/nodes/StateNodeAnimate.h"
#include "stategraph/nodes/StateNodeBlendTreeBase.h"
#include "stategraph/nodes/StateNodeClipBlendTree.h"
#include "stategraph/nodes/StateNodeClipJukebox.h"
#include "stategraph/nodes/StateNodeExecute.h"
#include "stategraph/nodes/StateNodeFolder.h"
#include "stategraph/nodes/StateNodeGroup.h"
#include "stategraph/nodes/StateNodeModifySound.h"
#include "stategraph/nodes/StateNodePlaySound.h"
#include "stategraph/nodes/StateNodeRoot.h"
#include "stategraph/nodes/StateNodeSetAnimFrame.h"
#include "stategraph/nodes/StateNodeSetProperty.h"
#include "stategraph/nodes/StateNodeSetValue.h"
#include "stategraph/nodes/StateNodeSubGraph.h"
#include "stategraph/nodes/StateNodeTriggerAction.h"
#include "stategraph/nodes/StateNodeTriggerAudioClip.h"
#include "stategraph/resources/StateGraphResource.h"
#include "stategraph/transitions/StateTransition.h"

// Reconstructed from eboot.elf at 0x6F6090. The base classes register
// before their subclasses.
void StateGraph::Init() {
    StateGraphResource::Init();
    StateGraphDriverCom::Init();
    StateBase::Init();
    StateNode::Init();
    StateNodeRoot::Init();
    StateNodeAnimate::Init();
    BlendTreeBaseCom::Init();
    ClipBlendTreeCom::Init();
    ClipJukeboxCom::Init();
    StateNodeBlendTreeBase::Init();
    StateNodeClipBlendTree::Init();
    StateNodeClipJukebox::Init();
    StateNodeExecute::Init();
    StateNodeFolder::Init();
    StateNodeGroup::Init();
    StateNodeSetValueInit();
    StateNodeSetAnimFrame::Init();
    StateNodeSubGraph::Init();
    StateNodeTriggerAction::Init();
    StateNodeTriggerAudioClip::Init();
    RegisterPropStruct<DrivenAudioParamSettings>(Symbol("DrivenAudioParamSettings"));
    StateNodeModifySound::Init();
    StateNodePlaySound::Init();
    StateNodeSetPropertyInit_A();
    StateNodeSetPropertyInit_B();
    StateNodeSetPropertyInit_C();
    StateNodeIncrementPropertyInit_A();
    StateNodeIncrementPropertyInit_B();
    StateNodeIncrementPropertyInit_C();
    StateTransition::Init();
    StateCondition::Init();
    StateConditionComparisonInit_A();
    StateConditionComparisonInit_B();
    StateConditionEvent::Init();
    StateConditionLogic::Init();
    StateConditionNodeFinished::Init();
    StateConditionNot::Init();
    StateConditionObjectDistance::Init();
    StateConditionRandom::Init();
    StateConditionSoundProgress::Init();
    StateConditionTimeElapsed::Init();
    StateConditionPropertyComparisonInit_A();
    StateConditionPropertyComparisonInit_B();
    StateConditionPropertyComparisonInit_C();
    StateConditionEventParameterInit();
}
