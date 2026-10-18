// utl/PollMgr.o (0x24CB60 to 0x2518E0): poll groups, the poll manager and
// its worker threads.
#include "utl/threading/PollMgr.h"

#include <kernel.h>
#include <unistd.h>

#include "audio/core/output/AudioRenderTarget.h"
#include "entity/core/Component.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/core/GameObject.h"
#include "os/system/System.h"
#include "utl/containers/Set.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataFunc.h"
#include "utl/data/DataNode.h"
#include "utl/text/MakeString.h"
#include "utl/text/TextStream.h"
#include "utl/threading/Thread.h"

namespace {

// The id of no object. The binary reads it from a shared header's global
// (0x19E7E68), which the object's static initializer sets to -1. Name not
// in the reference map.
constexpr unsigned int kNoObject = 0xFFFFFFFFU;

// The thread mask bits a job may name; a job with none runs anywhere.
// Name not in the reference map.
constexpr unsigned short kWorkerMask = 0x7F;

// Holds a lock that exists only while workers run, such as PollMgr::sCS.
// Name not in the reference map.
class ScopedWorkerCritSec {
public:
    explicit ScopedWorkerCritSec(CritSec* critSec) : mCritSec(critSec) {
        if (mCritSec != nullptr) {
            mCritSec->Enter();
        }
    }
    ~ScopedWorkerCritSec() {
        if (mCritSec != nullptr) {
            mCritSec->Exit();
        }
    }

private:
    CritSec* mCritSec;
};

// Searches a group's jobs for a dependency cycle (PollGroup::CheckForCycles).
// Only the depth-first visit survives in this build. Names not in the
// reference map; the evidence is weak.
class PollCycleSearch {
public:
    // Empty in this build.
    void AddJobs(PollDepBase* head);  // 0x251500
    // The cycle found, as the jobs from its end back; empty in this build.
    eastl::list<PollDepBase*> FindCycle(bool report);  // 0x251520
    // Visits the job's afters depth first; true when the job is on the
    // current path, or one of its afters leads back to it, which is then
    // added to the cycle.
    bool _Visit(PollDepBase* job, eastl::list<PollDepBase*>& cycle);  // 0x251560

    eastl::list<PollDepBase*> mRoots;
    // The jobs on the current path, and the jobs to search.
    eastl::set<PollDepBase*> mOnPath;
    eastl::set<PollDepBase*> mJobs;
};

}  // namespace

// The calling thread's poll state. The map's PollMgr::sThreadPolls; this
// build keeps it in the per-thread block at the offset recorded at
// 0x19B03D0.
thread_local ThreadPollContext gEntityThreadState;

// The statics, in the order of the static initializer (0x251230).
Condition PollMgr::sWorkerCondition;
Condition PollMgr::sAudioWorkerCondition;
eastl::vector<NamedThread*> PollMgr::sWorkerThreads;
eastl::vector<PollMgr::ThreadDesc> PollMgr::sThreadDescs;
PollDepBase::PollList PollMgr::sThreadedJobs;
PollDepBase::PollList PollMgr::sHeavyJobs;
eastl::vector<const char*> PollMgr::sAudioLockStateNames = {
    "kUnlocked",
    "kNeedLock",
    "kLocking",
    "kLocked",
    "kNeedUnlock",
    "kLockingNeedUnlock",
    "kUnlocking",
};
eastl::vector<PollMgr::LoadResourcesEntry> PollMgr::sLoadResourcesQueue;
CritSec PollMgr::sLoadResourcesCrit;

bool PollMgr::sThreadsEnabled;
bool PollMgr::sEarlyFreeToPoll;
CritSec* PollMgr::sCS;
CritSec* PollMgr::sAudioWorkerCS;
unsigned int PollMgr::sBusyMask;
bool PollMgr::sTerminating;
bool PollMgr::sSleepPostJob;
bool PollMgr::sAudioThreadBlocking;
bool PollMgr::sAudioBlocked;
PollMgr* PollMgr::sCurrent;
unsigned short PollMgr::sMaxPollWeight = 20;
std::atomic<int> PollMgr::sAudioLockState;

// Reconstructed from eboot.elf at 0x24CB60. The name is copied, truncated
// to 32 characters.
PollGroup::PollGroup(const char* name)
    : mHead(this),
      mTail(this),
      mState(kStateIdle),
      mThreaded(false),
      mPersistent(true),
      mShareParentGroup(false),
      mPostPollGroup(nullptr) {
    mName = name;
    mHead.mPollWeight = 0;
    mTail.mPollWeight = 0;
    mHead.mPollGroup = this;
    mTail.mPollGroup = this;
    mTail.AddPollDep(&mHead);
}

// Reconstructed from eboot.elf at 0x24CDA0.
PollGroup::~PollGroup() {}

// Reconstructed from eboot.elf at 0x24CEE0.
void PollGroup::SetThreaded(bool threaded) {
    mThreaded = threaded;
    mTail.SetThreaded(threaded);
}

// Reconstructed from eboot.elf at 0x24CF00.
void PollGroup::SetPersistent(bool persistent) {
    mPersistent = persistent;
}

