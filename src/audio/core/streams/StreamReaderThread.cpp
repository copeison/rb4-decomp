#include "audio/core/streams/StreamReaderThread.h"

#include <cerrno>
#include <cstring>

#include "audio/fmod/api/fmod_api.h"

namespace {

// Result given to readers still queued when the thread quits. Name not in
// the reference map.
constexpr int kReaderCancelled = 15;

// Whether the semaphore was initialized; the engine reads the first word of
// the sem_t. Names not in the reference map.
bool SemaphoreInitialized(const sem_t& semaphore) {
    int word = 0;
    std::memcpy(&word, &semaphore, sizeof(word));
    return word != 0;
}

void ClearSemaphoreWord(sem_t& semaphore) {
    const int word = 0;
    std::memcpy(&semaphore, &word, sizeof(word));
}

}  // namespace

// Reconstructed from eboot.elf at 0x263EC0.
void StreamReader::Setup(
    FMOD::Sound* sound,
    int numChannels,
    short* buffer,
    int bufferBytes,
    const std::function<void()>& onDone) {
    mState = kStateIdle;
    mOnDone = onDone;
    mSound = sound;
    mBuffer = buffer;
    mBufferBytes = bufferBytes;
    mNumChannels = numChannels;
}

// Reconstructed from eboot.elf at 0x263F70. A failed read other than end of
// file silences the buffer's start and stops.
void StreamReader::ReadSegments() {
    mFramesRead = 0;
    int framesRead = 0;
    auto* output = reinterpret_cast<char*>(mBuffer);
    int numChannels = mNumChannels;
    for (int index = 0; index < kMaxSegments; ++index) {
        int count = mSegments[index].mCount;
        if (count == 0) {
            break;
        }
        int start = mSegments[index].mStart;
        if (start < 0) {
            const long silence = -static_cast<long>(start);
            framesRead -= start;
            count += start;
            start = 0;
            mFramesRead = framesRead;
            std::memset(output, 0, mNumChannels * 2 * silence);
            output += 2 * silence * mNumChannels;
        }
        mSound->seekData(static_cast<unsigned int>(start));
        const int bytes = 2 * count * mNumChannels;
        unsigned int read = 0;
        mResult = mSound->readData(output, static_cast<unsigned int>(bytes), &read);
        if ((mResult | FMOD_ERR_FILE_EOF) != FMOD_ERR_FILE_EOF) {
            std::memset(mBuffer, 0, sizeof(int) * count);
            return;
        }
        unsigned int got = static_cast<unsigned int>(bytes);
        if (got != read) {
            std::memset(output + read, 0, got - read);
            got = read;
        }
        output += 2 * (bytes / 2);
        numChannels = mNumChannels;
        framesRead = mFramesRead + static_cast<int>(got / (2 * numChannels));
        mFramesRead = framesRead;
    }
    if (numChannels == 1 && framesRead > 0) {
        short* samples = mBuffer;
        for (int frame = framesRead; frame > 0; --frame) {
            samples[2 * frame - 2] = samples[frame - 1];
            samples[2 * frame - 1] = samples[frame - 1];
        }
    }
}

// Reconstructed from eboot.elf at 0x264140.
StreamReaderThread::StreamReaderThread() : mQuit(false) {
    mThread.Init("Unknown Thread!");
    ClearSemaphoreWord(mSemaphore);
    sem_init(&mSemaphore, 0, 0);
}

// Reconstructed from eboot.elf at 0x264260. The binary kills the thread
// after the reader list and lock are destroyed.
StreamReaderThread::~StreamReaderThread() {
    if (mThread.mThread.mHandle != nullptr) {
        mQuit = true;
        sem_post(&mSemaphore);
        mThread.mThread._Join();
    }
    if (SemaphoreInitialized(mSemaphore)) {
        sem_destroy(&mSemaphore);
        ClearSemaphoreWord(mSemaphore);
    }
    mThread.mThread._ForceKillThread();
}

// Reconstructed from eboot.elf at 0x264390.
void StreamReaderThread::QuitAsyncPoll() {
    if (mThread.mThread.mHandle != nullptr) {
        mQuit = true;
        sem_post(&mSemaphore);
        mThread.mThread._Join();
    }
}

// Reconstructed from eboot.elf at 0x2643D0.
void StreamReaderThread::StartAsyncPoll() {
    mQuit = false;
    const auto& task = *ThreadMap::GetTaskSettings("stream_reader");
    mThread.Create(
        _ReaderThreadMain,
        this,
        "stream_reader",
        task.mProcessor,
        task.mPriority,
        task.mStackSize,
        task.mAffinityMask);
    mThread.mThread.Start();
}

// Reconstructed from eboot.elf at 0x264470. Readers run one at a time
// outside the lock; the thread sleeps on the semaphore while none are
// queued.
int StreamReaderThread::_ReaderThreadMain(void* context) {
    auto* thread = static_cast<StreamReaderThread*>(context);
    while (!thread->mQuit) {
        if (thread->mReaders.mSize != 0) {
            thread->mCritSec.Enter();
            if (thread->mReaders.mSize == 0) {
                thread->mCritSec.Exit();
                continue;
            }
            StreamReader* reader = thread->mReaders.PopFront();
            reader->mState = StreamReader::kStateReading;
            thread->mCritSec.Exit();
            reader->ReadSegments();
            if (reader->mOnDone) {
                reader->mOnDone();
            }
            reader->mState = StreamReader::kStateDone;
        } else {
            while (sem_wait(&thread->mSemaphore) != 0) {
                static_cast<void>(errno);
            }
        }
    }

    ScopedCritSec tracker(thread->mCritSec);
    while (thread->mReaders.mSize != 0) {
        auto* reader = ReaderList::Owner(thread->mReaders.mNext);
        reader->mResult = kReaderCancelled;
        if (reader->mOnDone) {
            reader->mOnDone();
        }
        reader->mState = StreamReader::kStateIdle;
        thread->mReaders.PopFront();
    }
    return 0;
}

// Reconstructed from eboot.elf at 0x264660.
void StreamReaderThread::AddReader(StreamReader* reader) {
    if (mQuit) {
        return;
    }
    ScopedCritSec tracker(mCritSec);
    reader->mState = StreamReader::kStateQueued;
    mReaders.PushBack(*reader);
    sem_post(&mSemaphore);
}

// Reconstructed from eboot.elf at 0x264700.
void StreamReaderThread::RemoveReader(StreamReader* reader) {
    ScopedCritSec tracker(mCritSec);
    if (reader->mReaderNode.mList == &mReaders) {
        mReaders.Remove(*reader);
        reader->mState = StreamReader::kStateIdle;
    }
}
