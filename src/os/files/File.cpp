#include "os/files/File.h"

unsigned char gFileArchiveMode = 0;
bool gResourcePrecacheMode = false;

// Reconstructed from eboot.elf at 0x378940.
void* FileOpen(const char* path, FileMode mode) {
    return File::NewFile(path, mode);
}

// Reconstructed from eboot.elf at 0x378950. A missing file reports failure.
bool FileFail(void* file) {
    if (file == nullptr) {
        return true;
    }
    return static_cast<File*>(file)->Fail();
}

// Reconstructed from eboot.elf at 0x378960.
void FileClose(void* file) {
    delete static_cast<File*>(file);
}

// Reconstructed from eboot.elf at 0x378A10.
long FileRead(void* file, void* data, unsigned long size) {
    return static_cast<File*>(file)->Read(data, size);
}

// Reconstructed from eboot.elf at 0x378A40.
long FileWrite(void* file, const void* data, unsigned long size) {
    return static_cast<File*>(file)->Write(data, size);
}

// Reconstructed from eboot.elf at 0x378A50.
long FileSeek(void* file, long offset, SeekType origin) {
    return static_cast<File*>(file)->Seek(offset, origin);
}

// Reconstructed from eboot.elf at 0x378A60.
int FileTell(void* file) {
    return static_cast<File*>(file)->Tell();
}

// Reconstructed from eboot.elf at 0x378A70.
void FileFlush(void* file) {
    static_cast<File*>(file)->Flush();
}

// Reconstructed from eboot.elf at 0x378A80.
bool FileEof(void* file) {
    return static_cast<File*>(file)->Eof();
}

// Reconstructed from eboot.elf at 0x378A90.
long FileSize(void* file) {
    return static_cast<File*>(file)->Size();
}