// Reconstructed from eboot.elf at 0x24CF10.
void PollGroup::SetCallbacks(std::function<void()> onPollStart, std::function<void()> onPollEnd) {
    mOnPollStart = onPollStart;
    mOnPollEnd = onPollEnd;
}

// Reconstructed from eboot.elf at 0x24CFC0. The three pairs of slot 8 and 9
// calls are what remains of compiled-out checks.
void PollGroup::_AddLeafJobs(PollDepBase* job) {
    PollDepBase* dep = job;
    if (PollGroupJob* head = job->AsPollGroupHead()) {
        dep = &head->mOwner->mTail;
    }
    if (dep == &mTail) {
        return;
    }
    if (dep->AsPollGroupHead() == nullptr) {
        static_cast<void>(dep->AsPollGroupTail());
    }
    if (dep->AsPollGroupHead() != nullptr) {
        static_cast<void>(dep->AsPollGroupHead());
    }
    if (dep->AsPollGroupTail() != nullptr) {
        static_cast<void>(dep->AsPollGroupTail());
    }
    if (dep->mNumAfters != 0) {
        const unsigned long count = dep->mAfters.size();
        for (unsigned long i = 0; i != count; ++i) {
            _AddLeafJobs(dep->mAfters[i]);
        }
    } else {
        mTail.AddPollDep(dep);
        mLeafJobs.push_back(*dep);
    }
}

// Reconstructed from eboot.elf at 0x24D0D0. The post-polls are unlinked
// without being told.
void PollGroup::Reset() {
    static_cast<void>(scePthreadSelf());
    while (!mPostPolls.empty()) {
        mPostPolls.remove(mPostPolls.front());
    }
    mHead._Reset();
    mTail._Reset();
    while (!mLeafJobs.empty()) {
        mLeafJobs.remove(mLeafJobs.front());
    }
    mTail.AddPollDep(&mHead);
}

// Reconstructed from eboot.elf at 0x24D190.
void PollGroup::ClearJobs() {
    static_cast<void>(scePthreadSelf());
    while (!mLeafJobs.empty()) {
        PollDepBase& job = mLeafJobs.front();
        mTail.RemovePollDep(&job);
        mLeafJobs.remove(job);
    }
    mHead.ClearAllDeps();
    mTail.AddPollDep(&mHead);
}

// Reconstructed from eboot.elf at 0x24D270. The first read of the tail's
// counter is what remains of a compiled-out check.
int PollGroup::_ClearLeafJobs() {
    static_cast<void>(mTail._NextDepCount().load());
    while (!mLeafJobs.empty()) {
        PollDepBase& job = mLeafJobs.front();
        mTail.RemovePollDep(&job);
        mLeafJobs.remove(job);
    }
    return mTail._CurDepCount();
}

// Reconstructed from eboot.elf at 0x24D310.
void PollGroup::PollAfter(PollGroup* other) {
    static_cast<void>(scePthreadSelf());
    mHead.AddPollDep(&other->mTail);
}

// Reconstructed from eboot.elf at 0x24D340.
void PollGroup::RemovePollAfter(PollGroup* other) {
    static_cast<void>(scePthreadSelf());
    mHead.RemovePollDep(&other->mTail);
}

// Reconstructed from eboot.elf at 0x24D370.
bool PollGroup::TryRemovePollAfter(PollGroup* other) {
    static_cast<void>(scePthreadSelf());
    return mHead.TryRemovePollDep(&other->mTail);
}

// Reconstructed from eboot.elf at 0x24D3A0.
void PollGroup::AddJob(PollDepBase* job) {
    static_cast<void>(scePthreadSelf());
    job->AddPollDep(&mHead);
    _AddLeafJobs(job);
}

// Reconstructed from eboot.elf at 0x24D3F0.
void PollGroup::AddPollGroups(eastl::list<PollGroup*>& groups) {
    static_cast<void>(scePthreadSelf());
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        PollGroup* const group = *it;
        if (group->mHead._CurDepCount() == 0) {
            group->mHead.AddPollDep(&mHead);
            _AddLeafJobs(&group->mTail);
        }
    }
}

// Reconstructed from eboot.elf at 0x24D490.
void PollGroup::AddPollGroup(PollGroup* group) {
    static_cast<void>(scePthreadSelf());
    group->mHead.AddPollDep(&mHead);
    _AddLeafJobs(&group->mTail);
}

// Reconstructed from eboot.elf at 0x24D4D0.
void PollGroup::_PostPoll() {
    static_cast<void>(scePthreadSelf());
    mState = kStatePostPolling;
    const int thread = 0;
    while (!mPostPolls.empty()) {
        PollDepBase& job = mPostPolls.front();
        job._RemovePostPoll();
        job.mPollGroup = mPostPollGroup;
        job.PostPoll(thread);
        job.mPollGroup = nullptr;
    }
    mState = kStateIdle;
}

// Reconstructed from eboot.elf at 0x24D590.
void PollGroup::_Finish() {
    gEntityThreadState.mPollMgr->_AddFinishedGroup(this);
}

