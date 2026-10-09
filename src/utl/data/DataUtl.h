#pragma once

class Component;
class DataArray;

// The component a component-reference command array names: its first node
// names an object of the thread's default entity, by name or by id, and its
// second node the component's class or base class. Null when either is
// missing. In utl/DataUtl.o, which is not otherwise reconstructed.
Component* GetDataCom(DataArray* command);  // 0x23C2E0
