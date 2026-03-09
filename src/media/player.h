#pragma once

#include <string>
#include <functional>
#include <memory>
#include <cstdint>

class StarfishMediaAPIs;

namespace media {

enum class PlayerState {
    Idle,
    Loading,
    Playing,
    Paused,
    Stopped,
    EndOfStream,
    Error,
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
    void seek_to(int64_t position_ms);
    void toggle_pause();
    void set_volume(int percent);

    PlayerState state() const { return state_; }
    bool is_playing() const { return state_ == PlayerState::Playing; }
    int64_t current_time_ms() const;
    int64_t duration_ms() const { return duration_ms_; }

    void set_on_stopped(std::function<void()> cb) { on_stopped_ = cb; }
    void set_on_end_of_stream(std::function<void()> cb) { on_eos_ = cb; }

    static void starfish_callback(int type, int64_t num_value, const char* str_value, void* user_data);

private:
    void handle_event(int type, int64_t num_value, const char* str_value);

    std::unique_ptr<StarfishMediaAPIs> api_;
    PlayerState state_ = PlayerState::Idle;
    std::string current_url_;
    int64_t duration_ms_ = 0;
    int volume_ = 100;

    std::function<void()> on_stopped_;
    std::function<void()> on_eos_;
};

} // namespace media
