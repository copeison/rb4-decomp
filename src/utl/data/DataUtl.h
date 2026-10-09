#pragma once

class Component;
class Entity;
class DataArray;

// The component a component-reference command array names: its first node
// names an object of the thread's default entity, by name or by id, and its
// second node the component's class or base class. Null when either is
// missing. In utl/DataUtl.o, which is not otherwise reconstructed.
Component* GetDataCom(DataArray* command);  // 0x23C2E0

// Makes the entity the calling thread's default entity and its "entity"
// script variable. The map's signature is DataSetDefaultEntity(EntityPtr
// const&).
void DataSetDefaultEntity(Entity* entity);  // 0x23F780

class FixedString;

// Appends the script call stack. Empty in this build.
void DataAppendStackTrace(FixedString& out);  // 0x23B7E0

class DataNode;
class Symbol;

// Sets up the script runtime. Not reconstructed.
void DataInit();  // 0x23B710
// Defines the macro as the array, replacing an earlier definition.
void DataSetMacro(Symbol name, DataArray* value);  // 0x23B800
// A new one-node array holding a copy of the node, without a reference.
// The map instantiates it in os/PlatformMgr.o.
DataArray* MakeDataArray(const DataNode& node);  // 0x363FE0
