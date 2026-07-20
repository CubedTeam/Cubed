#include "Cubed/audio/audio_engine.hpp"

#include "Cubed/audio/audio_error.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/gameplay/network_client.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"

#include <stdexcept>
using namespace google::protobuf;
namespace {
constexpr std::size_t OPUS_MAX_PACKET_SIZE = 400;
}
namespace Cubed {
AudioEngine::AudioEngine(Config& config)
    : m_recording(*this), m_config(config) {};

AudioEngine::~AudioEngine() {
    if (!m_init) {
        return;
    }

    m_bgm.reset();
    m_pool.reset();
    m_sounds.clear();
    m_voice_source.reset();
    m_low_pass_filter.reset();
    m_underwater_effect.reset();
    m_underwater_slot.reset();

    opus_encoder_destroy(m_encoder);
    opus_decoder_destroy(m_decoder);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void AudioEngine::init() {
    {
        int error;
        m_encoder = opus_encoder_create(AudioRecording::SAMPLE_RATE,
                                        1, // Mono
                                        OPUS_APPLICATION_VOIP, &error);

        if (error != OPUS_OK) {
            Logger::error("Can't Create Opus Encoder Error {}", error);
        }
        opus_encoder_ctl(m_encoder, OPUS_SET_BITRATE(24000));
        opus_encoder_ctl(m_encoder, OPUS_SET_COMPLEXITY(5));
        opus_encoder_ctl(m_encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    }
    {
        int error;
        m_decoder = opus_decoder_create(AudioRecording::SAMPLE_RATE, 1, &error);

        if (error != OPUS_OK) {
            Logger::error("Can't Create Opus Encoder Error {}", error);
        }
    }
    device = alcOpenDevice(NULL);
    if (!device) {
        throw std::runtime_error("Failed to open OpenAL device.");
    }
    context = alcCreateContext(device, nullptr);

    if (!context) {
        throw std::runtime_error("Failed to create OpenAL context.");
    }
    alcMakeContextCurrent(context);

    if (!alcIsExtensionPresent(device, "ALC_EXT_EFX")) {
        Logger::error("EFX not supported!");
        m_efx_supported = false;
    } else {
        Logger::info("EFX supported!");
        m_efx_supported = true;
    }

    if (m_efx_supported) {

        m_low_pass_filter = std::make_unique<AudioFilter>();

        m_low_pass_filter->set_lowpass(1.0f, 0.1f);

        m_underwater_effect = std::make_unique<AudioEffect>();
        m_underwater_effect->set_reverb(0.8f, 0.3162f, 0.01f);
        // m_underwater_effect->set_reverb(20.0f, 1.0f, 1.0f);
        m_underwater_slot = std::make_unique<AudioEffectSlot>();
        m_underwater_slot->set_effect(*m_underwater_effect);
    }

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    check_al_error();
    m_recording.init();
    m_music_volume = m_config.get("volume.music", 1.0f);
    m_sfx_volume = m_config.get("volume.SFX", 1.0f);
    m_player_voice_volume = m_config.get("volume.player_voice", 1.0f);
    m_sounds.init();

    m_bgm = std::make_unique<AudioSource>(m_music_volume);

    m_bgm->set_buffer_2d(m_sounds.get_buffer("bgm/bgm001.mp3"));

    m_fade_map.try_emplace("bgm", m_bgm.get(), 5.0f, 2.0f);
    m_voice_source = std::make_unique<AudioStreamSource>();
    m_voice_source->set_volume(m_player_voice_volume);
    ALCint max_mono = 0;

    alcGetIntegerv(device, ALC_MONO_SOURCES, 1, &max_mono);

    if (max_mono <= 1) {
        Logger::error("Can't get max mono");
        max_mono = 4;
    }

    Logger::info("Set Source Pool Size {}", static_cast<int>(max_mono));
    // Reserve two sources for BGM and voice
    m_pool = std::make_shared<SourcePool>(max_mono - 2);

    Logger::info("Audio Engine Init Success");

    m_init = true;
}

void AudioEngine::play_bgm() { m_bgm->play(); }
void AudioEngine::stop_bgm() { m_bgm->stop(); }
void AudioEngine::change_bgm(const std::string& sound) {
    Logger::info("change bgm {}", sound);
    m_bgm->stop();
    m_bgm->reset();
    m_bgm->set_buffer_2d(m_sounds.get_buffer(sound));
    m_bgm->set_target_volume(m_music_volume);
    m_bgm->set_volume(m_music_volume);
    if (m_efx_supported && m_underwater) {
        m_bgm->set_filter(*m_low_pass_filter);
    }
    auto it = m_fade_map.find("bgm");
    if (it != m_fade_map.end()) {
        it->second.reset();
    }
    m_bgm->play();
};

void AudioEngine::play_3d(const std::string& sound, const glm::vec3& pos,
                          bool check) {
    if (!m_pool) {
        Logger::error("Source Pool is nullptr");
        return;
    }
    auto* source = m_pool->acquire();
    source->set_volume(m_sfx_volume);
    if (!source) {
        Logger::error("Source is Full");
    }

    try {
        auto& buffer = m_sounds.get_buffer(sound);
        if (m_efx_supported && m_underwater) {
            source->set_filter(*m_low_pass_filter);
            source->set_effect_slot(*m_underwater_slot);
        }
        source->play_3d(buffer, pos);
    } catch (const std::exception& e) {
        if (check) {
            ASSERT_MSG(false, e.what());
            Logger::error("Player Sound Error {}", e.what());
        }
    }
}

void AudioEngine::play_2d(const std::string& sound, bool check) {
    if (!m_pool) {
        Logger::error("Source Pool is nullptr");
        return;
    }
    auto* source = m_pool->acquire();
    if (!source) {
        Logger::error("Source is Full");
    }
    source->set_volume(m_sfx_volume);

    try {
        auto& buffer = m_sounds.get_buffer(sound);
        if (m_efx_supported && m_underwater) {
            source->set_filter(*m_low_pass_filter);
            source->set_effect_slot(*m_underwater_slot);
        }
        source->play_2d(buffer);
    } catch (const std::exception& e) {
        if (check) {
            ASSERT_MSG(false, e.what());
            Logger::error("Player Sound Error {}", e.what());
        }
    }
}

void AudioEngine::update_listener(const glm::vec3& pos,
                                  const glm::vec3& forward,
                                  const glm::vec3& up) {
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);

    float orientation[] = {forward.x, forward.y, forward.z,

                           up.x,      up.y,      up.z};

    alListenerfv(AL_ORIENTATION, orientation);
}
void AudioEngine::update() {

    for (auto& [key, fade] : m_fade_map) {
        fade.update();
    }
    m_pool->update();
    m_recording.update();
}

void AudioEngine::reload_config() {
    m_music_volume = m_config.get("volume.music", 1.0f);
    m_sfx_volume = m_config.get("volume.SFX", 1.0f);
    m_player_voice_volume = m_config.get("volume.player_voice", 1.0f);
    if (m_bgm) {
        m_bgm->set_target_volume(m_music_volume);
    }
    if (m_voice_source) {
        m_voice_source->set_volume(m_player_voice_volume);
    }
}

void AudioEngine::underwater_change(bool underwater) {
    m_underwater = underwater;
    if (!m_efx_supported) {
        return;
    }
    if (!m_pool) {
        return;
    }
    for (auto& source : m_pool->sources()) {
        if (m_underwater) {
            source->set_filter(*m_low_pass_filter);
            source->set_effect_slot(*m_underwater_slot);
        } else {
            source->clear_filter();
            source->clear_effect_slot();
        }
    }
    if (underwater) {
        m_bgm->set_filter(*m_low_pass_filter);
        m_voice_source->set_filter(*m_low_pass_filter);
        m_voice_source->set_effect_slot(*m_underwater_slot);

    } else {
        m_bgm->clear_filter();
        m_voice_source->clear_filter();
        m_voice_source->clear_effect_slot();
    }
    Logger::info("Under Water Change {}", m_underwater);
}

float& AudioEngine::bgm_target_volume() { return m_bgm->target_volume(); }

void AudioEngine::set_client(std::weak_ptr<NetworkClient> client) {
    m_client = client;
}

void AudioEngine::send_voice(
    const std::array<int16_t, AudioRecording::FRAME_SAMPLES>& pcm) {

    std::array<uint8_t, OPUS_MAX_PACKET_SIZE> opus;
    int len = opus_encode(m_encoder, pcm.data(), AudioRecording::FRAME_SAMPLES,
                          opus.data(), opus.size());

    if (len < 0) {
        Logger::error("Opus encode failed: {}", opus_strerror(len));
        return;
    }
    if (auto c = m_client.lock()) {
        Arena arena;
        auto msg = Arena::Create<VoiceMsg>(&arena);
        msg->set_uuid(c->world().get_player().get_uuid());
        msg->set_opus_data(reinterpret_cast<char*>(opus.data()), len);
        auto* pos = msg->mutable_pos();
        auto p = c->world().get_player().get_player_pos();
        pos->set_x(p.x);
        pos->set_y(p.y);
        pos->set_z(p.z);
        c->send(make_packet(*msg));
    }
}
void AudioEngine::receive_voice(std::span<const char> opus,
                                const glm::vec3& pos) {
    std::array<int16_t, AudioRecording::FRAME_SAMPLES> pcm;
    int len =
        opus_decode(m_decoder, reinterpret_cast<const uint8_t*>(opus.data()),
                    opus.size(), pcm.data(), AudioRecording::FRAME_SAMPLES, 0);
    if (len < 0) {
        Logger::error("Opus decode failed: {}", opus_strerror(len));
        return;
    }
    m_voice_source->set_pos(pos);
    m_voice_source->push_pcm(std::span(pcm.data(), len),
                             AudioRecording::SAMPLE_RATE);
}

AudioRecording& AudioEngine::audio_recording() { return m_recording; }
const AudioRecording& AudioEngine::audio_recording() const {
    return m_recording;
}
} // namespace Cubed