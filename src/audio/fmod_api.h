#pragma once

#include <cstddef>
#include <cstdint>

// The subset of the FMOD 1.10.04 ABI used by the reconstructed audio startup.
// These declarations keep the recovered code readable until the matching FMOD
// console headers are available to the build.

enum FMOD_RESULT : std::int32_t {
    FMOD_OK = 0,
    FMOD_ERR_FILE_BAD = 13,
    FMOD_ERR_FILE_EOF = 16,
    FMOD_ERR_FILE_NOTFOUND = 18,
    FMOD_ERR_HEADER_MISMATCH = 20,
    FMOD_ERR_INVALID_HANDLE = 30,
    FMOD_ERR_INVALID_PARAM = 31,
};

enum FMOD_OUTPUTTYPE : std::int32_t {
    FMOD_OUTPUTTYPE_AUTODETECT = 0,
};

enum FMOD_DSP_RESAMPLER : std::int32_t;

enum FMOD_SPEAKERMODE : std::int32_t {
    FMOD_SPEAKERMODE_DEFAULT = 0,
    FMOD_SPEAKERMODE_RAW = 1,
    FMOD_SPEAKERMODE_MONO = 2,
    FMOD_SPEAKERMODE_STEREO = 3,
    FMOD_SPEAKERMODE_5POINT1 = 6,
    FMOD_SPEAKERMODE_7POINT1 = 7,
};

enum FMOD_SOUND_FORMAT : std::int32_t {
    FMOD_SOUND_FORMAT_PCM16 = 2,
    FMOD_SOUND_FORMAT_PCMFLOAT = 5,
};

enum FMOD_SOUND_TYPE : std::int32_t;

enum FMOD_OUTPUT_METHOD : std::int32_t {
    FMOD_OUTPUT_METHOD_MIX_DIRECT = 0,
};

struct FMOD_VECTOR {
    float x;
    float y;
    float z;
};

struct FMOD_3D_ATTRIBUTES {
    FMOD_VECTOR position;
    FMOD_VECTOR velocity;
    FMOD_VECTOR forward;
    FMOD_VECTOR up;
};

static_assert(sizeof(FMOD_VECTOR) == 12);
static_assert(sizeof(FMOD_3D_ATTRIBUTES) == 48);
static_assert(offsetof(FMOD_3D_ATTRIBUTES, forward) == 24);
static_assert(offsetof(FMOD_3D_ATTRIBUTES, up) == 36);

using FMOD_INITFLAGS = std::uint32_t;
using FMOD_MODE = std::uint32_t;
using FMOD_TIMEUNIT = std::uint32_t;
using FMOD_STUDIO_INITFLAGS = std::uint32_t;
using FMOD_SYSTEM_CALLBACK_TYPE = std::uint32_t;
using FMOD_STUDIO_EVENT_CALLBACK_TYPE = std::uint32_t;

namespace FMOD {
class Sound;
}

enum FMOD_STUDIO_PLAYBACK_STATE : std::int32_t {
    FMOD_STUDIO_PLAYBACK_PLAYING = 0,
    FMOD_STUDIO_PLAYBACK_SUSTAINING = 1,
    FMOD_STUDIO_PLAYBACK_STOPPED = 2,
    FMOD_STUDIO_PLAYBACK_STARTING = 3,
    FMOD_STUDIO_PLAYBACK_STOPPING = 4,
};

enum FMOD_STUDIO_STOP_MODE : std::int32_t {
    FMOD_STUDIO_STOP_ALLOWFADEOUT = 0,
    FMOD_STUDIO_STOP_IMMEDIATE = 1,
};

