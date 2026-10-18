#include "utl/data/DataUtl.h"

#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"

// Reconstructed from eboot.elf at 0x23B7E0.
void DataAppendStackTrace(FixedString&) {}

// Reconstructed from eboot.elf at 0x23C2E0. Missing objects are reported;
// the report is compiled out.
Component* GetDataCom(DataArray* command) {
    const DataNode object = command->Evaluate(0);
    Entity* const entity = gDataThread.mDefaultEntity;
    GameObject* found;
    if (object.Type() == kDataSymbol) {
        found = entity->TryGetObject(*reinterpret_cast<const Symbol*>(&object.mValue.symbol), true);
    } else {
        found = entity->SafeGetObject(GameObjectId{static_cast<unsigned int>(object.mValue.integer)}, true);
    }
    if (found == nullptr) {
        return nullptr;
    }
    const Symbol com = command->Evaluate(1).LiteralSym(nullptr);
    return found->FindCom(com);
}

// Reconstructed from eboot.elf at 0x23F780. The variable's index is looked
// up once.
void DataSetDefaultEntity(Entity* entity) {
    gDataThread.mDefaultEntity = entity;
    static const unsigned long sEntityVar = DataVarIndex(Symbol("entity"));
    DataNode value;
    value.mValue.object = gDataThread.mDefaultEntity;
    value.mType = kDataObject;
    DataVariable(sEntityVar) = value;
}

// Reconstructed from eboot.elf at 0x82E00.
DataArray* MakeDataArray(const DataNode& node0, const DataNode& node1) {
    auto* array = new DataArray(2);
    array->Node(0) = node0;
    array->Node(1) = node1;
    return array;
}

// Reconstructed from eboot.elf at 0x83540.
DataArray* MakeDataArray(const DataNode& node0, const DataNode& node1, const DataNode& node2) {
    auto* array = new DataArray(3);
    array->Node(0) = node0;
    array->Node(1) = node1;
    array->Node(2) = node2;
    return array;
}
