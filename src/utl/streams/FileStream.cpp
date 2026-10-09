#include "utl/streams/FileStream.h"

#include <cstring>

#include "os/files/StreamChecksum.h"

namespace {

constexpr int kFileStreamPlatform = 3;

}  // namespace

// Reconstructed from eboot.elf at 0x2443A0. The name is copied with strncpy
// semantics, and the size is queried only after a successful open.
FileStream::FileStream(const char* path, FileMode mode, bool littleEndian)
    : BinStream(littleEndian, kFileStreamPlatform),
      mSize(0),
      mChecksum(nullptr),
      mBytesChecksummed(0) {
    std::strncpy(mFilename, path, sizeof(mFilename));
    mFile = FileOpen(path, mode);
    mFail = FileFail(mFile);
    if (!mFail) {
        mSize = FileSize(mFile);
    }
}

// Reconstructed from eboot.elf at 0x2444A0. Only named streams whose file
// opened successfully close it.
FileStream::~FileStream() {
    if (mFilename[0] != '\0' && !FileFail(mFile)) {
        FileClose(mFile);
    }
    DeleteChecksum();
}

// Inlined into the destructor in this build. The checksum's class delete
// releases it through MemFree.
void FileStream::DeleteChecksum() {
    delete mChecksum;
    mChecksum = nullptr;
    mBytesChecksummed = 0;
}

void FileStream::Flush() {
    FileFlush(mFile);
}

int FileStream::Tell() {
    return FileTell(mFile);
}

EofType FileStream::Eof() {
    return FileEof(mFile) ? IsEof : NotEof;
}

bool FileStream::Fail() {
    return mFail;
}

const char* FileStream::Name() const {
    return mFilename;
}

long FileStream::Size() const {
    return mSize;
}

void FileStream::ReadImpl(void* data, unsigned long size) {
    if (FileRead(mFile, data, size) != static_cast<long>(size)) {
        mFail = true;
        return;
    }
    if (mChecksum != nullptr) {
        mChecksum->Update(static_cast<const unsigned char*>(data), size);
        mBytesChecksummed += size;
    }
}

void FileStream::WriteImpl(const void* data, unsigned long size) {
    if (FileWrite(mFile, data, size) != static_cast<long>(size)) {
        mFail = true;
    }
}

void FileStream::SeekImpl(long offset, SeekType origin) {
    if (FileSeek(mFile, offset, origin) < 0) {
        mFail = true;
    }
}
