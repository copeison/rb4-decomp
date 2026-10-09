#include "entity/core/Component.h"

#include <kernel.h>
#include <strings.h>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/TransCom.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "entity/resources/Resource.h"
#include "os/memory/MemMgr.h"
#include "utl/data/DataArray.h"
#include "utl/streams/BinStream.h"
#include "utl/text/MakeString.h"

namespace {

// The revision _Save writes. Name not in the reference map.
constexpr int kComponentSaveRev = 3;

// The size of the buffer the compiled-out failure reports print the path
// into. Name not in the reference map.
constexpr unsigned long kPathStringSize = 512;

// The FNV-1a constants of the class CRC, and the value the dependency hash
// starts from when there are dependencies. Names not in the reference map.
constexpr unsigned int kFnvBasis = 2166136261U;
constexpr unsigned int kFnvPrime = 16777619U;
constexpr unsigned int kOrderDepsSeed = 671913016U;

// Sorts the symbols by their text without regard to case; equal symbols
// keep no particular order. The binary's EASTL introsort and insertion
// sort (0xE9170, 0xE9370) give the same order. Name not in the reference
// map.
void SortSymbolsNoCase(eastl::vector<Symbol>& symbols) {
    Symbol* const begin = symbols.begin();
    const unsigned long count = symbols.size();
    for (unsigned long index = 1; index < count; ++index) {
        const Symbol value = begin[index];
        unsigned long position = index;
        while (position != 0 && begin[position - 1] != value &&
               strcasecmp(value.Str(), begin[position - 1].Str()) < 0) {
            begin[position] = begin[position - 1];
            --position;
        }
        begin[position] = value;
    }
}

// What remains of a failure report: the class id is read and the path is
// printed into a cleared buffer. Name not in the reference map.
void ReportPropPathFailure(const Component& component, const PropPath& path) {
    static_cast<void>(component.GetId());
    char text[kPathStringSize] = {};
    path.ToString(text, sizeof(text));
}

}  // namespace

// The class factories at 0x19E2900.
eastl::map<Symbol, Component* (*)()> Component::sFactory;
// The flag at 0x19E2970.
bool Component::sRegressionTesting;

// Reconstructed from eboot.elf at 0xE7190.
Component::Component()
    : mObject(nullptr),
      mImprinted(false),
      mPropsSynced(true),
      mResourcesRequested(false),
      mResourcesReady(false),
      mEntered(false),
      mReserved(false) {}

// Reconstructed from eboot.elf at 0xE71C0. Nothing is copied from the other
// component; the imprint's properties are copied by _ImprintProps.
Component::Component(const Component& other)
    : mObject(nullptr),
      mImprinted(false),
      mPropsSynced(true),
      mResourcesRequested(false),
      mResourcesReady(false),
      mEntered(false),
      mReserved(false) {
    static_cast<void>(other);
}

// Reconstructed from eboot.elf at 0xE71F0.
Component::~Component() {}

// Reconstructed from eboot.elf at 0xE7210.
void Component::Destroy() {
    if (mImprinted) {
        this->~Component();
    } else {
        delete this;
    }
}

// Reconstructed from eboot.elf at 0xE7220.
bool Component::IsValid(Symbol id) {
    return sFactory.find(id) != sFactory.end();
}

// Reconstructed from eboot.elf at 0xE72A0.
unsigned int Component::GetClassCRC(Symbol id) {
    unsigned int temp;
    MemPushTemp(temp, true, true);
    unsigned int crc = kFnvBasis;
    const auto factory = sFactory.find(id);
    if (factory != sFactory.end()) {
        if (Component* const component = factory->second()) {
            crc = component->_GetOrderDepsCRC();
            delete component;
        }
    }
    MemPopTemp(temp);
    return crc;
}

