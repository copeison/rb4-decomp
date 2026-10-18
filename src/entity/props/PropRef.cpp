// entity/PropRef.o (part): the constructors.
#include "entity/props/PropRef.h"

// Reconstructed from eboot.elf at 0x17F4F0. The id is stored as a 64-bit
// -1, which also fills the padding after it.
PropRef::PropRef() : mObject{0xFFFFFFFFU}, mCom(), mPath() {}

// Reconstructed from eboot.elf at 0x17F550.
PropRef::PropRef(GameObjectId object, Symbol com, const PropPath& path)
    : mObject(object), mCom(com), mPath(path) {}

// Reconstructed from eboot.elf at 0x17F5B0.
PropRef::PropRef(Symbol com, const PropPath& path) : mObject{0xFFFFFFFFU}, mCom(com), mPath(path) {}