constexpr FMOD_INITFLAGS FMOD_INIT_NORMAL = 0;
constexpr FMOD_INITFLAGS FMOD_INIT_STREAM_FROM_UPDATE = 0x01;
constexpr FMOD_INITFLAGS FMOD_INIT_MIX_FROM_UPDATE = 0x02;
constexpr FMOD_INITFLAGS FMOD_INIT_3D_RIGHTHANDED = 0x04;
constexpr FMOD_MODE FMOD_3D = 0x10;
constexpr FMOD_MODE FMOD_LOOP_NORMAL = 0x02;
constexpr FMOD_MODE FMOD_CREATESTREAM = 0x80;
constexpr FMOD_TIMEUNIT FMOD_TIMEUNIT_MS = 0x01;
constexpr FMOD_TIMEUNIT FMOD_TIMEUNIT_PCM = 0x02;
constexpr std::int32_t FMOD_CHANNELCONTROL_DSP_HEAD = -3;
constexpr FMOD_STUDIO_INITFLAGS FMOD_STUDIO_INIT_NORMAL = 0;
constexpr FMOD_STUDIO_INITFLAGS FMOD_STUDIO_INIT_SYNCHRONOUS_UPDATE = 0x04;
constexpr FMOD_SYSTEM_CALLBACK_TYPE FMOD_SYSTEM_CALLBACK_PREMIX = 0x20;
constexpr FMOD_SYSTEM_CALLBACK_TYPE FMOD_SYSTEM_CALLBACK_POSTMIX = 0x40;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE
    FMOD_STUDIO_EVENT_CALLBACK_DESTROYED = 0x00000002;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE
    FMOD_STUDIO_EVENT_CALLBACK_STARTED = 0x00000008;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE
    FMOD_STUDIO_EVENT_CALLBACK_STOPPED = 0x00000020;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE
    FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND = 0x00000080;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE
    FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND = 0x00000100;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE
    FMOD_STUDIO_EVENT_CALLBACK_SOUND_PLAYED = 0x00002000;
constexpr FMOD_STUDIO_EVENT_CALLBACK_TYPE
    FMOD_STUDIO_EVENT_CALLBACK_ALL = 0xFFFFFFFF;

struct FMOD_STUDIO_SOUND_INFO {
    const char* name_or_data;
    FMOD_MODE mode;
    std::uint32_t padding;
    std::byte create_sound_info[232];
    std::int32_t subsound_index;
};

struct FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES {
    const char* name;
    FMOD::Sound* sound;
    std::int32_t subsound_index;
};

static_assert(sizeof(FMOD_STUDIO_SOUND_INFO) == 256);
static_assert(offsetof(FMOD_STUDIO_SOUND_INFO, create_sound_info) == 16);
static_assert(sizeof(FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES) == 24);
static_assert(offsetof(FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES, sound) == 8);

struct FMOD_ADVANCEDSETTINGS {
    std::int32_t cbSize;
    std::int32_t maxMPEGCodecs;
    std::int32_t maxADPCMCodecs;
    std::int32_t maxXMACodecs;
    std::int32_t maxVorbisCodecs;
    std::int32_t maxAT9Codecs;
    std::int32_t maxFADPCMCodecs;
    std::int32_t maxPCMCodecs;
    std::int32_t ASIONumChannels;
    char** ASIOChannelList;
    FMOD_SPEAKERMODE* ASIOSpeakerList;
    float HRTFMinAngle;
    float HRTFMaxAngle;
    float HRTFFreq;
    float vol0virtualvol;
    std::uint32_t defaultDecodeBufferSize;
    std::uint16_t profilePort;
    std::uint16_t profilePortPadding;
    std::uint32_t geometryMaxFadeTime;
    float distanceFilterCenterFreq;
    std::int32_t reverb3Dinstance;
    std::int32_t DSPBufferPoolSize;
    std::uint32_t stackSizeStream;
    std::uint32_t stackSizeNonBlocking;
    std::uint32_t stackSizeMixer;
    FMOD_DSP_RESAMPLER resamplerMethod;
    std::uint32_t commandQueueSize;
    std::uint32_t randomSeed;
};

struct FMOD_STUDIO_ADVANCEDSETTINGS {
    std::int32_t cbsize;
    std::uint32_t commandqueuesize;
    std::uint32_t handleinitialsize;
    std::int32_t studioupdateperiod;
    std::int32_t idlesampledatapoolsize;
};

static_assert(sizeof(FMOD_ADVANCEDSETTINGS) == 120);
static_assert(offsetof(FMOD_ADVANCEDSETTINGS, stackSizeMixer) == 104);
static_assert(offsetof(FMOD_ADVANCEDSETTINGS, commandQueueSize) == 112);
static_assert(sizeof(FMOD_STUDIO_ADVANCEDSETTINGS) == 20);

struct FMOD_DSP_DESCRIPTION;
struct FMOD_SYSTEM;

struct FMOD_OUTPUT_STATE {
    void* plugindata;
};

using FMOD_OUTPUT_GETNUMDRIVERS_CALLBACK = FMOD_RESULT (*)(
    FMOD_OUTPUT_STATE*, std::int32_t*);
