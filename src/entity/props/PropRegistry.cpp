#include "entity/props/PropRegistry.h"

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropUtl.h"

namespace {

// The property list's first allocation. Name not in the reference map.
constexpr unsigned long kInitialPropCapacity = 32;

// The type RegisterPropArrayItem gives an array of arrays: the array flag
// on element type 14. Name not in the reference map.
constexpr unsigned int kPropertyArrayOfArrays = kPropertyArray | 14;

// The dynamic storage of an array property: a PropArray. Name not in the
// reference map.
constexpr unsigned long kDynamicArraySize = 48;

// FNV-1a over the value's bytes, each sign-extended. Inlined into
// UpdateCRC. Name not in the reference map.
template <typename T>
unsigned int HashValue(unsigned int hash, const T& value) {
    const signed char* const bytes = reinterpret_cast<const signed char*>(&value);
    for (unsigned long index = 0; index < sizeof(T); ++index) {
        hash = (hash ^ static_cast<unsigned int>(static_cast<int>(bytes[index]))) * 16777619U;
    }
    return hash;
}

}  // namespace

// Reconstructed from eboot.elf at 0x1804C0.
PropRegistry::PropRegistry()
    : mProps(),
      mDynamic(false),
      mDynamicStructSize(0),
      mDynamicStructAlign(1),
      mRevisionFunc(),
      mInheritedRegistries(),
      mCurrentRev(0) {}

// Reconstructed from eboot.elf at 0x180540.
PropRegistry::~PropRegistry() {}

// Reconstructed from eboot.elf at 0x1805F0.
void PropRegistry::SetCurrentRev(int rev) {
    mCurrentRev = rev;
}

// Reconstructed from eboot.elf at 0x180600.
void PropRegistry::ReserveProps(unsigned long count) {
    if (count > kInitialPropCapacity && mProps.capacity() < count) {
        mProps.reserve(count);
    }
}

// Reconstructed from eboot.elf at 0x180630.
PropInfo& PropRegistry::_AddProp(const char* name) {
    mProps.reserve(kInitialPropCapacity);
    const PropInfo info;
    mProps.emplace_back(name, info);
    return mProps.back().mInfo;
}

// Reconstructed from eboot.elf at 0x1807F0.
PropInfo& PropRegistry::RegisterProp(
    const char* name,
    int offset,
    PropertyType type,
    unsigned long count) {
    PropInfo& info = _AddProp(name);
    info.mOffset = offset;
    info.mType = type;
    info.mCount = count;
    info.mMetadata = PropMetadata::Create(type);
    return info;
}

// Reconstructed from eboot.elf at 0x180840.
PropInfo& PropRegistry::RegisterPropArray(const char* name, int offset, unsigned long count) {
    PropInfo& info = _AddProp(name);
    info.mOffset = offset;
    info.mType = kPropertyArray;
    info.mCount = count;
    info.mMetadata = new ArrayMetadata();
    return info;
}

// Reconstructed from eboot.elf at 0x1808A0.
PropInfo& PropRegistry::RegisterPropArrayItem(
    PropInfo& array,
    PropertyType type,
    unsigned long count) {
    array.mType = static_cast<PropertyType>(
        (type & kPropertyArray) != 0 ? kPropertyArrayOfArrays : type | kPropertyArray);
    ArrayMetadata* const metadata = static_cast<ArrayMetadata*>(array.mMetadata.Get());
    metadata->mItemInfo.mOffset = 0;
    metadata->mItemInfo.mType = type;
    metadata->mItemInfo.mCount = count;
    metadata->mItemInfo.mMetadata = PropMetadata::Create(type);
    return metadata->mItemInfo;
}

// Reconstructed from eboot.elf at 0x180910. An action has no storage; its
// metadata runs it.
PropInfo& PropRegistry::RegisterAction(const char* name) {
    PropInfo& info = _AddProp(name);
    info.mOffset = -1;
    info.mType = kPropertyAction;
    info.mCount = 0;
    info.mMetadata = new ActionMetadata();
    return info;
}

// Reconstructed from eboot.elf at 0x180980. A struct starts with no size
// and byte alignment, since AmendDynamicStructSize places it once its
// members are registered; an array takes a PropArray's 48 bytes.
PropInfo& PropRegistry::RegisterDynamicProp(
    const char* name,
    PropertyType type,
    int dynamicOffset) {
    if (type == kPropertyAction) {
        return RegisterAction(name);
    }
    PropInfo& info = _AddProp(name);
    unsigned long size;
    int alignment;
    if (type == kPropertyStruct || (type & kPropertyArray) != 0) {
        size = type == kPropertyStruct ? 0 : kDynamicArraySize;
        alignment = type == kPropertyStruct ? 1 : 8;
    } else {
        size = PropertySize(type);
        alignment = PropertyAlignment(type);
    }
    const long offset = (alignment + mDynamicStructSize - 1) & -static_cast<long>(alignment);
    mDynamicStructSize = offset;
    if (mDynamicStructAlign < alignment) {
        mDynamicStructAlign = alignment;
    }
    info.mDynamicOffset = dynamicOffset;
    info.mOffset = static_cast<int>(offset);
    info.mType = type;
    info.mCount = size;
    info.mMetadata = PropMetadata::Create(type);
    mDynamicStructSize += size;
    return info;
}