// Reconstructed from eboot.elf at 0x24D620.
int PollGroup::GetNumJobs() {
    static_cast<void>(scePthreadSelf());
    return static_cast<int>(mHead.mAfters.size()) - 1;
}

// Reconstructed from eboot.elf at 0x24D650.
void PollGroup::PrintAfters(TextStream& stream) {
    stream << "-- PrintAfters: " << mName.c_str() << " --\n";
    for (PollDepBase* after : mTail.mAfters) {
        if (after->AsPollGroupTail() != nullptr || after->AsPollGroupHead() != nullptr) {
            stream << after->GetPollName() << " ";
        }
    }
    stream << "\n";
}

// Reconstructed from eboot.elf at 0x24D720.
void PollGroup::Print(TextStream& stream) {
    stream << "-- PollGroups wrapped -- " << "\n";
    _PrintWrapped(stream, this, 0);
    stream << "-- PollGroup deps -- " << "\n";
    _PrintDeps(stream, this, 0);
    stream << "\n";
}

// Reconstructed from eboot.elf at 0x24D7A0. The nested groups follow,
// indented by three.
void PollGroup::_PrintWrapped(TextStream& stream, PollGroup* root, int indent) {
    for (int i = indent; i != 0; --i) {
        stream << " ";
    }
    stream.Print(mName.c_str());
    stream << "\n";
    for (PollDepBase* after : mHead.mAfters) {
        PollGroupJob* const head = after->AsPollGroupHead();
        if (after != &mTail && head != nullptr) {
            after->mPollGroup->_PrintWrapped(stream, root, indent + 3);
        }
    }
}

// Reconstructed from eboot.elf at 0x24D870. Prints the groups waiting for
// the tail, then the nested groups indented by three, then the waiting
// groups indented by one between "*>" and "<*" lines.
void PollGroup::_PrintDeps(TextStream& stream, PollGroup* root, int indent) {
    String pad;
    for (int i = indent; i != 0; --i) {
        pad << " ";
    }
    stream.Print(pad.c_str());
    stream.Print(mName.c_str());
    stream << " <- ";
    bool any = false;
    for (PollDepBase* after : mTail.mAfters) {
        if (after->AsPollGroupHead() != nullptr) {
            TextStream& line = stream << " (";
            line.Print(after->mPollGroup->mName.c_str());
            line << ")";
            any = true;
        }
    }
    if (!any) {
        stream << "none";
    }
    stream << "\n";
    for (PollDepBase* after : mHead.mAfters) {
        PollGroupJob* const head = after->AsPollGroupHead();
        if (after != &mTail && head != nullptr) {
            after->mPollGroup->_PrintDeps(stream, root, indent + 3);
        }
    }
    if (mTail.mAfters.empty()) {
        return;
    }
    stream.Print(pad.c_str());
    TextStream& open = stream << "(";
    open.Print(mName.c_str());
    open << ")" << "*>\n";
    for (PollDepBase* after : mTail.mAfters) {
        if (after->AsPollGroupHead() != nullptr && after->mPollGroup != root) {
            after->mPollGroup->_PrintDeps(stream, root, indent + 1);
        }
    }
    stream.Print(pad.c_str());
    TextStream& close = stream << "(";
    close.Print(mName.c_str());
    close << ")" << "<*\n";
}

// Reconstructed from eboot.elf at 0x24DB60.
PollGroup::HeadJob::HeadJob(PollGroup* owner) : PollGroupJob(owner) {}

// Reconstructed from eboot.elf at 0x24DBB0.
const char* PollGroup::HeadJob::GetPollName() const {
    return "<PollGroup Head>";
}

// Reconstructed from eboot.elf at 0x24DBC0.
PollGroup::TailJob::TailJob(PollGroup* owner) : PollGroupJob(owner) {}

// Reconstructed from eboot.elf at 0x24DC10.
const char* PollGroup::TailJob::GetPollName() const {
    return "<PollGroup Tail>";
}

