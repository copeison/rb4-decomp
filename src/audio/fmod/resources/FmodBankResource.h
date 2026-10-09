#pragma once

#include <cstddef>

#include "audio/core/resources/Resource.h"
#include "audio/fmod/api/fmod_api.h"
#include "utl/containers/List.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "os/threading/CritSec.h"
#include "utl/text/Str.h"

// FMOD Studio bank loaded into every registered Studio system. Every live
// bank is also kept in a global list at 0x19F2E50, guarded by the CritSec at
// 0x19F2E40. The vtable is at 0x18F0C78; the object is 120 bytes.
class FModBankResource : public Resource {
public:
    FModBankResource();            // 0x273740
    ~FModBankResource() override;  // slots 10-11: 0x2738A0, 0x273A90

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x275030
    Symbol GetId() const override;          // slot 1: 0x275040
    bool IsA(Symbol type) const override;   // slot 2: 0x2750E0
    void LoadFile() override;               // slot 3: 0x275110
    bool Load(BinStream& stream, bool unknown) override;  // slot 4: 0x2741F0
    void Save(BinStream& stream, bool unknown) override;  // slot 5: 0x274E70
    bool Fail() const override;             // slot 6: 0x275120

    // The class id, shared by GetId and the typed Resource::GetOrLoad at
    // 0x264F70, which inline it.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("FModBankResource");
        }
        return id;
    }

    // Event paths of the first loaded bank, up to 2,048. At 0x273C10. The
    // map has GetEvents() const; this build returns the paths as Symbols.
    eastl::vector<Symbol> GetEvents() const;
    // Bus paths of the first loaded bank, up to 2,048. At 0x273F00. Name
    // not in the reference map.
    eastl::vector<Symbol> GetBuses() const;
    // Fills the type metadata at 0x19F2E80. At 0x273AF0.
    static void _Init(ResourceMetaData& metaData);

    static ResourceMetaData sMetaData;  // 0x19F2E80

    // Loads the bank into each Studio system and keeps the master bank and
    // its strings bank loaded together. At 0x274200. Name not in the
    // reference map.
    void _Load(bool reload);
    // Unloads every bank and frees the file data. At 0x273980. Name not in
    // the reference map.
    void _UnloadAll();
    // Maps a desktop bank path to the PS4 build and localizes English banks.
    // At 0x274710. Name not in the reference map.
    void _ResolvePlatformPath(String& path);
    // Loads one Studio system's copy with its sample data. At 0x274A80. Name
    // not in the reference map.
    void _LoadIntoSystem(const String& path, FMOD::Studio::System* system);
    // Companion bank paths. At 0x274920, 0x274C40 and 0x274CF0. Names not in
    // the reference map.
    String _GetLanguageMasterBankPath(const String& bankPath);
    String _GetStringsBankPath(const String& bankPath);
    String _GetMasterBankPath(const String& bankPath);
    // Copies the paths of every live bank. At 0x274F20; no caller remains in
    // this build. Name not in the reference map.
    static eastl::vector<String> GetAllBankPaths();
    // Lock the list of live banks and return it, and unlock it again. At
    // 0x273AB0 and 0x273AD0; the platform's localized-bank reload uses them.
    // Names not in the reference map.
    static eastl::list<FModBankResource*>& LockLoadedBanks();
    static void UnlockLoadedBanks();

    // The live banks and their lock. Names not in the reference map.
    static CritSec sLoadedBanksCritSec;                 // 0x19F2E40
    static eastl::list<FModBankResource*> sLoadedBanks;  // 0x19F2E50
    // The master bank and its strings bank. Loading either loads the other;
    // the flags mark the companion load in progress. Names not in the
    // reference map.
    static ResourcePtr<FModBankResource> sMasterBank;         // 0x19F2E70
    static ResourcePtr<FModBankResource> sMasterStringsBank;  // 0x19F2E78
    static bool sLoadingMasterStringsBank;                    // 0x19F2ED8
    static bool sLoadingMasterBank;                           // 0x19F2ED9

    // Field names are not in the reference map.
    // The bank loaded into each Studio system.
    eastl::map<FMOD::Studio::System*, FMOD::Studio::Bank*> mBanks;
    // Freed by _UnloadAll; nothing in this build sets it.
    void* mBankData;
    // Set when _ResolvePlatformPath localized an English bank.
    bool mLocalized;
};

static_assert(offsetof(FModBankResource, mBanks) == 48);
static_assert(offsetof(FModBankResource, mBanks.mnSize) == 88);
static_assert(offsetof(FModBankResource, mBankData) == 104);
static_assert(offsetof(FModBankResource, mLocalized) == 112);
static_assert(sizeof(FModBankResource) == 120);
