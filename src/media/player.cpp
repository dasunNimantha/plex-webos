#include "player.h"
#include <cstdio>

#ifdef WEBOS_PLATFORM
#include <luna-service2/lunaservice.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

namespace media {

Player::Player() {}
Player::~Player() { stop(); }

void Player::play(const std::string& url) {
    current_url_ = url;
    state_ = PlayerState::Loading;
    printf("Player: loading %s\n", url.c_str());

#ifdef WEBOS_PLATFORM
    json payload = {
        {"uri", url},
        {"type", "media"},
        {"payload", {
            {"option", {
                {"appId", "com.plex.webos"},
                {"windowId", ""}
            }},
            {"mediaTransportType", "URI"}
        }}
    };
    luna_call("luna://com.webos.media/load", payload.dump());
#endif
    state_ = PlayerState::Playing;
    printf("Player: playing\n");
}

void Player::pause() {
    if (state_ != PlayerState::Playing) return;
    state_ = PlayerState::Paused;
#ifdef WEBOS_PLATFORM
    json payload = {{"mediaId", media_id_}};
    luna_call("luna://com.webos.media/pause", payload.dump());
#endif
    printf("Player: paused\n");
}

void Player::resume() {
    if (state_ != PlayerState::Paused) return;
    state_ = PlayerState::Playing;
#ifdef WEBOS_PLATFORM
    json payload = {{"mediaId", media_id_}};
    luna_call("luna://com.webos.media/play", payload.dump());
#endif
    printf("Player: resumed\n");
}

void Player::stop() {
    if (state_ == PlayerState::Idle || state_ == PlayerState::Stopped) return;
#ifdef WEBOS_PLATFORM
    json payload = {{"mediaId", media_id_}};
    luna_call("luna://com.webos.media/unload", payload.dump());
#endif
    state_ = PlayerState::Stopped;
    current_url_.clear();
    printf("Player: stopped\n");
    if (on_stopped_) on_stopped_();
}

void Player::seek(int offset_seconds) {
    if (state_ != PlayerState::Playing && state_ != PlayerState::Paused) return;
#ifdef WEBOS_PLATFORM
    json payload = {
        {"mediaId", media_id_},
        {"position", offset_seconds * 1000}
    };
    luna_call("luna://com.webos.media/seek", payload.dump());
#endif
    printf("Player: seek %+ds\n", offset_seconds);
}

void Player::toggle_pause() {
    if (state_ == PlayerState::Playing) pause();
    else if (state_ == PlayerState::Paused) resume();
}

#ifdef WEBOS_PLATFORM
void Player::luna_call(const std::string& uri, const std::string& payload) {
    // Luna Service Bus call - requires luna-service2 library
    // This is a simplified version; production code needs proper error handling
    LSError error;
    LSErrorInit(&error);
    LSHandle* handle = nullptr;

    if (!LSRegister("com.plex.webos", &handle, &error)) {
        fprintf(stderr, "LSRegister failed: %s\n", error.message);
        LSErrorFree(&error);
        return;
    }

    LSCall(handle, uri.c_str(), payload.c_str(), nullptr, nullptr, nullptr, &error);
    LSUnregister(handle, &error);
}
#endif

} // namespace media
