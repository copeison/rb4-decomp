#pragma once

// The entity module's shared constants (entity/EntityConstants.o, which has
// no code). Only those the reconstructed code uses are declared.

// The number of layers an entity can have.
extern const unsigned long kMaxLayers;  // 0x124EE90
// The layer every entity has, which holds its root object.
extern const unsigned long kMainLayerIndex;  // 0x124EEA0
// The entity revision the loaders write and accept, 33.
extern const int kEntityRev;  // 0x124EEB8
// The entity resource revision Save writes and the newest _LoadEntity
// reads, 18.
extern const int kEntityResourceRev;  // 0x124EEBC
// The main layer's name, "main".
extern const char* kMainLayerName;  // 0x19B02C8
// The extension of an entity layer file, "layer".
extern const char* kLayerExt;  // 0x19B02D0
// The revision of the entity file being read; the loaders set it from the
// stream and restore it afterwards. Initially 33.
extern int gEntityRev;  // 0x19B02D8
