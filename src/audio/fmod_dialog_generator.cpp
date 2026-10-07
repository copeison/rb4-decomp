#include "fmod_dialog_generator.h"

#include "fmod_audio_system.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kActiveHandleBit = 0x80000000;
constexpr std::uint32_t kGenerationMask = 0x00003FFF;
constexpr std::uint32_t kPoolIndexMask = 0x00FFC000;
constexpr std::uint32_t kPoolIndexShift = 14;
constexpr std::int32_t kDialogFormat = 4;
constexpr std::string_view kEventPrefix = "event:/";
constexpr FMOD_MODE kProgrammerSoundMode = 0x00014200;

}  // namespace

// Reconstructed from the path handling at eboot.elf 0x26EC60.
std::string fmod_dialog_event_path(std::string_view path) {
    if (path.size() >= kEventPrefix.size() &&
        path.substr(0, kEventPrefix.size()) == kEventPrefix) {
        return std::string(path);
    }
    return std::string(kEventPrefix) + std::string(path);
}

void FmodDialogGenerator::initialize_pool_slot(
    FmodDialogGeneratorManager& manager,
    std::uint32_t index) {
    manager_ = &manager;
    pool_index_ = index;
    reference_count_.store(0, std::memory_order_relaxed);
    handle_ = 0;
    sound_source_ = nullptr;
    audio_state_ = nullptr;
    length_ms_ = 3600000.0F;
    state_ = AudioClipFmodState::stopped;
}

// Reconstructed from eboot.elf at 0x26E990.
bool FmodDialogGenerator::initialize_event(
    const char* event_path,
    const FmodDialogGeneratorOptions& options,
    AudioClipFmodSpatialSource* spatial_source) {
    if (event_path == nullptr || options.format != kDialogFormat ||
        audio_state_ == nullptr) {
        return false;
    }

    length_ms_ = 3600000.0F;
    const auto normalized_path = fmod_dialog_event_path(event_path);
    const auto initialized = studio_generator_.initialize_event(
        normalized_path.c_str(),
        options.sound,
        *audio_state_,
        spatial_source,
        programmer_sound_callback,
        this);
    state_ = studio_generator_.state();
    return initialized;
}

void FmodDialogGenerator::pause() {
    studio_generator_.pause();
    state_ = studio_generator_.state();
}

void FmodDialogGenerator::resume() {
    studio_generator_.resume();
    state_ = studio_generator_.state();
}

void FmodDialogGenerator::prepare_for_audio_reset() {
    studio_generator_.prepare_for_audio_reset();
    state_ = studio_generator_.state();
}

void FmodDialogGenerator::stop_and_wait() {
    studio_generator_.stop_and_wait();
    state_ = studio_generator_.state();
}

bool FmodDialogGenerator::update() {
    const auto active = studio_generator_.update();
    state_ = studio_generator_.state();
    return active;
}

AudioClipFmodState FmodDialogGenerator::state() const {
    return studio_generator_.state();
}

float FmodDialogGenerator::position_ms() const {
    return studio_generator_.position_ms();
}

float FmodDialogGenerator::channel_position_ms() const {
    return studio_generator_.channel_position_ms();
}

float FmodDialogGenerator::length_ms() const {
    return length_ms_;
}

void FmodDialogGenerator::set_position_ms(float position) {
    studio_generator_.set_position_ms(position);
}

bool FmodDialogGenerator::set_event_parameter(
    const char* name,
    float value) {
    return studio_generator_.set_event_parameter(name, value);
}

bool FmodDialogGenerator::get_event_parameter(
    const char* name,
    float& value) const {
    return studio_generator_.get_event_parameter(name, value);
}

void FmodDialogGenerator::configure_fade(
    std::int32_t completion_mode,
    float target,
    float duration_seconds) {
    studio_generator_.configure_fade(
        completion_mode, target, duration_seconds);
}

float FmodDialogGenerator::fade_value() const {
    return studio_generator_.fade_value();
}

