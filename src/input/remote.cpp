#include "remote.h"

namespace input {

Action RemoteInput::poll() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quit_ = true;
                return Action::Quit;

            case SDL_KEYDOWN:
                return map_key(event.key.keysym.sym);

            default:
                break;
        }
    }
    return Action::None;
}

Action RemoteInput::map_key(SDL_Keycode key) {
    switch (key) {
        // D-pad / Arrow keys
        case SDLK_UP:      return Action::Up;
        case SDLK_DOWN:    return Action::Down;
        case SDLK_LEFT:    return Action::Left;
        case SDLK_RIGHT:   return Action::Right;

        // OK / Select
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            return Action::Select;

        // Back
        case SDLK_ESCAPE:
        case SDLK_BACKSPACE:
            return Action::Back;

        // Media controls (webOS remote sends these)
        case SDLK_SPACE:
            return Action::PlayPause;

        // Quit
        case SDLK_q:
            quit_ = true;
            return Action::Quit;

        default:
            return Action::None;
    }
}

} // namespace input