// Reconstructed from eboot.elf at 0xE7380. Each list has room for three
// classes before the virtuals fill it. The hash of non-empty lists starts
// from a fixed value rather than the basis.
unsigned int Component::_GetOrderDepsCRC() {
    unsigned int temp;
    MemPushTemp(temp, true, true);
    eastl::vector<Symbol> pollFollows;
    eastl::vector<Symbol> pollPrecedes;
    eastl::vector<Symbol> comFollows;
    eastl::vector<Symbol> comPrecedes;
    pollFollows.reserve(3);
    pollPrecedes.reserve(3);
    comFollows.reserve(3);
    comPrecedes.reserve(3);
    _GetPollOrderDeps(pollFollows, pollPrecedes);
    _GetComponentOrderDeps(comFollows, comPrecedes);
    unsigned int crc = kFnvBasis;
    if (!pollFollows.empty() || !pollPrecedes.empty() || !comFollows.empty() || !comPrecedes.empty()) {
        eastl::vector<Symbol>* const lists[] = {&pollFollows, &pollPrecedes, &comFollows, &comPrecedes};
        crc = kOrderDepsSeed;
        for (unsigned long list = 0; list < 4; ++list) {
            SortSymbolsNoCase(*lists[list]);
        }
        for (unsigned long list = 0; list < 4; ++list) {
            for (const Symbol& name : *lists[list]) {
                for (const char* c = name.Str(); *c != '\0'; ++c) {
                    crc = kFnvPrime * (crc ^ static_cast<unsigned char>(*c));
                }
            }
            if (list != 3) {
                crc = kFnvPrime * (crc ^ '-');
            }
        }
    }
    MemPopTemp(temp);
    return crc;
}

// Reconstructed from eboot.elf at 0xE78E0.
int Component::Size(const PropPath& path) const {
    void* storage = const_cast<Component*>(this);
    const PropInfo* info = const_cast<Component*>(this)->_GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr || (info->mType & kPropertyArray) == 0) {
        ReportPropPathFailure(*this, path);
        return 0;
    }
    return static_cast<int>(static_cast<PropArrayBase*>(storage)->mSize);
}

// Reconstructed from eboot.elf at 0xE7AC0.
bool Component::Insert(const PropPath& path) {
    return _Insert(path, nullptr, kPropertyNone);
}

// Reconstructed from eboot.elf at 0xE7AD0. The array is found through the
// path without its last node, the index.
bool Component::_Insert(const PropPath& path, const void* value, PropertyType type) {
    PropPath arrayPath = path;
    arrayPath.PopNode();
    void* storage = this;
    const PropInfo* info = _GetPropRegistry().FindProp(arrayPath, &storage);
    if (info == nullptr || (info->mType & kPropertyArray) == 0) {
        ReportPropPathFailure(*this, arrayPath);
        return false;
    }
    PropArrayBase* const array = static_cast<PropArrayBase*>(storage);
    const unsigned int size = array->mSize;
    if (!_InsertProp(*this, path, *info, *array, value, type)) {
        ReportPropPathFailure(*this, path);
        return false;
    }
    return size < array->mSize;
}

// Reconstructed from eboot.elf at 0xE7E50.
bool Component::Remove(const PropPath& path) {
    PropPath arrayPath = path;
    arrayPath.PopNode();
    void* storage = this;
    const PropInfo* info = _GetPropRegistry().FindProp(arrayPath, &storage);
    if (info == nullptr || (info->mType & kPropertyArray) == 0) {
        ReportPropPathFailure(*this, arrayPath);
        return false;
    }
    PropArrayBase* const array = static_cast<PropArrayBase*>(storage);
    const unsigned int size = array->mSize;
    _RemoveProp(*this, path, *info, *array);
    return array->mSize < size;
}

// Reconstructed from eboot.elf at 0xE80F0. An entered component exits in
// its entity's mode before the load and re-enters after it unless slot 29
// refuses; the thread's load context names the component meanwhile.
bool Component::LoadResources(bool quiet) {
    const Resource::LoadContext previous = Resource::GetLoadContext();
    Resource::LoadContext context;
    context.mComponent = this;
    Resource::SetLoadContext(context);
    mResourcesRequested = true;

    unsigned int mode = 0;
    if (mEntered) {
        // The thread is checked by an assertion the release build drops.
        static_cast<void>(scePthreadSelf());
        const unsigned int flags = mObject->mEntity->mFlags;
        mode = (flags & 0x10) != 0 ? (flags >> 1) & 3 : 0;
        if (mode == 2) {
            mEntered = false;
            _EditExit(kDestroyComponent);
        } else if (mode == 1) {
            mEntered = false;
            _Exit(kDestroyComponent);
        }
    }

    const bool loaded = _LoadResources();
    if (!loaded && !quiet) {
        LoadPropResources(this);
    }
    bool reentered;
    if (_OnResourcesLoaded()) {
        if (!quiet && loaded) {
            LoadPropResources(this);
        }
        reentered = true;
        if (mode == 2) {
            mEntered = true;
            _EditEnter();
        } else if (mode == 1) {
            mEntered = true;
            _Enter();
        }
    } else {
        reentered = false;
    }
    Resource::SetLoadContext(previous);
    return reentered;
}

