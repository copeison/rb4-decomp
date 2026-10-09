#include "entity/resources/Resource.h"

#include <string.h>

#include "os/files/File.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"

// The map has ResourcePath's methods in entity/Resource.o. This build places
// them in their own object after ResourceMetaData.o (0x1AF950-0x1AFFAB); its
// name is not in the map, so the file is named after the class.

namespace {

// Resolves the path, with its sub-object part cut off, against the file root
// and resolves the sub-object part back on. The binary repeats this body in
// operator= and SetFilepathAndSubObject. Name not in the reference map.
Symbol ResolvePath(char* path) {
    char* const subObject = strstr(path, "::");
    if (subObject != nullptr) {
        *subObject = '\0';
    }
    FileNormalizePath(path);
    char relative[512];
    const char* const resolved = FileRelativePath(FileRoot(), path, relative);
    if (subObject != nullptr) {
        *subObject = ':';
    }
    return Symbol(resolved);
}

}  // namespace

// Reconstructed from eboot.elf at 0x1AF950.
ResourcePath& ResourcePath::operator=(const char* path) {
    char buffer[512];
    strcpy(buffer, path);
    mPath = ResolvePath(buffer);
    return *this;
}

// Reconstructed from eboot.elf at 0x1AFAA0.
void ResourcePath::SetFilepathAndSubObject(const char* file, Symbol subClass, const char* subPath) {
    char buffer[512];
    if (subPath != nullptr) {
        FormatString format("%s::%s::%s");
        format << file << subClass << subPath;
        strcpy(buffer, format.Str());
    } else {
        strcpy(buffer, file);
    }
    mPath = ResolvePath(buffer);
}

// Reconstructed from eboot.elf at 0x1AFC70.
const char* ResourcePath::GetFilepathAndSubObject(FixedString& file, Symbol* subClass) const {
    if (subClass != nullptr) {
        *subClass = Symbol();
    }
    file = mPath.Str();
    const char* const path = mPath.Str();
    const char* const first = strstr(path, "::");
    if (first == nullptr) {
        return nullptr;
    }
    const char* const second = strstr(first + 2, "::");
    if (second == nullptr) {
        return nullptr;
    }
    file[first - path] = '\0';
    file[second - path] = '\0';
    if (subClass != nullptr) {
        *subClass = Symbol(&file[first + 2 - path]);
    }
    return second + 2;
}

// Reconstructed from eboot.elf at 0x1AFDC0.
bool ResourcePath::HasSubObject() const {
    return strstr(mPath.Str(), "::") != nullptr;
}

// Reconstructed from eboot.elf at 0x1AFDE0.
const char* ResourcePath::GetFullPath() const {
    char buffer[512];
    const char* const path = FileMakePath(FileRoot(), mPath.Str(), buffer);
    FormatString format("%s");
    format << path;
    return format.Str();
}

// Reconstructed from eboot.elf at 0x1AFF10.
void ResourcePath::FixSubObjectPath(Symbol subClass) {
    const char* const path = mPath.Str();
    const char* const subObject = strstr(path, "::");
    if (subObject == nullptr) {
        return;
    }
    char buffer[512];
    strcpy(buffer, path);
    const long fileLength = subObject - path;
    buffer[fileLength] = '\0';
    SetFilepathAndSubObject(buffer, subClass, &buffer[fileLength + 2]);
}
