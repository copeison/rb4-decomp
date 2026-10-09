#pragma once

#include <cstddef>

#include "utl/text/Symbol.h"

class Component;
class DataArray;
class DataArrayPtr;
class DataNode;
class Entity;
class GameObject;
class String;

// Type tag of a DataNode. The name is the map's; the enumerators follow the
// Milo engine and are not in the reference map. Types with kDataArray set
// hold a reference-counted DataArray; the two waveform types hold a
// reference-counted waveform.
enum DataType : int {
    kDataInt = 0,
    kDataFloat = 1,
    kDataVar = 2,  // The index of a script variable.
    kDataFunc = 3,
    kDataObject = 4,
    kDataSymbol = 5,
    kDataUnhandled = 6,
    // Parser directives, kept in arrays only while a file is read.
    kDataIfdef = 7,
    kDataElse = 8,
    kDataEndif = 9,
    kDataArray = 0x10,
    kDataCommand = 0x11,
    kDataString = 0x12,  // A DataArray whose nodes are the characters.
    kDataProperty = 0x13,
    // Raw bytes in a DataArray with a negative size.
    kDataGlob = 0x14,
    kDataDefine = 0x20,
    kDataInclude = 0x21,
    kDataMerge = 0x22,
    kDataIfndef = 0x23,
    kDataAutorun = 0x24,
    kDataUndef = 0x25,
    kDataWaveformFloat = 0x26,
    kDataWaveformColor = 0x27,
    // A GameObjectId held in the low 32 bits. NotNull (0x2387C0) compares it
    // with the null id. Inferred name.
    kDataGameObjectId = 0x28,
    // A resource path: the value is the path's characters, which Str returns
    // as is and Sym resolves through FileResolvePath (0x2374E0). Sorting
    // (0x21E600) and CompatibleType (0x236F40) treat it as a symbol.
    // Inferred name.
    kDataResourcePath = 0x29,
};

// Value of a DataNode. Name not in the reference map.
union DataNodeValue {
    int integer;
    float real;
    unsigned long var;  // A kDataVar's variable index.
    DataNode (*func)(DataArray*);
    const char* symbol;
    DataArray* array;
    void* object;
};

// A typed script value. Copies share arrays and waveforms by reference.
class DataNode {
public:
    // An unhandled node with a zero value. Inline in the map's build.
    DataNode() : mType(kDataUnhandled) {
        mValue.object = nullptr;
    }
    explicit DataNode(int value) : mType(kDataInt) {
        mValue.object = nullptr;
        mValue.integer = value;
    }
    explicit DataNode(float value) : mType(kDataFloat) {
        mValue.object = nullptr;
        mValue.real = value;
    }
    // A script function node. Inline in the map's build.
    explicit DataNode(DataNode (*func)(DataArray*)) : mType(kDataFunc) {
        mValue.func = func;
    }
    // A string node holding a copy of the text.
    explicit DataNode(const char* str);  // 0x237ED0
    explicit DataNode(const String& str);  // 0x237F30
    // A glob node holding a copy of the bytes.
    DataNode(const void* glob, unsigned long size);  // 0x2383E0
    // An array node sharing the array.
    explicit DataNode(const DataArrayPtr& array);  // 0x238470
    DataNode(DataArray* array, DataType type);     // 0x238490
    DataNode(const DataNode& other);
    ~DataNode();
    DataNode& operator=(const DataNode& other);

    DataType Type() const {
        return mType;
    }

    // The node with variables, commands and properties evaluated. Inline in
    // the map's build, which emits copies in unrelated objects.
    DataNode Evaluate() const;

