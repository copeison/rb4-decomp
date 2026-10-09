#pragma once

#include <cstddef>
#include <functional>
#include <utility>
#include <semaphore.h>

#include "audio/core/containers/LinkedListSizeTracked.h"
#include "os/threading/CritSec.h"
#include "utl/threading/Thread.h"

namespace FMOD {
class Sound;
}

// One queued read of up to eight PCM ranges from an FMOD sound into 16-bit
// samples. The map's StreamReader has a vtable; this build's reader is a
// plain object whose members sit with StreamReaderThread's at 0x263EC0 and
// 0x263F70. Names not in the reference map unless noted.
class StreamReader {
public:
    // mState values.
    enum State : int {
        kStateIdle = 0,
        kStateQueued = 1,
        kStateReading = 2,
        kStateDone = 3,
    };

    // A range of frames; a negative start is leading silence.
    struct Segment {
        int mStart;
        int mCount;
    };

    static constexpr int kMaxSegments = 8;

    // Inlined into the EASTL growth of FmodBufferedStreamGenerator's reader
    // vector at 0x26E420: every segment starts empty at frame -1.
    StreamReader()
        : mState(kStateIdle),
          mSound(nullptr),
          mStartFrame(0),
          mBuffer(nullptr),
          mBufferBytes(0),
          mNumChannels(2),
          mFramesRead(0),
          mResult(0) {
        for (Segment& segment : mSegments) {
            segment.mStart = -1;
            segment.mCount = 0;
        }
    }
    // Moves the completion callback; the queue link starts unlinked. Inlined
    // at 0x26E420.
    StreamReader(StreamReader&& other)
        : mState(other.mState),
          mOnDone(std::move(other.mOnDone)),
          mSound(other.mSound),
          mStartFrame(other.mStartFrame),
          mBuffer(other.mBuffer),
          mBufferBytes(other.mBufferBytes),
          mNumChannels(other.mNumChannels),
          mFramesRead(other.mFramesRead),
          mResult(other.mResult) {
        for (int index = 0; index < kMaxSegments; ++index) {
            mSegments[index] = other.mSegments[index];
        }
    }

    // Binds the sound, the destination and the completion callback. At
    // 0x263EC0.
    void Setup(
        FMOD::Sound* sound,
        int numChannels,
        short* buffer,
        int bufferBytes,
        const std::function<void()>& onDone);
    // Reads every segment into the buffer, padding short reads with
    // silence, and widens mono to stereo in place. At 0x263F70.
    void ReadSegments();

    Segment mSegments[kMaxSegments];
    State mState;
    std::function<void()> mOnDone;
    FMOD::Sound* mSound;
    int mStartFrame;  // First stream frame of the buffer; set by its owner.
    short* mBuffer;
    int mBufferBytes;
    int mNumChannels;
    int mFramesRead;
    int mResult;  // FMOD_RESULT of the last read.
    LinkedListSizeTracked::Node mReaderNode;
};

static_assert(offsetof(StreamReader, mState) == 64);
static_assert(offsetof(StreamReader, mOnDone) == 80);
static_assert(offsetof(StreamReader, mSound) == 128);
static_assert(offsetof(StreamReader, mBuffer) == 144);
static_assert(offsetof(StreamReader, mNumChannels) == 156);
static_assert(offsetof(StreamReader, mResult) == 164);
static_assert(offsetof(StreamReader, mReaderNode) == 168);

// Background thread that services stream readers
// (audio/StreamReaderThread.o). The map has a global theStreamReaderThread;
// in this build FmodBufferedStreamGeneratorManager owns the thread.
class StreamReaderThread {
public:
    using ReaderList = LinkedListSizeTracked::List<StreamReader, &StreamReader::mReaderNode>;

    StreamReaderThread();   // 0x264140
    ~StreamReaderThread();  // 0x264260

    // Signals the thread to quit and joins it. At 0x264390.
    void QuitAsyncPoll();
    // Starts the "stream_reader" thread. At 0x2643D0.
    void StartAsyncPoll();
    // Queues a reader and wakes the thread; ignored while quitting. At
    // 0x264660.
    void AddReader(StreamReader* reader);
    // Unqueues a reader that has not started. At 0x264700. The map has
    // AsyncRemoveReader(StreamReader*) with a separate wait; this build
    // removes it at once.
    void RemoveReader(StreamReader* reader);

    // The thread body at 0x264470. On quit, every queued reader completes
    // with result 15.
    static int _ReaderThreadMain(void* context);

    // Field names are not in the reference map.
    bool mQuit;
    NamedThread mThread;
    CritSec mCritSec;
    ReaderList mReaders;
    int mUnusedWord;  // Between the reader list and the semaphore; never accessed.
    sem_t mSemaphore;
};

static_assert(offsetof(StreamReaderThread, mThread) == 8);
static_assert(offsetof(StreamReaderThread, mCritSec) == 144);
static_assert(offsetof(StreamReaderThread, mReaders) == 160);
static_assert(offsetof(StreamReaderThread, mSemaphore) == 188);
static_assert(sizeof(StreamReaderThread) == 208);
