#pragma once

#include <SDL2/SDL.h>

namespace input {

enum class Action {
    None,
    Up,
    Down,
    Left,
    Right,
    Select,     // OK/Enter
    Back,       // Back/Escape
    PlayPause,
    Stop,
    FastForward,
    Rewind,
    Quit,
};

class RemoteInput {
public:
    Action poll();
    bool should_quit() const { return quit_; }

private:
    Action map_key(SDL_Keycode key);
    bool quit_ = false;
};

} // namespace input
