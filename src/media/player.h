#pragma once

#include <string>
#include <functional>

namespace media {

enum class PlayerState {
    Idle,
    Loading,
    Playing,
    Paused,
    Stopped,
};

class Player {
public:
    Player();
    ~Player();

    void play(const std::string& url);
    void pause();
    void resume();
    void stop();
    void seek(int offset_seconds);
    void toggle_pause();

    PlayerState state() const { return state_; }
    bool is_playing() const { return state_ == PlayerState::Playing; }

    void set_on_stopped(std::function<void()> cb) { on_stopped_ = cb; }

private:
#ifdef WEBOS_PLATFORM
    void luna_call(const std::string& uri, const std::string& payload);
    std::string media_id_;
#endif
    PlayerState state_ = PlayerState::Idle;
    std::string current_url_;
    std::function<void()> on_stopped_;
};

} // namespace media