    // The value as an integer, float, symbol or string, evaluating
    // variables, commands and properties. The source array names the file
    // and line in errors. Inline apart from Str; the binary keeps
    // out-of-line copies of Sym at 0xE850, Int at 0xEB40 and Float at 0xEE30.
    int Int(const DataArray* source) const;
    float Float(const DataArray* source) const;
    Symbol Sym(const DataArray* source) const;
    const char* Str(const DataArray* source) const;  // 0x237570
    // The value as an array, evaluating variables, commands and
    // properties, with a reference held. Inline; the binary's copy is at
    // 0x5F0B0.
    DataArrayPtr Array(const DataArray* source) const;
    // The symbol of an evaluated node: a symbol as is, a string interned
    // and a resource path resolved. Name not in the reference map.
    Symbol LiteralSym(const DataArray* source) const;  // 0x2374E0
    // The glob's bytes, storing their count in `size` when it is given.
    // The map places a copy in utl/DataArray.o.
    const void* Glob(int* size, const DataArray* source) const;  // 0x21FD60
    // The variable a kDataVar node names, in the calling thread.
    DataNode* Var(const DataArray* source) const;  // 0x237E40
    // The array of a command node. The binary folds the map's identical
    // Func and Property accessors into this body.
    DataArray* Command(const DataArray* source) const;  // 0x237E30

    // Whether a value of the type may stand in for this node's value.
    bool CompatibleType(DataType type) const;  // 0x236F40
    // Whether the evaluated value is set: a non-empty symbol, string or
    // glob, a non-null object or a valid object id.
    bool NotNull() const;  // 0x2387C0

    void AddRefWaveform() const;   // 0x239290
    void ReleaseWaveform() const;  // 0x2392A0

    DataNodeValue mValue;
    DataType mType;
};

static_assert(sizeof(DataNode) == 16);

// The script state of one thread (the map's DataThread, created by
// utl/DataUtl.o): the thread's copy of every script variable, the stack of
// values saved by PushVar and the context commands run in.
class DataThread {
public:
    // A variable and the value it had before PushVar. Name not in the
    // reference map.
    struct VarStackEntry {
        DataNode* mVar;
        DataNode mValue;
    };

    DataThread();  // 0x23A400
    // Saves the variable's value and sets it. A weak copy in the map; not
    // located in this binary.
    void PushVar(DataNode& var, const DataNode& value);
    // Restores the most recently saved variable.
    void PopVar();  // 0x21EFD0

    // Field names are not in the reference map.
    DataNode mVars[500];
    // Saved values; the entry at the top pointer is the last one pushed, so
    // the first entry stays unused.
    VarStackEntry mVarStack[200];
    VarStackEntry* mVarStackTop;
    // A second stack with its top pointer, set up by the constructor;
    // presumably PushDataThis's. Weak evidence.
    void* mThisStack[50];
    void** mThisStackTop;
    // Keeps the result of a command evaluated by Str alive (0x237570).
    DataNode mStrResult;
    // The component the script runs for. Weak evidence: only cleared by
    // the constructor here.
    Component* mThis;
    // The object whose components receive commands named by their class
    // symbols (DataExecute, 0x21EAB0).
    GameObject* mThisObject;
    // The entity object names are looked up in (0x23C2E0); the map's
    // DataSetDefaultEntity sets it.
    Entity* mDefaultEntity;
};

static_assert(offsetof(DataThread, mVarStack) == 0x1F40);
static_assert(offsetof(DataThread, mVarStackTop) == 0x3200);
static_assert(offsetof(DataThread, mThisStackTop) == 0x3398);
static_assert(offsetof(DataThread, mStrResult) == 0x33A0);
static_assert(offsetof(DataThread, mThisObject) == 0x33B8);
static_assert(sizeof(DataThread) == 0x33C8);

// The calling thread's script state. The map has a TLSValue<DataThread>
// gDataThread in utl/DataUtl.o; the binary reaches the value through the
// thread-local descriptor at 0x19B0378, 0x20 bytes into the thread's slot.
extern thread_local DataThread gDataThread;

// The index of the script variable, registering it when it is new.
unsigned long DataVarIndex(Symbol name);  // 0x236750
// As above, but a new variable starts as `value` in every thread. Not in
// the reference map, which has only DataVarIndex(Symbol).
unsigned long DataVarIndex(Symbol name, DataNode value);  // 0x236910
// The calling thread's value of the script variable.
DataNode& DataVariable(unsigned long index);  // 0x236CC0
// Sets the named variable in every thread.
void DataSetGlobal(Symbol name, const DataNode& value);  // 0x236D50