void FmodDialogGenerator::configure_volume_transition(
    bool fade_out,
    bool immediate) {
    studio_generator_.configure_volume_transition(fade_out, immediate);
}

bool FmodDialogGenerator::volume_transition_requested() const {
    return studio_generator_.volume_transition_requested();
}

// Reconstructed from eboot.elf at 0x26EA10.
FMOD_RESULT FmodDialogGenerator::programmer_sound_callback(
    FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
    FMOD::Studio::EventInstance* event_instance,
    void* parameters) {
    void* user_data = nullptr;
    event_instance->getUserData(&user_data);
    auto* generator = static_cast<FmodDialogGenerator*>(user_data);
    if (generator == nullptr) {
        return FMOD_OK;
    }

    if (generator->state() == AudioClipFmodState::stopped ||
        generator->state() == AudioClipFmodState::stopping) {
        generator->studio_generator_.mark_event_callback_complete();
        return FMOD_OK;
    }

    if (type == FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND) {
        auto* properties =
            static_cast<FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES*>(parameters);
        return properties != nullptr
            ? generator->create_programmer_sound(*properties)
            : FMOD_ERR_INVALID_PARAM;
    }
    if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND) {
        auto* properties =
            static_cast<FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES*>(parameters);
        return properties != nullptr && properties->sound != nullptr
            ? properties->sound->release()
            : FMOD_OK;
    }
    if (type == FMOD_STUDIO_EVENT_CALLBACK_SOUND_PLAYED) {
        auto* sound = static_cast<FMOD::Sound*>(parameters);
        std::uint32_t length = 0;
        if (sound != nullptr &&
            sound->getLength(&length, FMOD_TIMEUNIT_MS) == FMOD_OK) {
            generator->length_ms_ = static_cast<float>(length);
        }
    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_STARTED) {
        generator->studio_generator_.notify_event_started();
    } else if (type == FMOD_STUDIO_EVENT_CALLBACK_DESTROYED ||
               type == FMOD_STUDIO_EVENT_CALLBACK_STOPPED) {
        generator->studio_generator_.mark_event_callback_complete();
    }
    return FMOD_OK;
}

FMOD_RESULT FmodDialogGenerator::create_programmer_sound(
    FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES& properties) {
    if (audio_state_ == nullptr || audio_state_->studio_system == nullptr ||
        audio_state_->core_system == nullptr || properties.name == nullptr) {
        return FMOD_ERR_INVALID_PARAM;
    }

    FMOD_STUDIO_SOUND_INFO sound_info{};
    const auto info_result = audio_state_->studio_system->getSoundInfo(
        properties.name, &sound_info);
    if (info_result != FMOD_OK) {
        return info_result;
    }

    FMOD::Sound* sound = nullptr;
    const auto create_result = audio_state_->core_system->createSound(
        sound_info.name_or_data,
        sound_info.mode | kProgrammerSoundMode,
        sound_info.create_sound_info,
        &sound);
    if (create_result != FMOD_OK) {
        return create_result;
    }

    properties.sound = sound;
    properties.subsound_index = sound_info.subsound_index;
    return FMOD_OK;
}

FmodDialogGeneratorManager::FmodDialogGeneratorManager(
    std::size_t capacity,
    FmodAudioState* default_audio_state)
    : capacity_(capacity), default_audio_state_(default_audio_state) {}

// Reconstructed from eboot.elf at 0x26EE80.
std::int32_t FmodDialogGeneratorManager::setting() const {
    return setting_;
}

// Reconstructed from eboot.elf at 0x26F270.
void FmodDialogGeneratorManager::set_setting(std::int32_t value) {
    setting_ = value;
}

// Reconstructed from the resource gate at eboot.elf 0x26EC60.
bool FmodDialogGeneratorManager::accepts(
    const FmodDialogGeneratorOptions& options) const {
    return options.format == kDialogFormat;
}

