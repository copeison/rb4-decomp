#pragma once

#include <cstddef>

#include "audio/core/resources/Resource.h"
#include "audio/fmod/api/fmod_api.h"

class String;

// FMOD Studio bank loaded into every registered Studio system. Every live
// bank is also kept in a global list at 0x19F2E50, guarded by the CritSec at
// 0x19F2E40. The vtable is at 0x18F0C78. Only the simple members are
// reconstructed; the loaders use EASTL maps that have not been modelled.
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

    // Event paths of the first loaded bank. At 0x273C10. The map has
    // GetEvents() const; this build fills an EASTL vector of Symbols.
    void GetEvents(void* paths) const;
    // Bus paths of the first loaded bank. At 0x273F00. Name not in the
    // reference map.
    void GetBuses(void* paths) const;
    // Fills the type metadata at 0x19F2E80. At 0x273AF0.
    static void _Init(ResourceMetaData& metaData);

    // At 0x19F2E80. Name not in the reference map.
    static ResourceMetaData sMetaData;

    // Loads the bank into each Studio system. At 0x274200. Name not in the
    // reference map.
    bool _Load(bool reload);
    // Unloads every bank and frees the file data. At 0x273980. Name not in
    // the reference map.
    void _UnloadAll();
    // Maps a desktop bank path to the PS4 build and localizes English banks.
    // At 0x274710. Name not in the reference map.
    void _ResolvePlatformPath(String& path);
    // Loads one Studio system's copy. At 0x274A80. Name not in the reference
    // map.
    bool _LoadIntoSystem(FMOD::Studio::System* system, const char* path);
    // Companion bank paths. At 0x274920, 0x274C40 and 0x274CF0. Names not in
    // the reference map.
    String _GetLanguageMasterBankPath(const String& bankPath);
    String _GetStringsBankPath(const String& bankPath);
    String _GetMasterBankPath(const String& bankPath);
    // Copies the paths of every live bank. At 0x274F20. Name not in the
    // reference map.
    static void GetAllBankPaths(void* paths);

    // Field names are not in the reference map.
    // EASTL map from Studio system to bank: comparator and tree anchor,
    // node count and allocator.
    void* mBanks[5];
    unsigned long mNumBanks;
    void* mBanksAllocator;
    void* mBankData;
    bool mLocalized;
};

static_assert(offsetof(FModBankResource, mBanks) == 48);
static_assert(offsetof(FModBankResource, mNumBanks) == 88);
static_assert(offsetof(FModBankResource, mBankData) == 104);
static_assert(offsetof(FModBankResource, mLocalized) == 112);
static_assert(sizeof(FModBankResource) == 120);