// Reconstructed from eboot.elf at 0x180AC0. The struct's size comes from
// its members' registry, and the struct starts at the next offset its
// alignment allows.
void PropRegistry::AmendDynamicStructSize() {
    PropInfo& info = mProps.back().mInfo;
    const PropRegistry& members = static_cast<GroupMetadata*>(info.mMetadata.Get())->mRegistry;
    const long size = members.mDynamicStructSize;
    const int align = members.mDynamicStructAlign;
    info.mCount = static_cast<unsigned long>(size);
    info.mOffset = (align + info.mOffset - 1) & -align;
    mDynamicStructSize = size + info.mOffset;
    if (mDynamicStructAlign < align) {
        mDynamicStructAlign = align;
    }
}

// Reconstructed from eboot.elf at 0x180B10.
void PropRegistry::FinalizeDynamicStruct() {
    mDynamicStructSize = (mDynamicStructAlign + mDynamicStructSize - 1) & -static_cast<long>(mDynamicStructAlign);
}

// Reconstructed from eboot.elf at 0x180B30.
void PropRegistry::SetDynamic(bool dynamic) {
    mDynamic = dynamic;
}

// Reconstructed from eboot.elf at 0x180B40. A name node looks the member up
// in the registry of the property found so far (this registry at first); an
// index node steps into the array. A missing member or an index into a
// non-array clears the storage; an index past the end leaves it.
const PropInfo* PropRegistry::FindProp(const PropPath& path, void** storage) const {
    if (path.mSize == 0) {
        return nullptr;
    }
    const PropRegistry* registry = this;
    const PropInfo* info = nullptr;
    for (unsigned long index = 0; index < path.mSize; ++index) {
        const PropPath::Node& node = path[static_cast<int>(index)];
        if (node.mType != PropPath::Node::kNodeSymbol) {
            if ((info->mType & kPropertyArray) == 0) {
                if (storage != nullptr) {
                    *storage = nullptr;
                }
                return nullptr;
            }
            if (storage != nullptr) {
                const PropArrayBase* array = static_cast<const PropArrayBase*>(*storage);
                if (node.mValue >= array->mSize) {
                    return nullptr;
                }
                *storage = array->ElementAt(node.mValue);
            }
            info = &info->PropArrayItemInfo();
            continue;
        }
        if (info != nullptr) {
            registry = info->GetStructRegistry();
        }
        const PropEntry* found = nullptr;
        if (registry != nullptr) {
            for (const PropEntry& entry : registry->mProps) {
                if (entry.mName.Str() == node.NameStr()) {
                    found = &entry;
                    break;
                }
            }
        }
        if (found == nullptr) {
            if (storage != nullptr) {
                *storage = nullptr;
            }
            return nullptr;
        }
        info = &found->mInfo;
        if (storage != nullptr) {
            *storage = info->Storage(*storage);
        }
    }
    return info;
}

// Reconstructed from eboot.elf at 0x180D20. Every value is hashed byte by
// byte with FNV-1a, the bytes sign-extended. A property in the component
// contributes its offset only when it has none (-1); the nesting count
// marks the properties inside dynamic storage, whose offsets are hashed.
// The visitor's copy of _VisitPropInfo is 0x184550.
void PropRegistry::UpdateCRC(unsigned int& crc, bool dynamicOnly) const {
    crc = HashValue(crc, mCurrentRev);
    int dynamicDepth = 0;
    int* const depth = &dynamicDepth;
    auto hashProp = [&dynamicDepth, dynamicOnly, &crc](const PropInfo& info, const PropPath& path) {
        static_cast<void>(path);
        if (dynamicDepth != 0 || info.mDynamicOffset >= 0) {
            ++dynamicDepth;
        }
        if ((dynamicOnly && dynamicDepth == 0) || !info.IsSaved()) {
            return false;
        }
        crc = HashValue(crc, dynamicDepth != 0 ? info.mOffset : info.mOffset & (info.mOffset >> 31));
        crc = HashValue(crc, info.mDynamicOffset);
        crc = HashValue(crc, static_cast<unsigned int>(info.mType));
        crc = HashValue(crc, info.mCount);
        if (const PropRegistry* const registry = info.GetStructRegistry()) {
            crc = HashValue(crc, registry->mCurrentRev);
        }
        return true;
    };
    VisitPropInfo(hashProp, depth);
}

// Reconstructed from eboot.elf at 0x1810C0.
void PropRegistry::RemoveProp(Symbol name) {
    for (PropEntry* entry = mProps.begin(); entry != mProps.end(); ++entry) {
        if (entry->mName == name) {
            mProps.erase(entry);
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x181180.
void PropRegistry::AddInheritedRegistry(Symbol name, const PropRegistry& registry) {
    InheritedRegistry inherited;
    inherited.mName = name;
    inherited.mRev = registry.mCurrentRev;
    inherited.mRegistry = &registry;
    mInheritedRegistries.push_back(inherited);
}

// Reconstructed from eboot.elf at 0x1813D0.
void PropRegistry::Save(const Component& component, BinStream& stream) {
    _SavePropInfo(stream);
    _SavePropValues(component, stream);
}
