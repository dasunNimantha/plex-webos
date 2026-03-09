#include "player.h"
#include <cstdio>
#include <cstring>
#include <string>

#include <starfish-media-pipeline/StarfishMediaAPIs.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace media {

Player::Player() : api_(std::make_unique<StarfishMediaAPIs>("com.plex.webos")) {}

Player::~Player() {
    stop();
    api_.reset();
}

void Player::starfish_callback(int type, int64_t num_value, const char* str_value, void* user_data) {
    auto* self = static_cast<Player*>(user_data);
    self->handle_event(type, num_value, str_value);
}

void Player::handle_event(int type, int64_t num_value, const char* str_value) {
    switch (type) {
        case PF_EVENT_TYPE_STR_STATE_UPDATE__LOADCOMPLETED:
            fprintf(stderr, "Starfish: load completed\n");
            state_ = PlayerState::Loading;
            api_->Play();
            break;

        case PF_EVENT_TYPE_STR_STATE_UPDATE__PLAYING:
            fprintf(stderr, "Starfish: playing\n");
            state_ = PlayerState::Playing;
            break;

        case PF_EVENT_TYPE_STR_STATE_UPDATE__PAUSED:
            fprintf(stderr, "Starfish: paused\n");
            state_ = PlayerState::Paused;
            break;

        case PF_EVENT_TYPE_STR_STATE_UPDATE__ENDOFSTREAM:
            fprintf(stderr, "Starfish: end of stream\n");
            state_ = PlayerState::EndOfStream;
            if (on_eos_) on_eos_();
            if (on_stopped_) on_stopped_();
            break;

        case PF_EVENT_TYPE_STR_STATE_UPDATE__SEEKDONE:
            fprintf(stderr, "Starfish: seek done\n");
            break;

        case PF_EVENT_TYPE_INT_DURATION:
            duration_ms_ = num_value;
            fprintf(stderr, "Starfish: duration = %lld ms\n", (long long)num_value);
            break;

        case PF_EVENT_TYPE_STR_VIDEO_INFO:
            if (str_value) fprintf(stderr, "Starfish: video info: %s\n", str_value);
            break;

        case PF_EVENT_TYPE_STR_AUDIO_INFO:
            if (str_value) fprintf(stderr, "Starfish: audio info: %s\n", str_value);
            break;

        case PF_EVENT_TYPE_INT_ERROR:
            fprintf(stderr, "Starfish: error (int): %lld\n", (long long)num_value);
            state_ = PlayerState::Error;
            break;

        case PF_EVENT_TYPE_STR_ERROR:
            fprintf(stderr, "Starfish: error: %s\n", str_value ? str_value : "(null)");
            state_ = PlayerState::Error;
            break;

        default:
            break;
    }
}

void Player::play(const std::string& url) {
    if (state_ == PlayerState::Playing || state_ == PlayerState::Loading) {
        stop();
    }

    current_url_ = url;
    state_ = PlayerState::Loading;
    fprintf(stderr, "Player: loading %s\n", url.c_str());

    json payload = {
        {"uri", url},
        {"type", "media"},
        {"payload", {
            {"option", {
                {"appId", "com.plex.webos"},
                {"windowId", ""},
                {"mediaTransportType", "URI"}
            }}
        }}
    };

    std::string payload_str = payload.dump();
    fprintf(stderr, "Player: Load payload: %s\n", payload_str.c_str());

    bool ok = api_->Load(payload_str.c_str(), Player::starfish_callback, this);
    if (!ok) {
        fprintf(stderr, "Player: StarfishMediaAPIs::Load failed\n");
        state_ = PlayerState::Error;
        return;
    }

    fprintf(stderr, "Player: Load() returned true\n");
}

void Player::pause() {
    if (state_ != PlayerState::Playing) return;
    if (api_->Pause()) {
        state_ = PlayerState::Paused;
        fprintf(stderr, "Player: paused\n");
    }
}

void Player::resume() {
    if (state_ != PlayerState::Paused) return;
    if (api_->Play()) {
        state_ = PlayerState::Playing;
        fprintf(stderr, "Player: resumed\n");
    }
}

void Player::stop() {
    if (state_ == PlayerState::Idle || state_ == PlayerState::Stopped) return;

    api_->Unload();
    state_ = PlayerState::Stopped;
    current_url_.clear();
    duration_ms_ = 0;
    fprintf(stderr, "Player: stopped\n");
    if (on_stopped_) on_stopped_();
}

void Player::seek(int offset_seconds) {
    if (state_ != PlayerState::Playing && state_ != PlayerState::Paused) return;

    int64_t current = current_time_ms();
    int64_t target = current + (int64_t)offset_seconds * 1000;
    if (target < 0) target = 0;
    if (duration_ms_ > 0 && target > duration_ms_) target = duration_ms_;

    seek_to(target);
    fprintf(stderr, "Player: seek %+ds -> %lld ms\n", offset_seconds, (long long)target);
}

void Player::seek_to(int64_t position_ms) {
    if (state_ != PlayerState::Playing && state_ != PlayerState::Paused) return;
    std::string pos = std::to_string(position_ms);
    api_->Seek(pos.c_str());
}

void Player::toggle_pause() {
    if (state_ == PlayerState::Playing) pause();
    else if (state_ == PlayerState::Paused) resume();
}

void Player::set_volume(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    volume_ = percent;

    json vol = {{"volume", percent}};
    api_->setVolume(vol.dump().c_str());
}

int64_t Player::current_time_ms() const {
    return api_->getCurrentPlaytime();
}

} // namespace media