// Reconstructed from eboot.elf at 0x24DC20. A threaded group's post-polls
// are left to its manager (PollMgr::_Poll); here the state is not reset
// after them, unlike PollGroup::_PostPoll.
void PollGroup::TailJob::ThreadPoll(const int& thread) {
    static_cast<void>(thread);
    if (mOwner->mOnPollEnd) {
        mOwner->mOnPollEnd();
    }
    PollGroup* const group = mOwner;
    if (group->mThreaded) {
        return;
    }
    group->mState = kStatePostPolling;
    const int postPollThread = 0;
    while (!group->mPostPolls.empty()) {
        PollDepBase& job = group->mPostPolls.front();
        job._RemovePostPoll();
        job.mPollGroup = group->mPostPollGroup;
        job.PostPoll(postPollThread);
        job.mPollGroup = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x24DD00.
void PollGroup::CheckForCycles() {
    PollCycleSearch search;
    search.AddJobs(&mHead);
    const eastl::list<PollDepBase*> cycle = search.FindCycle(true);
}

// Reconstructed from eboot.elf at 0x24DF10.
void PollGroup::HeadJob::ThreadPoll(const int& thread) {
    static_cast<void>(thread);
    PollGroup* const group = mOwner;
    group->mState = kStatePolling;
    if (group->mOnPollStart) {
        group->mOnPollStart();
    }
}

// Reconstructed from eboot.elf at 0x24DF50.
PollGroupJob* PollGroup::HeadJob::AsPollGroupHead() {
    return this;
}

// Reconstructed from eboot.elf at 0x24DF80.
PollGroupJob* PollGroup::TailJob::AsPollGroupTail() {
    return this;
}

// Reconstructed from eboot.elf at 0x24E0B0.
void PollMgr::Init() {
    static_cast<void>(scePthreadSelf());
    const ThreadMap::TaskDesc* const desc = ThreadMap::GetTaskSettings("job_manager");
    bool value = false;
    SystemConfig(Symbol("poll_mgr"))->FindData(Symbol("enabled_audio_thread_blocking"), value, false);
    sAudioThreadBlocking = value;
    value = false;
    SystemConfig(Symbol("poll_mgr"))->FindData(Symbol("enabled_early_free_to_poll"), value, false);
    sEarlyFreeToPoll = value;
    _StartWorkers(desc);
    DataRegisterFunc(Symbol("pollmgr"), OnPollMgr);
}

// Reconstructed from eboot.elf at 0x24E1C0. Each worker's place is set up
// from a temporary.
void PollMgr::_StartWorkers(const ThreadMap::TaskDesc* desc) {
    sThreadsEnabled = true;
    unsigned long count = desc->mNumThreads;
    if (count != sWorkerThreads.size()) {
        if (!sWorkerThreads.empty()) {
            _StopWorkers();
            count = desc->mNumThreads;
        }
        sTerminating = false;
        if (count != 0) {
            sWorkerThreads.resize(count);
            if (!sWorkerThreads.empty()) {
                sCS = new CritSec;
                sWorkerCondition.Init(*sCS);
                sAudioWorkerCS = new CritSec;
                sAudioWorkerCondition.Init(*sAudioWorkerCS);
                sThreadDescs.resize(sWorkerThreads.size());
                long audioProcessor = -1;
                if (sAudioThreadBlocking && sThreadsEnabled) {
                    audioProcessor = ThreadMap::GetTaskSettings("audio_render")->mProcessor;
                    if (audioProcessor == -1 &&
                        sWorkerThreads.size() == ThreadMap::gProcessorCount - 1) {
                        audioProcessor = static_cast<long>(ThreadMap::gProcessorCount - 1);
                    }
                }
                for (unsigned int i = 0; i < sWorkerThreads.size(); ++i) {
                    NamedThread* const thread = new NamedThread;
                    thread->Init("Unknown Thread!");
                    sWorkerThreads[i] = thread;
                    sThreadDescs[i] = ThreadDesc(static_cast<int>(i), 1U << i);
                    const ThreadMap::ThreadDesc& settings = desc->GetThread(i);
                    int (*entry)(void*) = _WorkerMain;
                    if (audioProcessor != -1 &&
                        (audioProcessor == settings.mProcessor || i >= ThreadMap::gProcessorCount - 2)) {
                        entry = _AudioWorkerMain;
                        audioProcessor = -1;
                    }
                    const char* const name = (FormatString("PollMgr Worker %d") << i).Str();
                    sWorkerThreads[i]->Create(
                        entry,
                        &sThreadDescs[i],
                        name,
                        settings.mProcessor,
                        settings.mPriority,
                        settings.mStackSize,
                        settings.mAffinityMask);
                    sWorkerThreads[i]->mThread.Start();
                }
            }
        }
    }
    if (sWorkerThreads.empty()) {
        sThreadsEnabled = false;
        sAudioThreadBlocking = false;
        sEarlyFreeToPoll = false;
    }
}

// Reconstructed from eboot.elf at 0x24E800. The error messages are built
// and dropped.
DataNode PollMgr::OnPollMgr(DataArray* msg) {
    static_cast<void>(scePthreadSelf());
    static Symbol toggleThreads;
    if (toggleThreads == Symbol()) {
        toggleThreads = Symbol("toggle_threads");
    }
    static Symbol setMaxWeight;
    if (setMaxWeight == Symbol()) {
        setMaxWeight = Symbol("set_max_weight");
    }
    static Symbol toggleSleepPostJob;
    if (toggleSleepPostJob == Symbol()) {
        toggleSleepPostJob = Symbol("toggle_sleep_post_job");
    }
    static Symbol setThreadPriority;
    if (setThreadPriority == Symbol()) {
        setThreadPriority = Symbol("set_thread_priority");
    }
    static Symbol toggleEarlyFreeToPoll;
    if (toggleEarlyFreeToPoll == Symbol()) {
        toggleEarlyFreeToPoll = Symbol("toggle_early_free_to_poll");
    }
    String error;
    if (msg->Size() >= 2) {
        if (msg->Node(1).Type() != kDataSymbol) {
            error = "Unrecognized cmd type to 'pollmgr <Symbol cmd> <params...>'. Cmd should be Symbol.";
        } else if (msg->Sym(1) == toggleThreads) {
            SetThreadsEnabled(!sThreadsEnabled);
        } else if (msg->Sym(1) == setMaxWeight) {
            if (msg->Size() != 3 || msg->Node(2).Type() != kDataInt) {
                error = "Unspecified max_weight value in 'pollmgr set_max_weight <int>'";
            } else {
                const unsigned short weight = static_cast<unsigned short>(msg->Int(2));
                sMaxPollWeight = weight > 1 ? weight : 1;
            }
        } else if (msg->Sym(1) == toggleSleepPostJob) {
            sSleepPostJob = !sSleepPostJob;
        } else if (msg->Sym(1) == setThreadPriority) {
        } else if (msg->Sym(1) == toggleEarlyFreeToPoll) {
            sEarlyFreeToPoll = !sEarlyFreeToPoll;
        } else {
            error = "Unrecognized console cmd to 'pollmgr <Symbol cmd> <params...>'. Please try again.";
        }
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x24EC10.
void PollMgr::Terminate() {
    _StopWorkers();
}

// Reconstructed from eboot.elf at 0x24EC20.
void PollMgr::_StopWorkers() {
    sTerminating = true;
    if (sCS == nullptr) {
        return;
    }
    sWorkerCondition.Broadcast();
    sAudioWorkerCondition.Broadcast();
    for (unsigned int i = 0; i < sWorkerThreads.size(); ++i) {
        sWorkerThreads[i]->mThread._Join();
        NamedThread* const thread = sWorkerThreads[i];
        if (thread != nullptr) {
            thread->mThread._ForceKillThread();
            delete thread;
        }
    }
    sWorkerThreads.clear();
    sBusyMask = 0;
    sThreadsEnabled = false;
    sAudioWorkerCondition.Destroy();
    delete sAudioWorkerCS;
    sAudioWorkerCS = nullptr;
    sWorkerCondition.Destroy();
    delete sCS;
    sCS = nullptr;
    sThreadDescs.clear();
}

// Reconstructed from eboot.elf at 0x24EEA0. Passes the worker's mask bit
// as the thread to the jobs.
int PollMgr::_WorkerMain(void* desc) {
    ThreadDesc& thread = *static_cast<ThreadDesc*>(desc);
    const int threadMask = static_cast<int>(thread.mMask);
    ThreadPollContext& context = gEntityThreadState;
    PollDepBase::PollList jobs;
    while (!sTerminating) {
        while (sCurrent != nullptr) {
            _PullJobFromWorker(thread, jobs);
            if (jobs.empty()) {
                scePthreadYield();
            } else {
                context.mPollMgr = sCurrent;
                _DoJob(jobs, threadMask);
                context.mPollMgr = nullptr;
                if (sSleepPostJob) {
                    usleep(0);
                }
            }
            if (sTerminating) {
                return 0;
            }
        }
        ScopedWorkerCritSec lock(sCS);
        while (!sTerminating && sCurrent == nullptr) {
            sWorkerCondition.Wait();
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x24F150. Woken with the lock requested,
// the worker takes the mixer lock and polls until it is asked to drop it.
int PollMgr::_AudioWorkerMain(void* desc) {
    ThreadDesc& thread = *static_cast<ThreadDesc*>(desc);
    const int threadMask = static_cast<int>(thread.mMask);
    ThreadPollContext& context = gEntityThreadState;
    PollDepBase::PollList jobs;
    bool beginMix = false;
    while (!sTerminating) {
        bool endMix;
        for (;;) {
            if (beginMix) {
                gAudioRenderTargets.mDefault->TryBeginMix();
                beginMix = false;
                int expected = kLocking;
                if (sAudioLockState.compare_exchange_strong(expected, kLocked)) {
                    endMix = false;
                } else {
                    expected = kLockingNeedUnlock;
                    endMix = sAudioLockState.compare_exchange_strong(expected, kUnlocking);
                }
            } else {
                int expected = kNeedUnlock;
                endMix = sAudioLockState.compare_exchange_strong(expected, kUnlocking);
            }
            if (sCurrent == nullptr || endMix) {
                break;
            }
            _PullJobFromWorker(thread, jobs);
            if (jobs.empty()) {
                scePthreadYield();
            } else {
                context.mPollMgr = sCurrent;
                _DoJob(jobs, threadMask);
                context.mPollMgr = nullptr;
                if (sSleepPostJob) {
                    usleep(0);
                }
            }
            if (sTerminating) {
                return 0;
            }
        }
        {
            ScopedWorkerCritSec lock(sCS);
            sBusyMask &= ~thread.mMask;
        }
        if (endMix) {
            int expected = kUnlocking;
            sAudioLockState.compare_exchange_strong(expected, kUnlocked);
            gAudioRenderTargets.mDefault->EndMix();
        }
        ScopedWorkerCritSec lock(sAudioWorkerCS);
        for (;;) {
            if (sTerminating) {
                break;
            }
            if (sCurrent != nullptr) {
                int expected = kNeedLock;
                if (sAudioLockState.compare_exchange_strong(expected, kLocking)) {
                    beginMix = true;
                    break;
                }
            }
            sAudioWorkerCondition.Wait();
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x24F4C0.
void PollMgr::WakeWorkers(bool all) {
    if (all) {
        if (sCS != nullptr) {
            sWorkerCondition.Broadcast();
            sAudioWorkerCondition.Broadcast();
        }
    } else if (sThreadsEnabled && sCS != nullptr) {
        sWorkerCondition.Broadcast();
    }
}

// Reconstructed from eboot.elf at 0x24F530.
void PollMgr::SetThreadsEnabled(bool enabled) {
    static_cast<void>(scePthreadSelf());
    sThreadsEnabled = enabled;
}

// Reconstructed from eboot.elf at 0x24F550. A lock request waits for a
// pending unlock to finish, unless the lock was still being taken.
void PollMgr::BlockAudioThread(bool block) {
    if (!sAudioThreadBlocking || !sThreadsEnabled || sAudioBlocked == block) {
        return;
    }
    sAudioBlocked = block;
    if (sCurrent != nullptr) {
        if (block) {
            int expected = kLockingNeedUnlock;
            if (sAudioLockState.compare_exchange_strong(expected, kLocking)) {
                return;
            }
            while (sAudioLockState != kUnlocked) {
            }
            {
                ScopedWorkerCritSec lock(sAudioWorkerCS);
                expected = kUnlocked;
                sAudioLockState.compare_exchange_strong(expected, kNeedLock);
            }
            sAudioWorkerCondition.Broadcast();
        } else {
            int expected = kNeedLock;
            if (!sAudioLockState.compare_exchange_strong(expected, kUnlocked)) {
                expected = kLocking;
                if (!sAudioLockState.compare_exchange_strong(expected, kLockingNeedUnlock)) {
                    expected = kLocked;
                    sAudioLockState.compare_exchange_strong(expected, kNeedUnlock);
                }
            }
        }
    } else if (block) {
        int expected = kUnlocked;
        if (sAudioLockState.compare_exchange_strong(expected, kLocked)) {
            gAudioRenderTargets.mDefault->TryBeginMix();
        }
    } else {
        int expected = kLocked;
        if (sAudioLockState.compare_exchange_strong(expected, kUnlocked)) {
            gAudioRenderTargets.mDefault->EndMix();
        }
    }
}

// Reconstructed from eboot.elf at 0x24F6E0.
bool PollMgr::IsWorkerThread(ScePthread thread) {
    for (NamedThread* worker : sWorkerThreads) {
        if (worker->mThread.mHandle == thread) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x24F720.
void PollMgr::SetMaxPollWeight(unsigned int weight) {
    sMaxPollWeight = static_cast<unsigned short>(weight > 1 ? weight : 1);
}

// Reconstructed from eboot.elf at 0x24F740. The name is not used.
PollMgr::PollMgr(const char* name)
    : mPollGroup(new PollGroup("PollMgr PollGroup")),
      mState(PollGroup::kStateIdle),
      mReserved{-1, -1, -1} {
    static_cast<void>(name);
    mPollGroup->SetThreaded(true);
    mState = PollGroup::kStatePolling;
}

// Reconstructed from eboot.elf at 0x24F820.
PollMgr::~PollMgr() {
    delete mPollGroup;
}

// Reconstructed from eboot.elf at 0x24F900.
void PollMgr::Select(PollGroupMap* pollGroups) {
    sCurrent = this;
    ThreadPollContext& context = gEntityThreadState;
    context.mPollMgr = this;
    context.mPollGroups = pollGroups;
}

// Reconstructed from eboot.elf at 0x24F9B0.
void PollMgr::Select() {
    sCurrent = this;
    ThreadPollContext& context = gEntityThreadState;
    context.mPollMgr = this;
    context.mPollGroups = nullptr;
    if (sCurrent != nullptr && sThreadsEnabled && sCS != nullptr) {
        sWorkerCondition.Broadcast();
    }
}

// Reconstructed from eboot.elf at 0x24FA90.
void PollMgr::Deselect() {
    int expected = kNeedUnlock;
    if (sAudioThreadBlocking && sThreadsEnabled &&
        sAudioLockState.compare_exchange_strong(expected, kUnlocking)) {
        gAudioRenderTargets.mDefault->EndMix();
        expected = kUnlocking;
        sAudioLockState.compare_exchange_strong(expected, kUnlocked);
    }
    sCurrent = nullptr;
    ThreadPollContext& context = gEntityThreadState;
    context.mPollMgr = nullptr;
    context.mPollGroups = nullptr;
}

// Reconstructed from eboot.elf at 0x24FB80.
void PollMgr::Sync(PollGroup* group) {
    if (group != nullptr) {
        _Poll(group);
        return;
    }
    do {
        _Poll(nullptr);
    } while (_HasPendingJobs());
    const PollGroup::State state = mState;
    mState = PollGroup::kStatePostPolling;
    _LoadQueuedResources();
    mState = state;
}

// Reconstructed from eboot.elf at 0x24FC50.
void PollMgr::_Poll(PollGroup* until) {
    PollDepBase::PollList jobs;
    if (until == nullptr) {
        while (_PullJobFromMainThread(jobs)) {
            _DoJob(jobs, 0);
        }
        _FreeFinishedGroups();
        mPollGroup->_PostPoll();
        return;
    }
    if (_TakeFinishedGroup(until)) {
        return;
    }
    while (_PullJobFromMainThread(jobs)) {
        _DoJob(jobs, 0);
        if (_TakeFinishedGroup(until)) {
            return;
        }
    }
    _TakeFinishedGroup(until);
}

// Reconstructed from eboot.elf at 0x24FFA0.
bool PollMgr::_HasPendingJobs() {
    ScopedWorkerCritSec lock(sCS);
    return sBusyMask != 0 || !mJobs.empty() || !sThreadedJobs.empty() || !sHeavyJobs.empty();
}

// Reconstructed from eboot.elf at 0x250020. A component loading its
// resources may queue more; the loop takes them too.
void PollMgr::_LoadQueuedResources() {
    ScopedCritSec lock(sLoadResourcesCrit);
    for (LoadResourcesEntry* entry = sLoadResourcesQueue.begin(); entry != sLoadResourcesQueue.end();
         ++entry) {
        if (entry->mObject.mId == kNoObject) {
            continue;
        }
        GameObject* const object = entry->mEntity->GetObject(entry->mObject);
        if (object == nullptr) {
            continue;
        }
        Component* const component = object->FindCom(entry->mComponent);
        if (component != nullptr) {
            component->mLoadResourcesQueued = false;
            component->LoadResources(false);
        }
    }
    sLoadResourcesQueue.clear();
}

// Reconstructed from eboot.elf at 0x250160. The read of the head's counter
// is what remains of a compiled-out check.
void PollMgr::StartPollGroup(PollGroup* group) {
    static_cast<void>(group->mHead._CurDepCount().load());
    const int thread = 0;
    group->mHead.ThreadPoll(thread);
    group->mHead._OnPolled(true);
}

// Reconstructed from eboot.elf at 0x2501D0. The jobs others wait for go in
// front of their list, the leaves behind it.
void PollMgr::QueueForPoll(PollDepBase::PollList* buckets) {
    if (sCS != nullptr) {
        ScopedCritSec lock(*sCS);
        sHeavyJobs.splice_front(buckets[PollDepBase::kPollBucketHeavy]);
        mJobs.splice_front(buckets[PollDepBase::kPollBucketMain]);
        mJobs.splice(buckets[PollDepBase::kPollBucketMainLeaf]);
        sThreadedJobs.splice_front(buckets[PollDepBase::kPollBucketThreaded]);
        sThreadedJobs.splice(buckets[PollDepBase::kPollBucketThreadedLeaf]);
    } else {
        mJobs.splice_front(buckets[PollDepBase::kPollBucketHeavy]);
        mJobs.splice_front(buckets[PollDepBase::kPollBucketThreaded]);
        mJobs.splice_front(buckets[PollDepBase::kPollBucketMain]);
        mJobs.splice(buckets[PollDepBase::kPollBucketThreadedLeaf]);
        mJobs.splice_front(buckets[PollDepBase::kPollBucketMainLeaf]);
    }
}

// Reconstructed from eboot.elf at 0x250450.
bool PollMgr::_TakeFinishedGroup(PollGroup* group) {
    ScopedCritSec lock(mCrit);
    LinkedList::Node& node = group->mFinishedNode;
    if (node.mNext == &node || node.mPrev == &node) {
        return false;
    }
    node.Remove();
    return true;
}

// Reconstructed from eboot.elf at 0x2504C0. The binary also works out a
// share of sMaxPollWeight per thread and a point past it from which the
// taken jobs would go back, but the point never leaves the list's end, so
// nothing goes back. Threaded jobs are taken in reverse; while workers
// exist the main thread skips jobs bound to them.
bool PollMgr::_PullJobFromMainThread(PollDepBase::PollList& jobs) {
    ScopedWorkerCritSec lock(sCS);
    jobs.splice_front(mJobs);
    if (!jobs.empty()) {
        return true;
    }
    if (!sHeavyJobs.empty()) {
        PollDepBase& job = sHeavyJobs.front();
        sHeavyJobs.remove(job);
        jobs.push_back(job);
        return true;
    }
    int weight = 0;
    PollDepBase* job = sThreadedJobs.empty() ? nullptr : &sThreadedJobs.front();
    while (job != nullptr && weight < sMaxPollWeight) {
        if (sCS != nullptr) {
            while (job != nullptr && (job->mFlags & kWorkerMask) != 0) {
                job = sThreadedJobs.next(*job);
            }
            if (job == nullptr) {
                break;
            }
        }
        weight += job->mPollWeight;
        PollDepBase* const next = sThreadedJobs.next(*job);
        sThreadedJobs.remove(*job);
        jobs.push_front(*job);
        job = next;
    }
    if (jobs.empty()) {
        return sBusyMask != 0 || !sThreadedJobs.empty();
    }
    return true;
}

// Reconstructed from eboot.elf at 0x2506F0.
void PollMgr::_DoJob(PollDepBase::PollList& jobs, int thread) {
    while (!jobs.empty()) {
        PollDepBase& job = jobs.front();
        jobs.remove(job);
        static_cast<void>(scePthreadSelf());
        const bool enabled = job.IsPollEnabled();
        if (enabled) {
            job._DoThreadPoll(thread);
        }
        job._OnPolled(enabled);
        if (PollGroupJob* tail = job.AsPollGroupTail()) {
            tail->mOwner->_Finish();
        }
    }
}

// Reconstructed from eboot.elf at 0x250790.
void PollMgr::_FreeFinishedGroups() {
    LinkedList::List<PollGroup, PollGroup::FinishedNode> finished;
    finished.splice(mFinishedGroups);
    while (!finished.empty()) {
        PollGroup& group = finished.front();
        finished.remove(group);
        if (!group.mPersistent) {
            group.ClearJobs();
            delete &group;
        }
    }
}

// Reconstructed from eboot.elf at 0x250870.
void PollMgr::_AddFinishedGroup(PollGroup* group) {
    ScopedCritSec lock(mCrit);
    mFinishedGroups.push_back(*group);
    group->mState = PollGroup::kStateIdle;
}

// Reconstructed from eboot.elf at 0x2508D0. A worker takes the jobs bound
// to no worker or to it.
bool PollMgr::_PullJobFromWorker(ThreadDesc& desc, PollDepBase::PollList& jobs) {
    const unsigned int mask = desc.mMask;
    const unsigned int maxWeight = sMaxPollWeight;
    ScopedWorkerCritSec lock(sCS);
    if (sThreadedJobs.empty() && sHeavyJobs.empty()) {
        sBusyMask &= ~mask;
        return !sTerminating;
    }
    sBusyMask |= mask;
    if (!sHeavyJobs.empty()) {
        PollDepBase& job = sHeavyJobs.front();
        sHeavyJobs.remove(job);
        jobs.push_back(job);
        return true;
    }
    unsigned int weight = 0;
    PollDepBase* job = &sThreadedJobs.front();
    while (weight < maxWeight) {
        while ((job->mFlags & kWorkerMask) != 0 && (job->mFlags & mask & kWorkerMask) == 0) {
            job = sThreadedJobs.next(*job);
            if (job == nullptr) {
                return true;
            }
        }
        weight += job->mPollWeight;
        PollDepBase* const next = sThreadedJobs.next(*job);
        sThreadedJobs.remove(*job);
        jobs.push_back(*job);
        if (next == nullptr) {
            break;
        }
        job = next;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x250AD0.
void PollMgr::QueueForLoadResources(GameObject* object, Component* component) {
    if (component->mLoadResourcesQueued) {
        return;
    }
    const LoadResourcesEntry entry{object->mEntity, object->mId, component->GetId()};
    {
        ScopedCritSec lock(sLoadResourcesCrit);
        sLoadResourcesQueue.push_back(entry);
    }
    component->mLoadResourcesQueued = true;
}

// Reconstructed from eboot.elf at 0x250C40.
void PollMgr::DequeueForLoadResourcesIfNeeded(Entity* entity) {
    ScopedCritSec lock(sLoadResourcesCrit);
    LoadResourcesEntry* const end = sLoadResourcesQueue.end();
    LoadResourcesEntry* out = sLoadResourcesQueue.begin();
    while (out != end && out->mEntity != entity) {
        ++out;
    }
    if (out != end) {
        for (LoadResourcesEntry* in = out; in != end; ++in) {
            if (in->mEntity != entity) {
                *out++ = *in;
            }
        }
    }
    sLoadResourcesQueue.erase(out, end);
}

// Reconstructed from eboot.elf at 0x2514C0.
void PollChildEntity(Entity* parent, Entity* child) {
    child->mPollGroup = parent->mPollGroup;
    child->mResource->PollEntity(child);
    child->mPollGroup = nullptr;
}

// Reconstructed from eboot.elf at 0x251500.
void PollCycleSearch::AddJobs(PollDepBase* head) {
    static_cast<void>(head);
}

// Reconstructed from eboot.elf at 0x251520.
eastl::list<PollDepBase*> PollCycleSearch::FindCycle(bool report) {
    static_cast<void>(report);
    return eastl::list<PollDepBase*>();
}

// Reconstructed from eboot.elf at 0x251560. Only jobs in mJobs are
// searched.
bool PollCycleSearch::_Visit(PollDepBase* job, eastl::list<PollDepBase*>& cycle) {
    bool found = mOnPath.find(job) != mOnPath.end();
    if (!found && mJobs.find(job) != mJobs.end()) {
        mOnPath.insert(job);
        for (PollDepBase* after : job->mAfters) {
            if (_Visit(after, cycle)) {
                cycle.push_back(after);
                found = true;
                break;
            }
        }
        if (!found) {
            mJobs.insert(job);
        }
    }
    mOnPath.erase(job);
    return found;
}
