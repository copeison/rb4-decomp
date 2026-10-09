#pragma once

#include <cstddef>
#include <cstdint>

// The subset of the FMOD 1.10.04 ABI used by the reconstructed audio code.
// Every method below matches an FMOD import of the executable, including its
// const qualification, so the relocatable link names the real FMOD symbols.
// These declarations stand in for the FMOD console headers.

enum FMOD_RESULT : std::int32_t {
    FMOD_OK = 0,
    FMOD_ERR_FILE_BAD = 13,
    FMOD_ERR_FILE_EOF = 16,
    FMOD_ERR_FILE_NOTFOUND = 18,
    FMOD_ERR_HEADER_MISMATCH = 20,
    FMOD_ERR_INVALID_HANDLE = 30,
    FMOD_ERR_INVALID_PARAM = 31,
    FMOD_ERR_STUDIO_NOT_LOADED = 76,
};

enum FMOD_OPENSTATE : std::int32_t {
    FMOD_OPENSTATE_READY = 0,
    FMOD_OPENSTATE_LOADING = 1,
    FMOD_OPENSTATE_ERROR = 2,
    FMOD_OPENSTATE_CONNECTING = 3,
    FMOD_OPENSTATE_BUFFERING = 4,
    FMOD_OPENSTATE_SEEKING = 5,
    FMOD_OPENSTATE_PLAYING = 6,
    FMOD_OPENSTATE_SETPOSITION = 7,
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

enum FMOD_DSP_PROCESS_OPERATION : std::int32_t {
    FMOD_DSP_PROCESS_PERFORM = 0,
    FMOD_DSP_PROCESS_QUERY = 1,
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

struct FMOD_ORBIS_THREAD_AFFINITY {
    std::uint32_t masks[11];
};

static_assert(sizeof(FMOD_ORBIS_THREAD_AFFINITY) == 44);

extern "C" FMOD_RESULT FMOD_Orbis_SetThreadAffinity(
    const FMOD_ORBIS_THREAD_AFFINITY* affinity);

using FMOD_INITFLAGS = std::uint32_t;
using FMOD_MODE = std::uint32_t;
using FMOD_TIMEUNIT = std::uint32_t;
using FMOD_CHANNELMASK = std::uint32_t;
using FMOD_STUDIO_INITFLAGS = std::uint32_t;
using FMOD_SYSTEM_CALLBACK_TYPE = std::uint32_t;
using FMOD_STUDIO_EVENT_CALLBACK_TYPE = std::uint32_t;
using FMOD_DRIVER_STATE = std::uint32_t;

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

enum FMOD_STUDIO_LOADING_STATE : std::int32_t {
    FMOD_STUDIO_LOADING_STATE_UNLOADING = 0,
    FMOD_STUDIO_LOADING_STATE_UNLOADED = 1,
    FMOD_STUDIO_LOADING_STATE_LOADING = 2,
    FMOD_STUDIO_LOADING_STATE_LOADED = 3,
    FMOD_STUDIO_LOADING_STATE_ERROR = 4,
};

constexpr FMOD_INITFLAGS FMOD_INIT_NORMAL = 0;
constexpr FMOD_INITFLAGS FMOD_INIT_STREAM_FROM_UPDATE = 0x01;
constexpr FMOD_INITFLAGS FMOD_INIT_MIX_FROM_UPDATE = 0x02;
constexpr FMOD_INITFLAGS FMOD_INIT_3D_RIGHTHANDED = 0x04;
constexpr FMOD_MODE FMOD_DEFAULT = 0x00000000;
constexpr FMOD_MODE FMOD_LOOP_OFF = 0x00000001;
constexpr FMOD_MODE FMOD_LOOP_NORMAL = 0x00000002;
constexpr FMOD_MODE FMOD_2D = 0x00000008;
constexpr FMOD_MODE FMOD_3D = 0x00000010;
constexpr FMOD_MODE FMOD_CREATESTREAM = 0x00000080;
constexpr FMOD_MODE FMOD_CREATESAMPLE = 0x00000100;
constexpr FMOD_MODE FMOD_CREATECOMPRESSEDSAMPLE = 0x00000200;
constexpr FMOD_MODE FMOD_OPENUSER = 0x00000400;
constexpr FMOD_MODE FMOD_ACCURATETIME = 0x00004000;
constexpr FMOD_MODE FMOD_NONBLOCKING = 0x00010000;
constexpr FMOD_TIMEUNIT FMOD_TIMEUNIT_MS = 0x01;
constexpr FMOD_TIMEUNIT FMOD_TIMEUNIT_PCM = 0x02;
constexpr FMOD_TIMEUNIT FMOD_TIMEUNIT_PCMBYTES = 0x04;
constexpr FMOD_CHANNELMASK FMOD_CHANNELMASK_FRONT_LEFT = 0x01;
constexpr FMOD_CHANNELMASK FMOD_CHANNELMASK_FRONT_RIGHT = 0x02;
constexpr FMOD_CHANNELMASK FMOD_CHANNELMASK_STEREO =
    FMOD_CHANNELMASK_FRONT_LEFT | FMOD_CHANNELMASK_FRONT_RIGHT;
constexpr std::int32_t FMOD_CHANNELCONTROL_DSP_HEAD = -1;
constexpr std::int32_t FMOD_CHANNELCONTROL_DSP_FADER = -2;
constexpr std::int32_t FMOD_CHANNELCONTROL_DSP_TAIL = -3;
constexpr FMOD_DRIVER_STATE FMOD_DRIVER_STATE_CONNECTED = 0x1;
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

// Extended creation information. Mic_FMOD::Start fills the leading fields
// for its record buffer; Studio's blocks are forwarded unread.
struct alignas(8) FMOD_CREATESOUNDEXINFO {
    std::int32_t cbsize;
    std::uint32_t length;
    std::uint32_t fileoffset;
    std::int32_t numchannels;
    std::int32_t defaultfrequency;
    FMOD_SOUND_FORMAT format;
    std::uint8_t remaining[208];
};

static_assert(sizeof(FMOD_CREATESOUNDEXINFO) == 232);

struct FMOD_STUDIO_SOUND_INFO {
    const char* name_or_data;
    FMOD_MODE mode;
    FMOD_CREATESOUNDEXINFO exinfo;
    std::int32_t subsoundindex;
};

struct FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES {
    const char* name;
    FMOD::Sound* sound;
    std::int32_t subsoundIndex;
};

static_assert(sizeof(FMOD_STUDIO_SOUND_INFO) == 256);
static_assert(offsetof(FMOD_STUDIO_SOUND_INFO, exinfo) == 16);
static_assert(offsetof(FMOD_STUDIO_SOUND_INFO, subsoundindex) == 248);
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

struct FMOD_SYSTEM;
struct FMOD_STUDIO_EVENTINSTANCE;
struct FMOD_STUDIO_PARAMETER_DESCRIPTION;
struct FMOD_TAG;

struct FMOD_GUID {
    std::uint32_t Data1;
    std::uint16_t Data2;
    std::uint16_t Data3;
    std::uint8_t Data4[8];
};

static_assert(sizeof(FMOD_GUID) == 16);

// Plugin state passed to every DSP callback. Only the instance pointer is
// read by the engine.
struct FMOD_DSP_STATE {
    void* instance;
};

struct FMOD_DSP_BUFFER_ARRAY {
    std::int32_t numbuffers;
    std::int32_t* buffernumchannels;
    FMOD_CHANNELMASK* bufferchannelmask;
    float** buffers;
    FMOD_SPEAKERMODE speakermode;
};

static_assert(offsetof(FMOD_DSP_BUFFER_ARRAY, buffernumchannels) == 8);
static_assert(offsetof(FMOD_DSP_BUFFER_ARRAY, bufferchannelmask) == 16);
static_assert(offsetof(FMOD_DSP_BUFFER_ARRAY, buffers) == 24);
static_assert(offsetof(FMOD_DSP_BUFFER_ARRAY, speakermode) == 32);

using FMOD_DSP_CREATE_CALLBACK = FMOD_RESULT (*)(FMOD_DSP_STATE*);
using FMOD_DSP_PROCESS_CALLBACK = FMOD_RESULT (*)(
    FMOD_DSP_STATE*,
    std::uint32_t,
    const FMOD_DSP_BUFFER_ARRAY*,
    FMOD_DSP_BUFFER_ARRAY*,
    bool,
    FMOD_DSP_PROCESS_OPERATION);

struct FMOD_DSP_DESCRIPTION {
    std::uint32_t pluginsdkversion;
    char name[32];
    std::uint32_t version;
    std::int32_t numinputbuffers;
    std::int32_t numoutputbuffers;
    FMOD_DSP_CREATE_CALLBACK create;
    void* release;
    void* reset;
    void* read;
    FMOD_DSP_PROCESS_CALLBACK process;
    void* setposition;
    std::int32_t numparameters;
    void* paramdesc;
    void* setparameterfloat;
    void* setparameterint;
    void* setparameterbool;
    void* setparameterdata;
    void* getparameterfloat;
    void* getparameterint;
    void* getparameterbool;
    void* getparameterdata;
    void* shouldiprocess;
    void* userdata;
    void* sys_register;
    void* sys_deregister;
    void* sys_mix;
};

static_assert(offsetof(FMOD_DSP_DESCRIPTION, version) == 36);
static_assert(offsetof(FMOD_DSP_DESCRIPTION, create) == 48);
static_assert(offsetof(FMOD_DSP_DESCRIPTION, process) == 80);
static_assert(sizeof(FMOD_DSP_DESCRIPTION) == 216);

struct FMOD_OUTPUT_STATE;
using FMOD_OUTPUT_READFROMMIXER = FMOD_RESULT (*)(
    FMOD_OUTPUT_STATE*, void*, std::uint32_t);

struct FMOD_OUTPUT_STATE {
    void* plugindata;
    FMOD_OUTPUT_READFROMMIXER readfrommixer;
};

using FMOD_OUTPUT_GETNUMDRIVERS_CALLBACK = FMOD_RESULT (*)(
    FMOD_OUTPUT_STATE*, std::int32_t*);
using FMOD_OUTPUT_GETDRIVERINFO_CALLBACK = FMOD_RESULT (*)(
    FMOD_OUTPUT_STATE*,
    std::int32_t,
    char*,
    std::int32_t,
    FMOD_GUID*,
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
using FMOD_STUDIO_EVENT_CALLBACK = FMOD_RESULT (*)(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE, FMOD_STUDIO_EVENTINSTANCE*, void*);

namespace FMOD {

class DSP;
class DSPConnection;
class Channel;
class ChannelGroup;
class Sound;

class ChannelControl {
public:
    FMOD_RESULT stop();
    FMOD_RESULT setPaused(bool paused);
    FMOD_RESULT getPaused(bool* paused);
    FMOD_RESULT isPlaying(bool* isplaying);
    FMOD_RESULT setVolume(float volume);
    FMOD_RESULT getVolume(float* volume);
    FMOD_RESULT setMute(bool mute);
    FMOD_RESULT removeDSP(DSP* dsp);
    FMOD_RESULT getNumDSPs(std::int32_t* numdsps);
    FMOD_RESULT getDSP(std::int32_t index, DSP** dsp);
    FMOD_RESULT addDSP(std::int32_t index, DSP* dsp);
    FMOD_RESULT setMode(FMOD_MODE mode);
    FMOD_RESULT set3DSpread(float angle);
    FMOD_RESULT set3DLevel(float level);
    FMOD_RESULT set3DMinMaxDistance(float mindistance, float maxdistance);
    FMOD_RESULT set3DAttributes(
        const FMOD_VECTOR* pos,
        const FMOD_VECTOR* vel,
        const FMOD_VECTOR* alt_pan_pos);
};

class Channel : public ChannelControl {
public:
    FMOD_RESULT setChannelGroup(ChannelGroup* channelgroup);
    FMOD_RESULT getFrequency(float* frequency);
    FMOD_RESULT setFrequency(float frequency);
    FMOD_RESULT getPosition(std::uint32_t* position, FMOD_TIMEUNIT postype);
    FMOD_RESULT setPosition(std::uint32_t position, FMOD_TIMEUNIT postype);
    FMOD_RESULT setLoopPoints(
        std::uint32_t loopstart,
        FMOD_TIMEUNIT loopstarttype,
        std::uint32_t loopend,
        FMOD_TIMEUNIT loopendtype);
    FMOD_RESULT setLoopCount(std::int32_t loopcount);
};

class ChannelGroup : public ChannelControl {
public:
    FMOD_RESULT addGroup(
        ChannelGroup* group,
        bool propagatedspclock,
        DSPConnection** connection);
};

class DSP {
public:
    FMOD_RESULT getInfo(
        char* name,
        std::uint32_t* version,
        std::int32_t* channels,
        std::int32_t* configwidth,
        std::int32_t* configheight);
    FMOD_RESULT getUserData(void** userdata);
    FMOD_RESULT setUserData(void* userdata);
    FMOD_RESULT setChannelFormat(
        FMOD_CHANNELMASK channelmask,
        std::int32_t numchannels,
        FMOD_SPEAKERMODE source_speakermode);
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
        FMOD_OPENSTATE* openstate,
        std::uint32_t* percentbuffered,
        bool* starving,
        bool* diskbusy);
    FMOD_RESULT getLength(std::uint32_t* length, FMOD_TIMEUNIT lengthtype);
    FMOD_RESULT getDefaults(float* frequency, std::int32_t* priority);
    FMOD_RESULT seekData(std::uint32_t pcm);
    FMOD_RESULT lock(
        std::uint32_t offset,
        std::uint32_t length,
        void** ptr1,
        void** ptr2,
        std::uint32_t* len1,
        std::uint32_t* len2);
    FMOD_RESULT unlock(void* ptr1, void* ptr2, std::uint32_t len1, std::uint32_t len2);
    FMOD_RESULT readData(
        void* buffer,
        std::uint32_t length,
        std::uint32_t* read);
};

class System {
public:
    FMOD_RESULT getVersion(std::uint32_t* version);
    FMOD_RESULT getUserData(void** userdata);
    FMOD_RESULT setUserData(void* userdata);
    FMOD_RESULT setOutput(FMOD_OUTPUTTYPE output);
    FMOD_RESULT getAdvancedSettings(FMOD_ADVANCEDSETTINGS* settings);
    FMOD_RESULT setAdvancedSettings(FMOD_ADVANCEDSETTINGS* settings);
    FMOD_RESULT setSoftwareChannels(std::int32_t numsoftwarechannels);
    FMOD_RESULT createDSP(const FMOD_DSP_DESCRIPTION* description, DSP** dsp);
    FMOD_RESULT playDSP(
        DSP* dsp,
        ChannelGroup* channelgroup,
        bool paused,
        Channel** channel);
    FMOD_RESULT createSound(
        const char* name_or_data,
        FMOD_MODE mode,
        FMOD_CREATESOUNDEXINFO* exinfo,
        Sound** sound);
    FMOD_RESULT playSound(
        Sound* sound,
        ChannelGroup* channelgroup,
        bool paused,
        Channel** channel);
    FMOD_RESULT registerOutput(
        const FMOD_OUTPUT_DESCRIPTION* description,
        std::uint32_t* handle);
    FMOD_RESULT setOutputByPlugin(std::uint32_t handle);
    FMOD_RESULT setDSPBufferSize(std::uint32_t bufferlength, std::int32_t numbuffers);
    FMOD_RESULT getDSPBufferSize(std::uint32_t* bufferlength, std::int32_t* numbuffers);
    FMOD_RESULT setSoftwareFormat(
        std::int32_t samplerate,
        FMOD_SPEAKERMODE speakermode,
        std::int32_t numrawspeakers);
    FMOD_RESULT getSoftwareFormat(
        std::int32_t* samplerate,
        FMOD_SPEAKERMODE* speakermode,
        std::int32_t* numrawspeakers);
    FMOD_RESULT setFileSystem(
        FMOD_FILE_OPEN_CALLBACK useropen,
        FMOD_FILE_CLOSE_CALLBACK userclose,
        FMOD_FILE_READ_CALLBACK userread,
        FMOD_FILE_SEEK_CALLBACK userseek,
        FMOD_FILE_ASYNCREAD_CALLBACK userasyncread,
        FMOD_FILE_ASYNCCANCEL_CALLBACK userasynccancel,
        std::int32_t blockalign);
    FMOD_RESULT getDriver(std::int32_t* driver);
    FMOD_RESULT getDriverInfo(
        std::int32_t id,
        char* name,
        std::int32_t namelen,
        FMOD_GUID* guid,
        std::int32_t* systemrate,
        FMOD_SPEAKERMODE* speakermode,
        std::int32_t* speakermodechannels);
    FMOD_RESULT getRecordNumDrivers(
        std::int32_t* numdrivers,
        std::int32_t* numconnected);
    FMOD_RESULT getRecordDriverInfo(
        std::int32_t id,
        char* name,
        std::int32_t namelen,
        FMOD_GUID* guid,
        std::int32_t* systemrate,
        FMOD_SPEAKERMODE* speakermode,
        std::int32_t* speakermodechannels,
        FMOD_DRIVER_STATE* state);
    FMOD_RESULT setCallback(
        FMOD_SYSTEM_CALLBACK callback,
        FMOD_SYSTEM_CALLBACK_TYPE callbackmask);
    FMOD_RESULT recordStart(std::int32_t id, Sound* sound, bool loop);
    FMOD_RESULT recordStop(std::int32_t id);
    FMOD_RESULT getRecordPosition(std::int32_t id, std::uint32_t* position);
    FMOD_RESULT mixerSuspend();
    FMOD_RESULT mixerResume();
};

namespace Studio {

class Bank;
class EventDescription;
class EventInstance;
class ParameterInstance;

class Bus {
public:
    FMOD_RESULT getChannelGroup(FMOD::ChannelGroup** group) const;
    FMOD_RESULT getVolume(float* volume, float* finalvolume) const;
    FMOD_RESULT setVolume(float volume);
    FMOD_RESULT setMute(bool mute);
    FMOD_RESULT getPath(char* path, std::int32_t size, std::int32_t* retrieved) const;
    FMOD_RESULT lockChannelGroup();
};

class EventInstance {
public:
    FMOD_RESULT getChannelGroup(FMOD::ChannelGroup** group) const;
    FMOD_RESULT getUserData(void** userdata) const;
    FMOD_RESULT setUserData(void* userdata);
    FMOD_RESULT setCallback(
        FMOD_STUDIO_EVENT_CALLBACK callback,
        FMOD_STUDIO_EVENT_CALLBACK_TYPE callbackmask);
    FMOD_RESULT setPaused(bool paused);
    FMOD_RESULT start();
    FMOD_RESULT stop(FMOD_STUDIO_STOP_MODE mode);
    FMOD_RESULT release();
    FMOD_RESULT getPlaybackState(FMOD_STUDIO_PLAYBACK_STATE* state) const;
    FMOD_RESULT getTimelinePosition(std::int32_t* position) const;
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
    FMOD_RESULT isOneshot(bool* oneshot) const;
    FMOD_RESULT getLength(std::int32_t* length) const;
    FMOD_RESULT getPath(char* path, std::int32_t size, std::int32_t* retrieved) const;
    FMOD_RESULT createInstance(EventInstance** instance) const;
};

class Bank {
public:
    FMOD_RESULT unload();
    FMOD_RESULT loadSampleData();
    FMOD_RESULT getLoadingState(FMOD_STUDIO_LOADING_STATE* state) const;
    FMOD_RESULT getSampleLoadingState(FMOD_STUDIO_LOADING_STATE* state) const;
    FMOD_RESULT getEventCount(std::int32_t* count) const;
    FMOD_RESULT getEventList(
        EventDescription** array,
        std::int32_t capacity,
        std::int32_t* count) const;
    FMOD_RESULT getBusCount(std::int32_t* count) const;
    FMOD_RESULT getBusList(
        Bus** array,
        std::int32_t capacity,
        std::int32_t* count) const;
};

class System {
public:
    static FMOD_RESULT create(System** system, std::uint32_t headerversion);
    FMOD_RESULT getUserData(void** userdata) const;
    FMOD_RESULT setUserData(void* userdata);
    FMOD_RESULT getLowLevelSystem(FMOD::System** system) const;
    FMOD_RESULT getAdvancedSettings(FMOD_STUDIO_ADVANCEDSETTINGS* settings);
    FMOD_RESULT setAdvancedSettings(FMOD_STUDIO_ADVANCEDSETTINGS* settings);
    FMOD_RESULT initialize(
        std::int32_t maxchannels,
        FMOD_STUDIO_INITFLAGS studioflags,
        FMOD_INITFLAGS flags,
        void* extradriverdata);
    FMOD_RESULT release();
    FMOD_RESULT registerPlugin(const FMOD_DSP_DESCRIPTION* description);
    FMOD_RESULT getEvent(const char* path, EventDescription** event) const;
    FMOD_RESULT getSoundInfo(const char* key, FMOD_STUDIO_SOUND_INFO* info) const;
    FMOD_RESULT getBus(const char* path, Bus** bus) const;
    FMOD_RESULT lookupID(const char* path, FMOD_GUID* id) const;
    FMOD_RESULT loadBankFile(
        const char* filename,
        std::uint32_t flags,
        Bank** bank);
    FMOD_RESULT flushCommands();
    FMOD_RESULT setListenerAttributes(
        std::int32_t listener,
        const FMOD_3D_ATTRIBUTES* attributes);
    bool isValid() const;
    FMOD_RESULT update();
};

}  // namespace Studio
}  // namespace FMOD
