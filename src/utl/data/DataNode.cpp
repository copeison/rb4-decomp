#include "utl/data/DataNode.h"

#include <cstddef>

#include <string.h>

#include "entity/resources/Resource.h"
#include "entity/core/GameObject.h"
#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "os/threading/CritSec.h"
#include "utl/containers/List.h"
#include "utl/containers/Map.h"
#include "utl/data/DataArray.h"
#include "utl/text/Str.h"

namespace {

// One thread's DataThread as gDataThread keeps it: a lock taken around
// writes from other threads, with the owning thread and its depth, and the
// link into gDataThreadSlots. Layout from the walk at 0x236B20; names not in
// the reference map.
struct DataThreadSlot {
    CritSec* mCrit;
    unsigned char mPadding[8];  // Not touched here.
    ScePthread mOwner;
    int mOwnerDepth;
    DataThread mThread;
    eastl::ListNodeBase mLink;
};

static_assert(offsetof(DataThreadSlot, mThread) == 0x20);
static_assert(offsetof(DataThreadSlot, mLink) == 0x33E8);

}  // namespace

// The list of every thread's slot and its lock, at 0x19E7A88: part of the
// map's gDataThread (TLSValue<DataThread>, utl/DataUtl.o), modelled on its
// own until TLSValue is reconstructed. Name not in the reference map.
struct DataThreadSlots {
    CritSec* mCrit;
    eastl::ListNodeBase mSlots;
};

extern DataThreadSlots gDataThreadSlots;

// The variable indices by name, at 0x19E7830, and their lock, at 0x19E7A18.
// The map reaches them through GetDataVariableIndicesMap() and
// GetDataVariableIndicesMapCritSec(), which this binary does not keep. The
// map's name of the index map is not known.
static eastl::map<Symbol, unsigned long> gVarIndices;
CritSec gVarIndexCrit;