using FMOD_OUTPUT_GETDRIVERINFO_CALLBACK = FMOD_RESULT (*)(
    FMOD_OUTPUT_STATE*,
    std::int32_t,
    char*,
    std::int32_t,
    void*,
    std::int32_t*,
    FMOD_SPEAKERMODE*,
    std::int32_t*);
using FMOD_OUTPUT_INIT_CALLBACK = FMOD_RESULT (*)(
    FMOD_OUTPUT_STATE*,
    std::int32_t,
    FMOD_INITFLAGS,
    std::int32_t*,
    FMOD_SPEAKERMODE*,
    std::int32_t*,
    FMOD_SOUND_FORMAT*,
    std::int32_t,
    std::int32_t,
    void*);
using FMOD_OUTPUT_STATE_CALLBACK = FMOD_RESULT (*)(FMOD_OUTPUT_STATE*);
using FMOD_OUTPUT_GETHANDLE_CALLBACK = FMOD_RESULT (*)(
    FMOD_OUTPUT_STATE*, void**);

struct FMOD_OUTPUT_DESCRIPTION {
    std::uint32_t apiversion;
    const char* name;
    std::uint32_t version;
    FMOD_OUTPUT_METHOD method;
    FMOD_OUTPUT_GETNUMDRIVERS_CALLBACK getnumdrivers;
    FMOD_OUTPUT_GETDRIVERINFO_CALLBACK getdriverinfo;
    FMOD_OUTPUT_INIT_CALLBACK init;
    FMOD_OUTPUT_STATE_CALLBACK start;
    FMOD_OUTPUT_STATE_CALLBACK stop;
    FMOD_OUTPUT_STATE_CALLBACK close;
    FMOD_OUTPUT_STATE_CALLBACK update;
    FMOD_OUTPUT_GETHANDLE_CALLBACK gethandle;
    void* optional_callbacks[10];
};

static_assert(sizeof(FMOD_OUTPUT_DESCRIPTION) == 168);
static_assert(offsetof(FMOD_OUTPUT_DESCRIPTION, getnumdrivers) == 24);
static_assert(offsetof(FMOD_OUTPUT_DESCRIPTION, update) == 72);
static_assert(offsetof(FMOD_OUTPUT_DESCRIPTION, gethandle) == 80);

struct FMOD_ASYNCREADINFO;
using FMOD_ASYNCDONE_FUNC = void (*)(FMOD_ASYNCREADINFO*, FMOD_RESULT);

struct FMOD_ASYNCREADINFO {
    void* handle;
    std::uint32_t offset;
    std::uint32_t sizebytes;
    std::int32_t priority;
    void* userdata;
    void* buffer;
    std::uint32_t bytesread;
    FMOD_ASYNCDONE_FUNC done;
};

static_assert(sizeof(FMOD_ASYNCREADINFO) == 56);
static_assert(offsetof(FMOD_ASYNCREADINFO, priority) == 16);
static_assert(offsetof(FMOD_ASYNCREADINFO, buffer) == 32);
static_assert(offsetof(FMOD_ASYNCREADINFO, done) == 48);

using FMOD_FILE_OPEN_CALLBACK = FMOD_RESULT (*)(
    const char*, std::uint32_t*, void**, void*);
using FMOD_FILE_CLOSE_CALLBACK = FMOD_RESULT (*)(void*, void*);
using FMOD_FILE_READ_CALLBACK = FMOD_RESULT (*)(
    void*, void*, std::uint32_t, std::uint32_t*, void*);
using FMOD_FILE_SEEK_CALLBACK = FMOD_RESULT (*)(void*, std::uint32_t, void*);
using FMOD_FILE_ASYNCREAD_CALLBACK = FMOD_RESULT (*)(FMOD_ASYNCREADINFO*, void*);
using FMOD_FILE_ASYNCCANCEL_CALLBACK = FMOD_RESULT (*)(FMOD_ASYNCREADINFO*, void*);
using FMOD_SYSTEM_CALLBACK = FMOD_RESULT (*)(
    FMOD_SYSTEM*, FMOD_SYSTEM_CALLBACK_TYPE, void*, void*, void*);

