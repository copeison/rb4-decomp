#pragma once

// Path utilities (utl/FileUtl.o). Only the functions the reconstructed code
// calls are declared; FileUtl.cpp reconstructs the path splitting, and
// FileMakePath, FileRelativePath and FileMkDirRecur are not reconstructed.

// Whether the path starts with a slash or a drive ("c:/").
bool FileIsAbsolute(const char* path);  // 0x245820
// Lower-cases the path in place and turns backslashes into slashes.
char* FileNormalizePath(char* path);  // 0x244860
// Joins the path to the root, resolving "." and "..", into `buffer`.
const char* FileMakePath(const char* root, const char* path, char* buffer);  // 0x244960
// The path relative to the root, into `buffer`, or the path itself when it
// is already relative.
const char* FileRelativePath(const char* root, const char* path, char* buffer);  // 0x244F60
// The directory part of a path in `buffer`; "." when the path has none.
char* FileGetPath(const char* path, char* buffer);  // 0x245300
// The extension after the last dot of the file name; with `withDot` the dot
// is included. A name without one gives the empty end of the string.
const char* FileGetExt(const char* path, bool withDot);  // 0x245380
// The file name after the last separator.
const char* FileGetName(const char* path);  // 0x2453D0
// The file name without its directory and extension, in `buffer`.
char* FileGetBase(const char* path, char* buffer);  // 0x245420
// Creates the directory and its parents under the root.
bool FileMkDirRecur(const char* path, const char* root);  // 0x245530
