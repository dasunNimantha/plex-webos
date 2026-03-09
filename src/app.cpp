#include "app.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

using json = nlohmann::json;

App::App() : images_(200) {}
App::~App() {}

std::string App::config_path() const {
    const char* home = getenv("HOME");
    if (!home) home = "/tmp";
    return std::string(home) + "/.config/plex-webos.json";
}

void App::save_session() {
    json j = {
        {"server_url", plex_.server_url()},
        {"token", plex_.token()},
        {"client_id", plex_.client_id()},
    };
    std::ofstream f(config_path());
    f << j.dump(2);
}

bool App::load_session() {
    std::ifstream f(config_path());
    if (!f.is_open()) return false;
    try {
        auto j = json::parse(f);
        auto url = j.value("server_url", "");
        auto token = j.value("token", "");
        if (url.empty() || token.empty()) return false;
        plex_.set_server(url, token);
        return true;
    } catch (...) {
        return false;
    }
}

bool App::init() {
#ifdef WEBOS_PLATFORM
    if (!renderer_.init(1920, 1080, "Plex"))
#else
    if (!renderer_.init(1280, 720, "Plex"))
#endif
        return false;

    if (!text_.init()) return false;

    images_.start_workers(2);

    player_.set_on_stopped([this]() {
        go_to_screen(prev_screen_);
    });

    if (load_session()) {
        try {
            libraries_ = plex_.get_libraries();
            go_to_screen(Screen::Home);
            load_home();
        } catch (...) {
            screen_ = Screen::Login;
        }
    }

    if (screen_ == Screen::Login) {
        auth_ = std::make_unique<plex::PlexAuth>(plex_.client_id());
        try {
            current_pin_ = auth_->request_pin();
        } catch (const std::exception& e) {
            fprintf(stderr, "PIN request failed: %s\n", e.what());
        }
    }

    return true;
}

void App::run() {
    running_ = true;
    auto frame_time = std::chrono::microseconds(1000000 / 60);

    while (running_) {
        auto start = std::chrono::steady_clock::now();

        input::Action action = input_.poll();
        if (input_.should_quit()) {
            running_ = false;
            break;
        }

        if (action != input::Action::None) {
            handle_input(action);
        }

        update();
        render();

        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed < frame_time) {
            std::this_thread::sleep_for(frame_time - elapsed);
        }
    }
}

void App::shutdown() {
    player_.stop();
    images_.stop_workers();
    text_.shutdown();
    renderer_.shutdown();
}

void App::handle_input(input::Action action) {
    if (action == input::Action::Quit) {
        running_ = false;
        return;
    }

    switch (screen_) {
        case Screen::Login:       handle_login_input(action); break;
        case Screen::ServerSelect: handle_server_select_input(action); break;
        case Screen::Home:        handle_home_input(action); break;
        case Screen::Grid:        handle_grid_input(action); break;
        case Screen::Detail:      handle_detail_input(action); break;
        case Screen::Playing:     handle_playing_input(action); break;
    }
}

void App::update() {
    images_.upload_pending(renderer_);

    if (screen_ == Screen::Login && current_pin_.id > 0) {
        pin_poll_timer_++;
        if (pin_poll_timer_ >= 120) { // Poll every ~2 seconds at 60fps
            pin_poll_timer_ = 0;
            try {
                auto result = auth_->check_pin(current_pin_.id);
                if (!result.auth_token.empty()) {
                    fprintf(stderr, "Auth token received! Fetching servers...\n");
                    servers_ = plex_.get_servers(result.auth_token);
                    fprintf(stderr, "Found %zu servers\n", servers_.size());
                    for (auto& s : servers_) {
                        fprintf(stderr, "  Server: %s at %s\n", s.name.c_str(), s.address.c_str());
                    }
                    if (servers_.size() == 1) {
                        plex_.set_server(servers_[0].address, servers_[0].access_token);
                        libraries_ = plex_.get_libraries();
                        save_session();
                        go_to_screen(Screen::Home);
                        load_home();
                    } else if (servers_.size() > 1) {
                        go_to_screen(Screen::ServerSelect);
                    } else {
                        fprintf(stderr, "No servers found!\n");
                    }
                }
            } catch (const std::exception& e) {
                fprintf(stderr, "Auth error: %s\n", e.what());
            }
        }
    }
}

void App::render() {
    renderer_.begin_frame();
    renderer_.clear(ui::colors::background);

    switch (screen_) {
        case Screen::Login:       render_login(); break;
        case Screen::ServerSelect: render_server_select(); break;
        case Screen::Home:        render_home(); break;
        case Screen::Grid:        render_grid(); break;
        case Screen::Detail:      render_detail(); break;
        case Screen::Playing:     render_playing(); break;
    }

    renderer_.end_frame();
}

