#pragma once

#include <cstddef>

#include "render/shaders/RndShaderCollection.h"
#include "render/shaders/RndShaderDefines.h"
#include "render/shaders/RndShaderEnums.h"

class RndContext;
class RndShaderCBufferConfig;
class RndShaderResourceConfig;

// Stage masks a shader declares. Names not in the reference map.
enum RndShaderStages : int {
    kShaderStagesVertexPixel = 0x09,
    kShaderStagesGraphics = 0x0D,  // Vertex, geometry, and pixel.
    kShaderStagesCompute = 0x10,
};

// Intrusive link in the shader manager's list. Name not in the reference
// map.
struct RndShaderLink {
    RndShaderLink* mNext;
    RndShaderLink* mPrev;
};

// A built-in shader: its defines, constant buffer, and resources, and the
// compiled programs of every permutation, loaded from the shader cache. The
// base vtable is at 0x192EF60.
class RndShader {
public:
    RndShader();             // 0x6380B0
    virtual ~RndShader();    // 0x638110, 0x638250

    virtual const char* _GetClassNameImpl() const = 0;    // slot 2
    virtual const char* _GetShaderFilePath() const = 0;   // slot 3
    virtual void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) = 0;          // slot 4
    // Selects which startup option loads the shader: 0 the rendering
    // option, 1 the third option. No shader overrides it. Name not in the
    // reference map.
    virtual int _GetLoadOption() const;                   // slot 5 at 0x450850
    virtual int _GetShaderStages() const;                 // slot 6 at 0x6388E0
    // The map's key parameter is RndShaderKey.
    virtual bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const;  // slot 7 at 0x6388F0
    virtual void _SelectErrorShader(RndContext& context) const;  // slot 8 at 0x638900
    virtual bool _SupportsRTSlicing() const;              // slot 9 at 0x450870
    virtual bool _UsesCustomGeometryShader() const;       // slot 10 at 0x450880

    // Creates the configuration and runs _InitConfigImpl once.
    void InitConfig();                                    // 0x638270
    // Configures the shader and loads its programs, when the startup options
    // allow it.
    void Init();                                          // 0x6383D0
    // Frees the programs; the next select loads them again.
    void Reload();                                        // 0x6388C0
    void _Register();                                     // 0x638A20
    // Loads the programs from the shader cache.
    void _InitShaderCollection();                         // 0x638430
    bool _LoadCached(const char* path, bool validate);    // 0x638A40
    // Hash of the defines and of every valid permutation's key.
    unsigned int _ChecksumDefines() const;                // 0x638D70
    // Applies the render-target slice count to the keys and selects the
    // programs, falling back to the error shader.
    bool _SelectShaderCollection(RndContext& context, RndShaderKeyGroup& keys);  // 0x638920

    static RndShader* FromLink(RndShaderLink* link);

    // Field names are not in the reference map.
    int mShaderStages;
    bool mCollectionLoaded;
    RndShaderCollection mCollection;
    const char* mFilePath;
    RndShaderFixedDefines* mFixedDefines;
    RndShaderDefinesGroup* mDefines;
    RndShaderCBufferConfig* mCBufferConfig;
    RndShaderResourceConfig* mResourceConfig;
    RndShaderDefInfo mNumRTSlices;
    RndShaderLink mLink;
};

static_assert(offsetof(RndShader, mShaderStages) == 8);
static_assert(offsetof(RndShader, mCollectionLoaded) == 12);
static_assert(offsetof(RndShader, mCollection) == 16);
static_assert(offsetof(RndShader, mFilePath) == 208);
static_assert(offsetof(RndShader, mFixedDefines) == 216);
static_assert(offsetof(RndShader, mDefines) == 224);
static_assert(offsetof(RndShader, mCBufferConfig) == 232);
static_assert(offsetof(RndShader, mResourceConfig) == 240);
static_assert(offsetof(RndShader, mNumRTSlices) == 248);
static_assert(offsetof(RndShader, mLink) == 272);
static_assert(sizeof(RndShader) == 288);

// Base of the compute shaders. The vtable is at 0x192F0B8.
class RndShaderCompute : public RndShader {
public:
    RndShaderCompute();            // 0x63C1C0
    ~RndShaderCompute() override;  // 0x63C1F0, 0x63C200

    int _GetShaderStages() const override;  // 0x63C220
};
