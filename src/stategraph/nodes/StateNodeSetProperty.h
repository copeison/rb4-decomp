#pragma once

// The registrations of the StateNodeSetProperty<T> node types, the nodes
// that set a property of each type (stategraph/StateNodeSetPropertyInit_A.o
// to _C.o), and of the StateNodeIncrementProperty<T> types this build
// adds. Each calls its types' Init. Not reconstructed.

// The binary's three groups follow the map's order; which types each holds
// is not checked, so the match of the letters is weak.
void StateNodeSetPropertyInit_A();  // 0x791400
void StateNodeSetPropertyInit_B();  // 0x793350
void StateNodeSetPropertyInit_C();  // 0x7A9AC0
// The last group is empty. Names not in the reference map.
void StateNodeIncrementPropertyInit_A();  // 0x71E670
void StateNodeIncrementPropertyInit_B();  // 0x75F980
void StateNodeIncrementPropertyInit_C();  // 0x780B10
