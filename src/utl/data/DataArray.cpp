#include "utl/data/DataArray.h"

#include <new>

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "entity/core/GameObject.h"
#include "os/memory/MemMgr.h"
#include "utl/data/DataFunc.h"
#include "utl/data/DataUtl.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Str.h"

// Parses the text into an array, from utl/DataFile.o.
DataArrayPtr DataReadString(const char* str);  // 0x2203A0

namespace {

// The file name given to arrays as they are read, at 0x19E7708. Name not in
// the reference map.
Symbol gFile;

// The symbol a node holds, without interning. Name not in the reference
// map.
const Symbol& RawSym(const DataNode& node) {
    return *reinterpret_cast<const Symbol*>(&node.mValue.symbol);
}

// Allocates storage for `count` nodes; the caller constructs them. Name not
// in the reference map.
DataNode* AllocNodes(unsigned long count) {
    if (count == 0) {
        return nullptr;
    }
    return static_cast<DataNode*>(MemOrPoolAlloc(count * sizeof(DataNode), "DataNodes", 0));
}

// Destroys `count` nodes and frees their storage. Name not in the reference
// map.
void FreeNodes(DataNode* nodes, unsigned long count) {
    for (unsigned long i = 0; i < count; ++i) {
        nodes[i].~DataNode();
    }
    if (nodes != nullptr) {
        MemOrPoolFree(count * sizeof(DataNode), nodes, nullptr);
    }
}

// Assigns `source` to a newly allocated node, as the binary's inlined
// copies do. Name not in the reference map.
void InitNode(DataNode& node, const DataNode& source) {
    new (&node) DataNode();
    node = source;
}

// Reconstructed from eboot.elf at 0x21E600.
// SortNodes's qsort order: numbers by value, symbols and strings without
// case, arrays by their first node; other types compare equal. Name not in
// the reference map.
int NodeCompare(const void* left, const void* right) {
    const DataNode* const a = static_cast<const DataNode*>(left);
    const DataNode* const b = static_cast<const DataNode*>(right);
    switch (a->mType) {
    case kDataInt:
    case kDataFloat: {
        const float x =
            a->mType == kDataInt ? static_cast<float>(a->mValue.integer) : a->mValue.real;
        const float y =
            b->mType == kDataInt ? static_cast<float>(b->mValue.integer) : b->mValue.real;
        if (x < y) {
            return -1;
        }
        return x != y ? 1 : 0;
    }
    case kDataArray: {
        const DataArrayPtr x = a->Array(nullptr);
        const DataArrayPtr y = b->Array(nullptr);
        return NodeCompare(x->mNodes, y->mNodes);
    }
    case kDataSymbol:
    case kDataResourcePath:
    case kDataString: {
        const char* const x = a->mType == kDataString
            ? reinterpret_cast<const char*>(a->mValue.array->mNodes)
            : a->mValue.symbol;
        const char* const y = b->mType == kDataResourcePath || b->mType == kDataSymbol
            ? b->mValue.symbol
            : reinterpret_cast<const char*>(b->mValue.array->mNodes);
        return strcasecmp(x, y);
    }
    default:
        return 0;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x21BCC0.
void DataArray::Insert(unsigned long index, const DataNode& node) {
    DataNode* const old = mNodes;
    const unsigned long size = mSize + 1;
    mNodes = AllocNodes(size);
    unsigned long i = 0;
    for (; i < index; ++i) {
        InitNode(mNodes[i], old[i]);
    }
    for (; i < index + 1; ++i) {
        InitNode(mNodes[i], node);
    }
    for (; i < size; ++i) {
        InitNode(mNodes[i], old[i - 1]);
    }
    FreeNodes(old, mSize);
    mSize = size;
}

// Reconstructed from eboot.elf at 0x21BFA0.
void DataArray::InsertNodes(unsigned long index, const DataArray* nodes) {
    if (nodes == nullptr || nodes->mSize == 0) {
        return;
    }
    const unsigned long count = nodes->mSize;
    DataNode* const old = mNodes;
    const unsigned long size = mSize + count;
    mNodes = AllocNodes(size);
    unsigned long i = 0;
    for (; i < index; ++i) {
        InitNode(mNodes[i], old[i]);
    }
    for (; i < index + count; ++i) {
        InitNode(mNodes[i], nodes->mNodes[i - index]);
    }
    for (; i < size; ++i) {
        InitNode(mNodes[i], old[i - count]);
    }
    FreeNodes(old, mSize);
    mSize = size;
}

// Reconstructed from eboot.elf at 0x21C2E0.
void DataArray::Resize(unsigned long size) {
    DataNode* const old = mNodes;
    mNodes = AllocNodes(size);
    unsigned long kept = mSize;
    if (kept > size) {
        kept = size;
    }
    unsigned long i = 0;
    for (; i < kept; ++i) {
        InitNode(mNodes[i], old[i]);
    }
    for (; i < size; ++i) {
        new (&mNodes[i]) DataNode();
    }
    FreeNodes(old, mSize);
    mSize = size;
}

// Reconstructed from eboot.elf at 0x21C4A0.
void DataArray::Remove(unsigned long index, unsigned long count) {
    if (count == 0) {
        return;
    }
    DataNode* const old = mNodes;
    const unsigned long size = mSize - count;
    mNodes = AllocNodes(size);
    unsigned long i = 0;
    for (; i < index; ++i) {
        InitNode(mNodes[i], old[i]);
    }
    for (; i < size; ++i) {
        InitNode(mNodes[i], old[i + count]);
    }
    FreeNodes(old, mSize);
    mSize = size;
}

// Reconstructed from eboot.elf at 0x21C6E0.
void DataArray::RemoveNode(const DataNode& node) {
    for (long i = mSize - 1; i >= 0; --i) {
        if (mNodes[i].mValue.object == node.mValue.object) {
            Remove(i, 1);
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x21C720.
bool DataArray::Contains(const DataNode& node) const {
    for (long i = mSize - 1; i >= 0; --i) {
        if (mNodes[i].mValue.object == node.mValue.object) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x21C970.
DataArray* DataArray::FindArray(Symbol key, bool fail) const {
    static_cast<void>(fail);
    for (const DataNode* node = mNodes; node < mNodes + mSize; ++node) {
        if (node->mType == kDataArray) {
            DataArray* const array = node->mValue.array;
            if (array->mSize != 0 && RawSym(array->mNodes[0]) == key) {
                return array;
            }
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x21C9C0.
DataArray* DataArray::FindArray(Symbol key1, Symbol key2) const {
    return FindArray(key1, true)->FindArray(key2, true);
}

// Reconstructed from eboot.elf at 0x21CA30.
DataArray* DataArray::FindArray(Symbol key1, Symbol key2, Symbol key3) const {
    return FindArray(key1, key2)->FindArray(key3, true);
}

// Reconstructed from eboot.elf at 0x21CE20.
bool DataArray::FindData(Symbol key, String& value, bool fail) const {
    const DataArray* const array = FindArray(key, fail);
    if (array == nullptr) {
        return false;
    }
    value = array->Str(1);
    return true;
}

// Reconstructed from eboot.elf at 0x21CF20.
bool DataArray::FindData(Symbol key, const char*& value, bool fail) const {
    const DataArray* const array = FindArray(key, fail);
    if (array == nullptr) {
        return false;
    }
    value = array->Str(1);
    return true;
}

// Reconstructed from eboot.elf at 0x21CFC0.
bool DataArray::FindData(Symbol key, Symbol& value, bool fail) const {
    const DataArray* const array = FindArray(key, fail);
    if (array == nullptr) {
        return false;
    }
    value = array->Sym(1);
    return true;
}

// Reconstructed from eboot.elf at 0x21D060.
bool DataArray::FindData(Symbol key, int& value, bool fail) const {
    const DataArray* const array = FindArray(key, fail);
    if (array == nullptr) {
        return false;
    }
    value = array->Int(1);
    return true;
}

// Reconstructed from eboot.elf at 0x21D100.
bool DataArray::FindData(Symbol key, unsigned long& value, bool fail) const {
    const DataArray* const array = FindArray(key, fail);
    if (array == nullptr) {
        return false;
    }
    value = array->Int(1);
    return true;
}

// Reconstructed from eboot.elf at 0x21D1B0.
bool DataArray::FindData(Symbol key, float& value, bool fail) const {
    const DataArray* const array = FindArray(key, fail);
    if (array == nullptr) {
        return false;
    }
    value = array->Float(1);
    return true;
}

// Reconstructed from eboot.elf at 0x21D260.
bool DataArray::FindData(Symbol key, bool& value, bool fail) const {
    const DataArray* const array = FindArray(key, fail);
    if (array == nullptr) {
        return false;
    }
    value = array->Node(1).NotNull();
    return true;
}

// Reconstructed from eboot.elf at 0x21DAC0.
DataArray* DataArray::Clone(unsigned int flags, int extra) const {
    short count = static_cast<short>(mSize + extra);
    if (count > mSize) {
        count = mSize;
    }
    DataArray* const clone = new DataArray(mSize + extra);
    clone->mFile = mFile;
    clone->mLine = mLine;
    for (int i = 0; i < count; ++i) {
        DataNode& node = clone->mNodes[i];
        const DataNode& source = mNodes[i];
        if ((flags & 4) != 0 && source.mType == kDataVar) {
            node = DataVariable(source.mValue.var);
        } else if ((flags & 4) != 0 && source.mType == kDataProperty) {
            node = DataGetProperty(source.mValue.array);
        } else if ((flags & 4) != 0 && source.mType == kDataCommand) {
            node = DataExecute(source.mValue.array);
        } else {
            node = source;
        }

        if ((flags & 1) != 0 && node.mType == kDataArray) {
            node = DataNode(node.mValue.array->Clone(flags, 0), kDataArray);
        } else if ((flags & 2) != 0 && node.mType == kDataCommand) {
            node = DataNode(node.Command(this)->Clone(flags, 0), kDataCommand);
        } else if ((flags & 8) != 0 && (node.mType & kDataArray) != 0) {
            DataArray* const array = node.mValue.array;
            if (array->mSize >= 0) {
                node = DataNode(array->Clone(flags, 0), node.mType);
            } else if (node.mType == kDataGlob) {
                // The binary never frees this temporary copy.
                const unsigned long size = -array->mSize;
                char* const bytes = new char[size];
                memcpy(bytes, array->mNodes, size);
                node = DataNode(new DataArray(bytes, size), node.mType);
            } else if (node.mType == kDataString) {
                node = DataNode(node.Str(this));
            } else {
                const DataNode& same = node;
                node = same;
            }
        }
    }
    return clone;
}

// Reconstructed from eboot.elf at 0x21E410.
DataArray::DataArray(unsigned long size) : mFile(), mSize(size), mLine(0) {
    __atomic_store_n(&mRefs, 0, __ATOMIC_SEQ_CST);
    mNodes = AllocNodes(size);
    for (unsigned long i = 0; i < size; ++i) {
        new (&mNodes[i]) DataNode();
    }
}

// Reconstructed from eboot.elf at 0x21E490.
DataArray::DataArray(const void* glob, unsigned long size) : mFile(), mSize(-size), mLine(0) {
    __atomic_store_n(&mRefs, 0, __ATOMIC_SEQ_CST);
    mNodes = size != 0 ? static_cast<DataNode*>(MemOrPoolAlloc(size, "DataNodes", 0)) : nullptr;
    memcpy(mNodes, glob, size);
}

// Reconstructed from eboot.elf at 0x21E500.
DataArray::~DataArray() {
    if (mSize < 0) {
        if (mNodes != nullptr) {
            MemOrPoolFree(-mSize, mNodes, nullptr);
        }
        return;
    }
    FreeNodes(mNodes, mSize);
}

// Reconstructed from eboot.elf at 0x21E5B0.
void DataArray::SetFileLine(Symbol file, unsigned long line) {
    mFile = file;
    mLine = line;
}

// Reconstructed from eboot.elf at 0x21E5C0.
void DataArray::SortNodes(int start, int count) {
    if (mSize > 0) {
        qsort(mNodes + start, count != -1 ? count : mSize - start, sizeof(DataNode), NodeCompare);
    }
}

// Reconstructed from eboot.elf at 0x21E720.
void DataArray::UniqueNodes(DataArray* counts) {
    if (mSize <= 0) {
        return;
    }
    if (counts != nullptr) {
        counts->Resize(0);
    }
    DataNode last = mNodes[0];
    int start = 1;
    int i = 1;
    for (; i < mSize; ++i) {
        if (NodeCompare(&mNodes[i], &last) != 0) {
            const int duplicates = i - start;
            Remove(start, duplicates);
            if (counts != nullptr) {
                counts->Insert(counts->mSize, DataNode(duplicates + 1));
            }
            last = mNodes[start];
            i = start;
            ++start;
        }
    }
    Remove(start, i - start);
    if (counts != nullptr) {
        counts->Insert(counts->mSize, DataNode(i - start + 1));
    }
}

// Reconstructed from eboot.elf at 0x21EAB0.
DataNode DataExecute(DataArray* command) {
    const DataNode func = command->Evaluate(0);
    switch (func.mType) {
    case kDataFunc:
        return func.mValue.func(command);
    case kDataObject:
        if (func.mValue.object != nullptr) {
            return static_cast<MsgSink*>(func.mValue.object)->Handle(command, true);
        }
        return DataNode();
    case kDataArray: {
        // Components receive messages through MsgSink's Handle slot.
        Component* const com = GetDataCom(command);
        if (com != nullptr) {
            return reinterpret_cast<MsgSink*>(com)->Handle(command, true);
        }
        break;
    }
    case kDataSymbol:
        break;
    default:
        return DataNode();
    }

    // A symbol names a component of the thread's object, by its class or
    // base class, or a registered function.
    const Symbol& name = RawSym(func);
    const GameObject* const object = gDataThread.mThisObject;
    if (object != nullptr && object->mComs.size() != 0) {
        const GameObject::ComIndex* index = object->mComs.begin();
        const GameObject::ComIndex* const end = object->mComs.end();
        for (; index < end; ++index) {
            if (name == Symbol() ? index->mId == Symbol()
                                 : index->mId == name || index->mBaseId == name) {
                break;
            }
        }
        if (index < end && index->mCom != nullptr) {
            return reinterpret_cast<MsgSink*>(index->mCom)->Handle(command, true);
        }
    }
    const auto it = gDataFuncs.find(name);
    if (it == gDataFuncs.end()) {
        return DataNode();
    }
    command->Node(0) = DataNode(it->second);
    return it->second(command);
}

// Reconstructed from eboot.elf at 0x21EDA0.
DataNode DataExecuteBlock(DataArray* array, unsigned long firstNode) {
    const unsigned long size = static_cast<unsigned int>(array->Size());
    if (size <= firstNode) {
        return DataNode(0);
    }
    DataThread& thread = gDataThread;
    const DataThread::VarStackEntry* const top = thread.mVarStackTop;
    for (unsigned long i = firstNode; i < size - 1; ++i) {
        DataExecute(array->Node(i).Command(array));
    }
    DataNode result = array->Evaluate(size - 1);
    while (thread.mVarStackTop > top) {
        thread.PopVar();
    }
    return result;
}

// Reconstructed from eboot.elf at 0x21EFD0.
void DataThread::PopVar() {
    VarStackEntry* const top = mVarStackTop;
    *top->mVar = top->mValue;
    top->mValue = DataNode(0);
    mVarStackTop = top - 1;
}

// Reconstructed from eboot.elf at 0x21F0E0.
DataNode DataExecuteString(const char* str) {
    const DataArrayPtr command = DataReadString(str);
    return DataExecute(command);
}

// Reconstructed from eboot.elf at 0x21FC60.
void DataArray::SetFile(Symbol file) {
    gFile = file;
}

// Reconstructed from eboot.elf at 0x21FC70.
unsigned long GetDataArrayMemUsage(DataArray* array) {
    if (array == nullptr) {
        return 0;
    }
    const unsigned long count = static_cast<unsigned int>(array->mSize);
    unsigned long usage = count * sizeof(DataNode) + sizeof(DataArray);
    for (unsigned long i = 0; i < count; ++i) {
        const DataNode& node = array->mNodes[i];
        switch (node.mType) {
        case kDataArray:
        case kDataCommand:
        case kDataProperty:
            usage += GetDataArrayMemUsage(node.mValue.array);
            break;
        case kDataString:
            usage += strlen(reinterpret_cast<const char*>(node.mValue.array->mNodes)) +
                sizeof(DataArray);
            break;
        case kDataGlob: {
            int size = 0;
            node.Glob(&size, nullptr);
            usage += size + sizeof(DataArray);
            break;
        }
        default:
            break;
        }
    }
    return usage;
}

// Reconstructed from eboot.elf at 0x21FD60.
const void* DataNode::Glob(int* size, const DataArray* source) const {
    static_cast<void>(source);
    const DataNode node = Evaluate();
    const DataArray* const array = node.mValue.array;
    if (size != nullptr) {
        *size = -array->mSize;
    }
    return array->mNodes;
}