namespace {

// The id of no object, at 0x19E7828, which DataNode.o's initializer sets.
// Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFu};

// Reconstructed from eboot.elf at 0x236B20.
// Writes the variable's value into every thread's DataThread. Name not in
// the reference map.
void SetVarInAllThreads(unsigned long index, DataNode value) {
    CritSec* const listCrit = gDataThreadSlots.mCrit;
    if (listCrit != nullptr) {
        listCrit->Enter();
    }
    eastl::ListNodeBase* const anchor = &gDataThreadSlots.mSlots;
    for (eastl::ListNodeBase* link = anchor->mpNext; link != anchor; link = link->mpNext) {
        DataThreadSlot* const slot = reinterpret_cast<DataThreadSlot*>(
            reinterpret_cast<char*>(link) - offsetof(DataThreadSlot, mLink));
        if (slot->mCrit != nullptr) {
            slot->mCrit->Enter();
            if (slot->mOwnerDepth == 0) {
                slot->mOwner = scePthreadSelf();
            }
            ++slot->mOwnerDepth;
        }
        slot->mThread.mVars[index] = value;
        if (slot->mCrit != nullptr) {
            if (slot->mOwnerDepth-- == 1) {
                slot->mOwner = nullptr;
            }
            slot->mCrit->Exit();
        }
    }
    if (gDataThreadSlots.mCrit != nullptr) {
        gDataThreadSlots.mCrit->Exit();
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x236750.
unsigned long DataVarIndex(Symbol name) {
    ScopedCritSec lock(gVarIndexCrit);
    const auto it = gVarIndices.find(name);
    if (it != gVarIndices.end()) {
        return it->second;
    }
    const unsigned long index = gVarIndices.size();
    static long sHeap = MemFindHeap("system");
    MemPushHeap(sHeap);
    gVarIndices[name] = index;
    MemPopHeap();
    return index;
}

// Reconstructed from eboot.elf at 0x236910.
unsigned long DataVarIndex(Symbol name, DataNode value) {
    CritSec* const listCrit = gDataThreadSlots.mCrit;
    if (listCrit != nullptr) {
        listCrit->Enter();
    }
    gVarIndexCrit.Enter();
    unsigned long index;
    const auto it = gVarIndices.find(name);
    if (it != gVarIndices.end()) {
        index = it->second;
        gVarIndexCrit.Exit();
    } else {
        index = DataVarIndex(name);
        gVarIndexCrit.Exit();
        SetVarInAllThreads(index, value);
    }
    if (gDataThreadSlots.mCrit != nullptr) {
        gDataThreadSlots.mCrit->Exit();
    }
    return index;
}

// Reconstructed from eboot.elf at 0x236CC0.
DataNode& DataVariable(unsigned long index) {
    return gDataThread.mVars[index];
}

// Reconstructed from eboot.elf at 0x236D50.
// The map's DataSetGlobal also queues the change for SetGlobalJob, which
// this binary does not keep.
void DataSetGlobal(Symbol name, const DataNode& value) {
    SetVarInAllThreads(DataVarIndex(name), value);
}

// Reconstructed from eboot.elf at 0x236F40.
bool DataNode::CompatibleType(DataType type) const {
    if (mType == type) {
        return true;
    }
    switch (mType) {
    case kDataInt:
        return type == kDataFloat;
    case kDataSymbol:
        return type == kDataString || type == kDataObject;
    case kDataString:
        return type == kDataObject;
    case kDataResourcePath:
        return type == kDataString || type == kDataSymbol;
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0x2374E0.
Symbol DataNode::LiteralSym(const DataArray* source) const {
    static_cast<void>(source);
    switch (mType) {
    case kDataResourcePath: {
        ResourcePath path;
        path = mValue.symbol;
        return path.mPath;
    }
    case kDataString:
        return Symbol(reinterpret_cast<const char*>(mValue.array->mNodes));
    case kDataSymbol:
        return *reinterpret_cast<const Symbol*>(&mValue.symbol);
    default:
        return Symbol();
    }
}

// Reconstructed from eboot.elf at 0x237570.
const char* DataNode::Str(const DataArray* source) const {
    static_cast<void>(source);
    if (mType == kDataCommand) {
        // The result is kept in the thread so that its text outlives the
        // call.
        DataNode& result = gDataThread.mStrResult;
        result = DataExecute(mValue.array);
        if (result.mType == kDataResourcePath || result.mType == kDataSymbol) {
            return result.mValue.symbol;
        }
        return reinterpret_cast<const char*>(result.mValue.array->mNodes);
    }
    const DataNode node = Evaluate();
    if (node.mType == kDataResourcePath || node.mType == kDataSymbol) {
        return node.mValue.symbol;
    }
    return reinterpret_cast<const char*>(node.mValue.array->mNodes);
}

// Reconstructed from eboot.elf at 0x237E30.
DataArray* DataNode::Command(const DataArray* source) const {
    static_cast<void>(source);
    return mValue.array;
}

// Reconstructed from eboot.elf at 0x237E40.
DataNode* DataNode::Var(const DataArray* source) const {
    static_cast<void>(source);
    return &gDataThread.mVars[mValue.var];
}

// Reconstructed from eboot.elf at 0x237ED0.
DataNode::DataNode(const char* str) {
    DataArray* const array = new DataArray(str, strlen(str) + 1);
    mValue.array = array;
    array->AddRef();
    mType = kDataString;
}

// Reconstructed from eboot.elf at 0x237F30.
DataNode::DataNode(const String& str) {
    const char* const text = str.c_str();
    DataArray* const array = new DataArray(text, strlen(text) + 1);
    mValue.array = array;
    array->AddRef();
    mType = kDataString;
}

// Reconstructed from eboot.elf at 0x2383E0.
DataNode::DataNode(const void* glob, unsigned long size) {
    DataArray* const array = new DataArray(glob, size);
    mValue.array = array;
    array->AddRef();
    mType = kDataGlob;
}

// Reconstructed from eboot.elf at 0x238470.
DataNode::DataNode(const DataArrayPtr& array) {
    mValue.array = array.mData;
    array.mData->AddRef();
    mType = kDataArray;
}

// Reconstructed from eboot.elf at 0x238490.
DataNode::DataNode(DataArray* array, DataType type) {
    mValue.array = array;
    array->AddRef();
    mType = type;
}

// Reconstructed from eboot.elf at 0x2387C0.
bool DataNode::NotNull() const {
    const DataNode node = Evaluate();
    switch (node.mType) {
    case kDataSymbol:
    case kDataResourcePath:
        return node.mValue.symbol[0] != '\0';
    case kDataUnhandled:
        return false;
    case kDataString:
        // The size counts the terminator.
        return node.mValue.array->mSize != 0 && -node.mValue.array->mSize != 1;
    case kDataGlob:
        return node.mValue.array->mSize != 0;
    case kDataGameObjectId:
        return node.mValue.var != gNullObjectId.mId;
    default:
        return node.mValue.object != nullptr;
    }
}

// Reconstructed from eboot.elf at 0x239290.
// Waveforms are resources.
void DataNode::AddRefWaveform() const {
    static_cast<Resource*>(mValue.object)->AddRef();
}

// Reconstructed from eboot.elf at 0x2392A0.
void DataNode::ReleaseWaveform() const {
    static_cast<Resource*>(mValue.object)->ReleaseRef();
}