// Reconstructed from eboot.elf at 0xE8250.
bool Component::AreResourcesReady() {
    if (mResourcesReady) {
        return true;
    }
    mResourcesReady = _AreResourcesReady();
    return mResourcesReady;
}

// Reconstructed from eboot.elf at 0xE8280.
const char* Component::MakeErrorName() const {
    const Symbol id = GetId();
    const char* const object = mObject->MakeErrorName();
    FormatString format("%s component in %s");
    format << id << object;
    return format.Str();
}

// Reconstructed from eboot.elf at 0xE8310.
bool Component::StorePropState(const PropPath& path, BinStream& stream) const {
    return _StorePropState(*this, path, stream, nullptr);
}

// Reconstructed from eboot.elf at 0xE8320.
bool Component::ApplyPropState(const PropPath& path, BinStream& stream) {
    return _ApplyStoredPropState(*this, path, stream, false);
}

// Reconstructed from eboot.elf at 0xE84C0.
DataNode Component::_OnRemove(DataArray* msg) {
    PropPath path;
    path.FromDataArray(msg, 2);
    Remove(path);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xE8590.
DataNode Component::_OnInsert(DataArray* msg) {
    PropPath path;
    path.FromDataArray(msg, 2);
    _Insert(path, nullptr, kPropertyNone);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xE8670. The binary inlines Size,
// _OnRemove and _OnInsert; the message's second node names the command.
DataNode Component::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol command = msg->Sym(1);

    static Symbol loadResources;
    if (loadResources == Symbol()) {
        loadResources = Symbol("load_resources");
    }
    if (command == loadResources) {
        LoadResources(false);
        return DataNode(0);
    }

    static Symbol size;
    if (size == Symbol()) {
        size = Symbol("size");
    }
    if (command == size) {
        PropPath path;
        PropPath::Node node;
        node = msg->Sym(2);
        path.PushNode(node);
        return DataNode(Size(path));
    }

    static Symbol remove;
    if (remove == Symbol()) {
        remove = Symbol("remove");
    }
    if (command == remove) {
        PropPath path;
        path.FromDataArray(msg, 2);
        Remove(path);
        return DataNode(0);
    }

    static Symbol insert;
    if (insert == Symbol()) {
        insert = Symbol("insert");
    }
    if (command == insert) {
        PropPath path;
        path.FromDataArray(msg, 2);
        _Insert(path, nullptr, kPropertyNone);
        return DataNode(0);
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0xE8C20.
int Component::SaveRev() {
    return kComponentSaveRev;
}

// Reconstructed from eboot.elf at 0xE8C30.
void Component::_Save(BinStream& stream) {
    const int rev = kComponentSaveRev;
    stream.WriteEndian(&rev, sizeof(rev));
    _GetPropRegistry().Save(*this, stream);
}

// Reconstructed from eboot.elf at 0xE8CA0. A revision below 2 has no
// properties; its failure report is compiled out but still reads the
// stream's name.
void Component::_Load(Component* component, BinStream& stream) {
    unsigned int rev = 0;
    stream.ReadEndian(&rev, sizeof(rev));
    if (rev <= 1) {
        static_cast<void>(stream.Name());
    } else {
        PropRegistry::Load(component, stream);
    }
    if (component != nullptr) {
        component->_PostLoad(stream);
    }
}

// Reconstructed from eboot.elf at 0xE8D30.
char* Component::_ImprintProps(char* buffer, Component* imprint) {
    const PropRegistry& registry = _GetPropRegistry();
    const PropRegistry* imprintRegistry = imprint != nullptr ? &imprint->_GetPropRegistry() : nullptr;
    return _ImprintRegistry(buffer, imprint, registry, imprintRegistry);
}

// Reconstructed from eboot.elf at 0xE8FA0.
void Component::_GetComponentOrderDeps(eastl::vector<Symbol>& follows, eastl::vector<Symbol>& precedes) {
    static_cast<void>(precedes);
    follows.push_back(TransCom::sClassName);
}

// Reconstructed from eboot.elf at 0xE9060.
void Component::_GetPollOrderDeps(eastl::vector<Symbol>& follows, eastl::vector<Symbol>& precedes) {
    static_cast<void>(follows);
    static_cast<void>(precedes);
}

// Reconstructed from eboot.elf at 0xAEA0.
Entity* Component::GetEntity() const {
    return mObject != nullptr ? mObject->mEntity : nullptr;
}

// Reconstructed from eboot.elf at 0xAED0.
Symbol Component::GetInterfaceId() const {
    return const_cast<Component*>(this)->_GetMetaData().mInterface;
}