void App::go_to_screen(Screen s) {
    prev_screen_ = screen_;
    screen_ = s;
    focus_.reset();
    scroll_y_ = 0;
}

void App::go_back() {
    switch (screen_) {
        case Screen::Detail:
        case Screen::Grid:
            go_to_screen(Screen::Home);
            break;
        case Screen::Playing:
            player_.stop();
            break;
        case Screen::ServerSelect:
            go_to_screen(Screen::Login);
            break;
        default:
            break;
    }
}

void App::load_home() {
    try {
        hubs_ = plex_.get_hubs();
    } catch (const std::exception& e) {
        fprintf(stderr, "load_home failed: %s\n", e.what());
    }
}

void App::load_library(const std::string& key) {
    try {
        grid_items_ = plex_.get_library_items(key);
        go_to_screen(Screen::Grid);
    } catch (const std::exception& e) {
        fprintf(stderr, "load_library failed: %s\n", e.what());
    }
}

void App::load_detail(const plex::MediaItem& item) {
    detail_item_ = item;
    go_to_screen(Screen::Detail);
}

void App::play_item(const plex::MediaItem& item) {
    auto pk = item.stream_part_key();
    if (pk.empty()) return;
    auto url = plex_.stream_url(pk);
    go_to_screen(Screen::Playing);
    player_.play(url);
}

void App::select_server(int index) {
    if (index < 0 || index >= (int)servers_.size()) return;
    plex_.set_server(servers_[index].address, servers_[index].access_token);
    try {
        libraries_ = plex_.get_libraries();
        save_session();
        go_to_screen(Screen::Home);
        load_home();
    } catch (const std::exception& e) {
        fprintf(stderr, "Server connect failed: %s\n", e.what());
    }
}

// ── Input handlers ──────────────────────────────────────────────

void App::handle_login_input(input::Action action) {
    if (action == input::Action::Back) {
        running_ = false;
    }
}

void App::handle_server_select_input(input::Action action) {
    switch (action) {
        case input::Action::Up:
            if (server_focus_ > 0) server_focus_--;
            break;
        case input::Action::Down:
            if (server_focus_ < (int)servers_.size() - 1) server_focus_++;
            break;
        case input::Action::Select:
            select_server(server_focus_);
            break;
        case input::Action::Back:
            go_back();
            break;
        default:
            break;
    }
}

void App::handle_home_input(input::Action action) {
    switch (action) {
        case input::Action::Up:
            focus_.move_up();
            break;
        case input::Action::Down:
            if (focus_.row < (int)hubs_.size() + (int)libraries_.size() - 1)
                focus_.row++;
            break;
        case input::Action::Left:
            focus_.move_left();
            break;
        case input::Action::Right:
            focus_.move_right();
            break;
        case input::Action::Select: {
            // First rows are libraries, then hubs
            int lib_count = (int)libraries_.size();
            if (focus_.row < lib_count) {
                current_library_title_ = libraries_[focus_.row].title;
                load_library(libraries_[focus_.row].key);
            } else {
                int hub_idx = focus_.row - lib_count;
                if (hub_idx < (int)hubs_.size()) {
                    auto& items = hubs_[hub_idx].items;
                    if (focus_.col < (int)items.size()) {
                        load_detail(items[focus_.col]);
                    }
                }
            }
            break;
        }
        case input::Action::Back:
            running_ = false;
            break;
        default:
            break;
    }
}

void App::handle_grid_input(input::Action action) {
    int cols = 7;
    int total = (int)grid_items_.size();
    int rows = (total + cols - 1) / cols;

    switch (action) {
        case input::Action::Up:
            if (focus_.row > 0) focus_.row--;
            break;
        case input::Action::Down:
            if (focus_.row < rows - 1) focus_.row++;
            break;
        case input::Action::Left:
            if (focus_.col > 0) focus_.col--;
            break;
        case input::Action::Right:
            if (focus_.col < cols - 1) focus_.col++;
            break;
        case input::Action::Select: {
            int idx = focus_.row * cols + focus_.col;
            if (idx < total) {
                load_detail(grid_items_[idx]);
            }
            break;
        }
        case input::Action::Back:
            go_back();
            break;
        default:
            break;
    }
}

void App::handle_detail_input(input::Action action) {
    switch (action) {
        case input::Action::Select:
            play_item(detail_item_);
            break;
        case input::Action::Back:
            go_back();
            break;
        default:
            break;
    }
}

void App::handle_playing_input(input::Action action) {
    switch (action) {
        case input::Action::PlayPause:
        case input::Action::Select:
            player_.toggle_pause();
            break;
        case input::Action::Stop:
        case input::Action::Back:
            player_.stop();
            break;
        case input::Action::FastForward:
        case input::Action::Right:
            player_.seek(10);
            break;
        case input::Action::Rewind:
        case input::Action::Left:
            player_.seek(-10);
            break;
        default:
            break;
    }
}
