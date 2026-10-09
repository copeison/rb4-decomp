#pragma once

#include <cstddef>

#include "os/memory/PoolAlloc.h"
#include "utl/data/DataNode.h"
#include "utl/text/Symbol.h"

// A reference-counted array of DataNodes, parsed from a script file. A
// string node keeps its characters in an array with a negative size.
class DataArray {
public:
    POOL_OVERLOAD(DataArray)

    // An array of unhandled nodes.
    explicit DataArray(unsigned long size);  // 0x21E410
    // A glob array holding a copy of the bytes.
    DataArray(const void* glob, unsigned long size);  // 0x21E490
    ~DataArray();  // 0x21E500

    int Size() const {
        return mSize;
    }
    DataNode& Node(unsigned long index) {
        return mNodes[index];
    }
    const DataNode& Node(unsigned long index) const {
        return mNodes[index];
    }
    // The node at the index, with variables, commands and properties
    // evaluated. Inline in the map's build; the binary's copy is at
    // 0xC7D30.
    DataNode Evaluate(unsigned long index) const {
        return Node(index).Evaluate();
    }
    int Int(unsigned long index) const {
        return Node(index).Int(this);
    }
    float Float(unsigned long index) const {
        return Node(index).Float(this);
    }
    Symbol Sym(unsigned long index) const {
        return Node(index).Sym(this);
    }
    const char* Str(unsigned long index) const {
        return Node(index).Str(this);
    }

    // Resizes the array, keeping the leading nodes; new nodes are
    // unhandled.
    void Resize(unsigned long size);  // 0x21C2E0
    // Inserts the node before the index.
    void Insert(unsigned long index, const DataNode& node);  // 0x21BCC0
    // Inserts the array's nodes before the index.
    void InsertNodes(unsigned long index, const DataArray* nodes);  // 0x21BFA0
    // Removes `count` nodes from the index on.
    void Remove(unsigned long index, unsigned long count);  // 0x21C4A0
    // Removes the last node whose value equals the node's.
    void RemoveNode(const DataNode& node);  // 0x21C6E0
    // Whether a node's value equals the node's.
    bool Contains(const DataNode& node) const;  // 0x21C720
    // Sorts `count` nodes from `start` on, or the rest of the array when
    // `count` is -1: numbers by value, symbols and strings without case,
    // arrays by their first node.
    void SortNodes(int start, int count);  // 0x21E5C0
    // Removes adjacent duplicates of a sorted array, appending each run's
    // length to `counts` when it is given.
    void UniqueNodes(DataArray* counts);  // 0x21E720
    // A copy with `extra` more nodes. Flag 1 clones child arrays, 2
    // command arrays, 4 evaluates variables, properties and commands, and
    // 8 copies every other child, strings and globs included.
    DataArray* Clone(unsigned int flags, int extra) const;  // 0x21DAC0
    void SetFileLine(Symbol file, unsigned long line);  // 0x21E5B0
    // Sets the file name that arrays read next are given.
    static void SetFile(Symbol file);  // 0x21FC60

    // The child array whose first node is the key, or null. `fail` asks for
    // a missing key to be reported; the release build ignores it.
    DataArray* FindArray(Symbol key, bool fail) const;  // 0x21C970
    // Follows the keys through nested child arrays, which must exist.
    DataArray* FindArray(Symbol key1, Symbol key2) const;  // 0x21C9C0
    DataArray* FindArray(Symbol key1, Symbol key2, Symbol key3) const;  // 0x21CA30
    // Reads the value after the key into the destination, leaving it
    // unchanged when the key is missing. Returns whether it was found.
    bool FindData(Symbol key, String& value, bool fail) const;         // 0x21CE20
    bool FindData(Symbol key, const char*& value, bool fail) const;    // 0x21CF20
    bool FindData(Symbol key, Symbol& value, bool fail) const;         // 0x21CFC0
    bool FindData(Symbol key, int& value, bool fail) const;            // 0x21D060
    bool FindData(Symbol key, unsigned long& value, bool fail) const;  // 0x21D100
    bool FindData(Symbol key, float& value, bool fail) const;          // 0x21D1B0
    bool FindData(Symbol key, bool& value, bool fail) const;           // 0x21D260

    void AddRef() {
        __atomic_add_fetch(&mRefs, 1, __ATOMIC_SEQ_CST);
    }
    // The binary also has an out-of-line copy at 0x21FF70.
    void Release() {
        if (__atomic_fetch_sub(&mRefs, 1, __ATOMIC_SEQ_CST) == 1) {
            delete this;
        }
    }