namespace FMOD {

class DSP;
class Channel;
class ChannelGroup;
class Sound;

class ChannelControl {
public:
    FMOD_RESULT stop();
    FMOD_RESULT setPaused(bool paused);
    FMOD_RESULT getPaused(bool* paused);
    FMOD_RESULT isPlaying(bool* playing);
    FMOD_RESULT setVolume(float volume);
    FMOD_RESULT removeDSP(DSP* dsp);
    FMOD_RESULT getNumDSPs(std::int32_t* count);
    FMOD_RESULT getDSP(std::int32_t index, DSP** dsp);
    FMOD_RESULT addDSP(std::int32_t index, DSP* dsp);
    FMOD_RESULT setMode(FMOD_MODE mode);
    FMOD_RESULT set3DSpread(float angle);
    FMOD_RESULT set3DAttributes(
        const FMOD_VECTOR* position,
        const FMOD_VECTOR* velocity,
        const FMOD_VECTOR* alternate_pan_position);
};

class Channel : public ChannelControl {
public:
    FMOD_RESULT setChannelGroup(ChannelGroup* channel_group);
    FMOD_RESULT getFrequency(float* frequency);
    FMOD_RESULT setFrequency(float frequency);
    FMOD_RESULT getPosition(
        std::uint32_t* position,
        FMOD_TIMEUNIT unit);
    FMOD_RESULT setPosition(
        std::uint32_t position,
        FMOD_TIMEUNIT unit);
    FMOD_RESULT setLoopPoints(
        std::uint32_t loop_start,
        FMOD_TIMEUNIT loop_start_unit,
        std::uint32_t loop_end,
        FMOD_TIMEUNIT loop_end_unit);
    FMOD_RESULT setLoopCount(std::int32_t loop_count);
};

class ChannelGroup : public ChannelControl {
public:
    FMOD_RESULT addGroup(
        ChannelGroup* group,
        bool propagate_clock,
        void* connection);
};

class DSP {
public:
    FMOD_RESULT getInfo(
        char* name,
        std::uint32_t* version,
        std::int32_t* channels,
        std::int32_t* config_width,
        std::int32_t* config_height);
    FMOD_RESULT getUserData(void** user_data);
    FMOD_RESULT setUserData(void* user_data);
    FMOD_RESULT release();
};

class Sound {
public:
    FMOD_RESULT release();
    FMOD_RESULT getFormat(
        FMOD_SOUND_TYPE* type,
        FMOD_SOUND_FORMAT* format,
        std::int32_t* channels,
        std::int32_t* bits);
    FMOD_RESULT getOpenState(
        std::int32_t* open_state,
        std::uint32_t* percent_buffered,
        bool* starving,
        bool* disk_busy);
    FMOD_RESULT getLength(
        std::uint32_t* length,
        FMOD_TIMEUNIT unit);
};

class System {
public:
    FMOD_RESULT getVersion(std::uint32_t* version);
    FMOD_RESULT getUserData(void** user_data);
    FMOD_RESULT setUserData(void* user_data);
    FMOD_RESULT setOutput(FMOD_OUTPUTTYPE output);
    FMOD_RESULT getAdvancedSettings(FMOD_ADVANCEDSETTINGS* settings);
    FMOD_RESULT setAdvancedSettings(FMOD_ADVANCEDSETTINGS* settings);
    FMOD_RESULT setSoftwareChannels(std::int32_t channels);
    FMOD_RESULT createDSP(
        const FMOD_DSP_DESCRIPTION* description,
        DSP** dsp);
    FMOD_RESULT playDSP(
        DSP* dsp,
        ChannelGroup* channel_group,
        bool paused,
        Channel** channel);
    FMOD_RESULT createSound(
        const char* path,
        FMOD_MODE mode,
        void* create_sound_info,
        Sound** sound);
    FMOD_RESULT playSound(
        Sound* sound,
        ChannelGroup* channel_group,
        bool paused,
        Channel** channel);
    FMOD_RESULT registerOutput(
        const FMOD_OUTPUT_DESCRIPTION* description,
        std::uint32_t* handle);
    FMOD_RESULT setOutputByPlugin(std::uint32_t handle);
    FMOD_RESULT setDSPBufferSize(
        std::uint32_t buffer_length,
        std::int32_t buffer_count);
    FMOD_RESULT setSoftwareFormat(
        std::int32_t sample_rate,
        FMOD_SPEAKERMODE speaker_mode,
        std::int32_t raw_speakers);
    FMOD_RESULT setFileSystem(
        FMOD_FILE_OPEN_CALLBACK open,
        FMOD_FILE_CLOSE_CALLBACK close,
        FMOD_FILE_READ_CALLBACK read,
        FMOD_FILE_SEEK_CALLBACK seek,
        FMOD_FILE_ASYNCREAD_CALLBACK async_read,
        FMOD_FILE_ASYNCCANCEL_CALLBACK async_cancel,
        std::int32_t block_alignment);
    FMOD_RESULT getDriver(std::int32_t* driver);
    FMOD_RESULT getDriverInfo(
        std::int32_t driver,
        char* name,
        std::int32_t name_length,
        void* guid,
        std::int32_t* system_rate,
        FMOD_SPEAKERMODE* speaker_mode,
        std::int32_t* speaker_mode_channels);
    FMOD_RESULT getSoftwareFormat(
        std::int32_t* sample_rate,
        FMOD_SPEAKERMODE* speaker_mode,
        std::int32_t* raw_speakers);
    FMOD_RESULT getDSPBufferSize(
        std::uint32_t* buffer_length,
        std::int32_t* buffer_count);
    FMOD_RESULT setCallback(
        FMOD_SYSTEM_CALLBACK callback,
        FMOD_SYSTEM_CALLBACK_TYPE callback_mask);
};

namespace Studio {

class Bus {
public:
    FMOD_RESULT getChannelGroup(FMOD::ChannelGroup** channel_group);
    FMOD_RESULT getPath(
        char* path,
        std::int32_t capacity,
        std::int32_t* retrieved);
};

class EventDescription;
class EventInstance;
class ParameterInstance;

using EventCallback = FMOD_RESULT (*)(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE,
    EventInstance*,
    void*);

class EventInstance {
public:
    FMOD_RESULT getChannelGroup(FMOD::ChannelGroup** channel_group);
    FMOD_RESULT getUserData(void** user_data);
    FMOD_RESULT setUserData(void* user_data);
    FMOD_RESULT setCallback(
        EventCallback callback,
        FMOD_STUDIO_EVENT_CALLBACK_TYPE callback_mask);
    FMOD_RESULT setPaused(bool paused);
    FMOD_RESULT start();
    FMOD_RESULT stop(FMOD_STUDIO_STOP_MODE mode);
    FMOD_RESULT release();
    FMOD_RESULT getPlaybackState(
        FMOD_STUDIO_PLAYBACK_STATE* state) const;
    FMOD_RESULT getTimelinePosition(std::int32_t* position);
    FMOD_RESULT setTimelinePosition(std::int32_t position);
    FMOD_RESULT setVolume(float volume);
    FMOD_RESULT set3DAttributes(const FMOD_3D_ATTRIBUTES* attributes);
    FMOD_RESULT setParameterValue(const char* name, float value);
    FMOD_RESULT getParameter(
        const char* name,
        ParameterInstance** parameter) const;
};

class ParameterInstance {
public:
    FMOD_RESULT getValue(float* value) const;
};

class EventDescription {
public:
    FMOD_RESULT isOneshot(bool* oneshot);
    FMOD_RESULT getLength(std::int32_t* length_ms) const;
    FMOD_RESULT createInstance(EventInstance** instance);
};

class System {
public:
    static FMOD_RESULT create(System** system, std::uint32_t header_version);
    FMOD_RESULT getUserData(void** user_data);
    FMOD_RESULT setUserData(void* user_data);
    FMOD_RESULT getLowLevelSystem(FMOD::System** system) const;
    FMOD_RESULT getAdvancedSettings(FMOD_STUDIO_ADVANCEDSETTINGS* settings);
    FMOD_RESULT setAdvancedSettings(FMOD_STUDIO_ADVANCEDSETTINGS* settings);
    FMOD_RESULT initialize(
        std::int32_t max_channels,
        FMOD_STUDIO_INITFLAGS studio_flags,
        FMOD_INITFLAGS core_flags,
        void* extra_driver_data);
    FMOD_RESULT registerPlugin(const FMOD_DSP_DESCRIPTION* description);
    FMOD_RESULT getEvent(
        const char* path,
        EventDescription** description);
    FMOD_RESULT getSoundInfo(
        const char* key,
        FMOD_STUDIO_SOUND_INFO* info) const;
    FMOD_RESULT getBus(const char* path, Bus** bus);
    FMOD_RESULT flushCommands();
    FMOD_RESULT setListenerAttributes(
        std::int32_t listener,
        const FMOD_3D_ATTRIBUTES* attributes);
    bool isValid() const;
    FMOD_RESULT update();
};

}  // namespace Studio
}  // namespace FMOD