// Reconstructed from eboot.elf at 0x26F280.
void FmodDialogGeneratorManager::initialize_pool() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    generators_ = std::make_unique<FmodDialogGenerator[]>(capacity_);
    free_indices_.clear();
    for (std::uint32_t index = 0; index < capacity_; ++index) {
        generators_[index].initialize_pool_slot(*this, index);
        generators_[index].in_free_list_ = true;
        free_indices_.push_back(index);
    }

    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26F420.
bool FmodDialogGeneratorManager::shutdown_pool() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    if (free_indices_.size() != capacity_) {
        --lock_depth_;
        return false;
    }

    generators_.reset();
    free_indices_.clear();
    --lock_depth_;
    return true;
}

// Reconstructed from eboot.elf at 0x26EFD0.
FmodDialogGenerator* FmodDialogGeneratorManager::retain(
    FmodDialogGeneratorHandle handle,
    std::uint32_t index) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;

    FmodDialogGenerator* result = nullptr;
    if (generators_ != nullptr && index < capacity_) {
        auto& generator = generators_[index];
        if (generator.handle_ == handle) {
            generator.reference_count_.fetch_add(
                1, std::memory_order_relaxed);
            result = &generator;
        }
    }

    --lock_depth_;
    return result;
}

FmodDialogGenerator* FmodDialogGeneratorManager::acquire(
    FmodAudioState* audio_state,
    void* sound_source) {
    if (audio_state == nullptr) {
        audio_state = default_audio_state_;
    }

    std::lock_guard lock(mutex_);
    ++lock_depth_;
    if (free_indices_.empty()) {
        --lock_depth_;
        return nullptr;
    }

    const auto index = free_indices_.front();
    free_indices_.pop_front();
    auto& generator = generators_[index];
    generator.in_free_list_ = false;
    generator.sound_source_ = sound_source;
    generator.audio_state_ = audio_state;
    generator.handle_ = activate_handle(generator);
    generator.state_ = AudioClipFmodState::uninitialized;

    --lock_depth_;
    return &generator;
}

// Reconstructed from eboot.elf at 0x26EBE0.
void FmodDialogGeneratorManager::release(
    FmodDialogGenerator& generator) {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    generator.handle_ &= ~kActiveHandleBit;
    generator.sound_source_ = nullptr;
    generator.audio_state_ = nullptr;
    generator.studio_generator_.release_event();
    generator.state_ = AudioClipFmodState::stopped;
    if (!generator.in_free_list_) {
        generator.in_free_list_ = true;
        free_indices_.push_back(generator.pool_index_);
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26F040.
void FmodDialogGeneratorManager::prepare_all_for_audio_reset() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].prepare_for_audio_reset();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26F0B0.
void FmodDialogGeneratorManager::stop_all() {
    std::lock_guard lock(mutex_);
    ++lock_depth_;
    for (std::size_t index = 0; index < capacity_; ++index) {
        generators_[index].stop_and_wait();
    }
    --lock_depth_;
}

// Reconstructed from eboot.elf at 0x26F120.
std::vector<FmodDialogGeneratorHandle>
FmodDialogGeneratorManager::active_handles() const {
    std::vector<FmodDialogGeneratorHandle> handles;
    handles.reserve(capacity_ - free_indices_.size());
    for (std::size_t index = 0; index < capacity_; ++index) {
        const auto handle = generators_[index].handle_;
        if (static_cast<std::int32_t>(handle) < 0) {
            handles.push_back(handle);
        }
    }
    return handles;
}

FmodDialogGeneratorHandle FmodDialogGeneratorManager::activate_handle(
    FmodDialogGenerator& generator) const {
    const auto generation = (generator.handle_ + 1) & kGenerationMask;
    const auto pool_index =
        (generator.pool_index_ << kPoolIndexShift) & kPoolIndexMask;
    return kActiveHandleBit | pool_index | generation;
}

}  // namespace rb4