    // Field names follow the Milo engine.
    DataNode* mNodes;
    Symbol mFile;
    int mRefs;
    short mSize;
    short mLine;
};

static_assert(sizeof(DataArray) == 24);

// A reference to a DataArray. Header-only apart from the destructor, which
// the map places out of line in os/System.o; callers inline it.
class DataArrayPtr {
public:
    DataArrayPtr() : mData(nullptr) {}
    // Holds a reference to the array. Name not in the reference map.
    explicit DataArrayPtr(DataArray* data) : mData(data) {
        if (mData != nullptr) {
            mData->AddRef();
        }
    }
    DataArrayPtr(const DataArrayPtr&) = delete;
    DataArrayPtr& operator=(const DataArrayPtr&) = delete;
    ~DataArrayPtr() {
        if (mData != nullptr) {
            mData->Release();
        }
    }

    DataArray* operator->() const {
        return mData;
    }
    operator DataArray*() const {
        return mData;
    }

    DataArray* mData;  // Name not in the reference map.
};

static_assert(sizeof(DataArrayPtr) == 8);

inline DataNode::DataNode(const DataNode& other) : mValue(other.mValue), mType(other.mType) {
    if ((mType & kDataArray) != 0) {
        mValue.array->AddRef();
    } else if ((mType & ~1) == kDataWaveformFloat) {
        AddRefWaveform();
    }
}

inline DataNode::~DataNode() {
    if ((mType & kDataArray) != 0) {
        mValue.array->Release();
    }
    if ((mType & ~1) == kDataWaveformFloat) {
        ReleaseWaveform();
    }
}

inline DataNode& DataNode::operator=(const DataNode& other) {
    if ((other.mType & kDataArray) != 0) {
        other.mValue.array->AddRef();
    } else if ((other.mType & ~1) == kDataWaveformFloat) {
        other.AddRefWaveform();
    }
    if ((mType & kDataArray) != 0) {
        mValue.array->Release();
    }
    if ((mType & ~1) == kDataWaveformFloat) {
        ReleaseWaveform();
    }
    mValue = other.mValue;
    mType = other.mType;
    return *this;
}

// Runs the command array: a function or object node first is called, a
// symbol names a component of the thread's object or a registered script
// function. An unknown command gives an unhandled node.
DataNode DataExecute(DataArray* command);  // 0x21EAB0
// Executes the array's nodes from `firstNode` on as commands and returns
// the last one evaluated, restoring the variables they pushed; an array too
// short gives an int zero.
DataNode DataExecuteBlock(DataArray* array, unsigned long firstNode);  // 0x21EDA0
// Parses the text and executes it as a command.
DataNode DataExecuteString(const char* str);  // 0x21F0E0
// The bytes the array and its children use.
unsigned long GetDataArrayMemUsage(DataArray* array);  // 0x21FC70
// The value of the property path, from utl/DataUtl.o.
DataNode DataGetProperty(const DataArray* property);  // 0x23F3E0

// The variable's, property's or command's value, else the node. Inlined
// into the accessors below and into DataArray::Evaluate's copy at 0xC7D30.
inline DataNode DataNode::Evaluate() const {
    if (mType == kDataVar) {
        return DataVariable(mValue.var);
    }
    if (mType == kDataProperty) {
        return DataGetProperty(mValue.array);
    }
    if (mType == kDataCommand) {
        return DataExecute(mValue.array);
    }
    return *this;
}

// Reconstructed from eboot.elf at 0xEB40.
inline int DataNode::Int(const DataArray* source) const {
    static_cast<void>(source);
    const DataNode node = Evaluate();
    return node.mValue.integer;
}

// Reconstructed from eboot.elf at 0xEE30.
inline float DataNode::Float(const DataArray* source) const {
    static_cast<void>(source);
    const DataNode node = Evaluate();
    if (node.mType == kDataInt) {
        return static_cast<float>(node.mValue.integer);
    }
    return node.mValue.real;
}

// Reconstructed from eboot.elf at 0xE850.
inline Symbol DataNode::Sym(const DataArray* source) const {
    const DataNode node = Evaluate();
    return node.LiteralSym(source);
}

// Reconstructed from eboot.elf at 0x5F0B0.
inline DataArrayPtr DataNode::Array(const DataArray* source) const {
    static_cast<void>(source);
    const DataNode node = Evaluate();
    return DataArrayPtr(node.mValue.array);
}
