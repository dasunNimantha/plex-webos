#pragma once

#include <string>
#include <vector>
#include <memory>

#include "plex/client.h"
#include "plex/auth.h"
#include "plex/models.h"
#include "ui/renderer.h"
#include "ui/text.h"
#include "ui/image_cache.h"
#include "input/remote.h"
#include "media/player.h"

enum class Screen {
    Login,
    ServerSelect,
    Home,
    Grid,
    Detail,
    Playing,
};

struct FocusState {
    int row = 0;
    int col = 0;
    int max_cols = 1;
    int max_rows = 1;

    void move_up()    { if (row > 0) { row--; col = 0; } }
    void move_down()  { if (row < max_rows - 1) { row++; col = 0; } }
    void move_left()  { if (col > 0) col--; }
    void move_right() { if (col < max_cols - 1) col++; }
    void reset()      { row = 0; col = 0; }
};

class App {
public:
    App();
    ~App();

    bool init();
    void run();
    void shutdown();

private:
    void handle_input(input::Action action);
    void update();
    void render();

    // Screen renderers
    void render_login();
    void render_server_select();
    void render_home();
    void render_grid();
    void render_detail();
    void render_playing();

    // Screen input handlers
    void handle_login_input(input::Action action);
    void handle_server_select_input(input::Action action);
    void handle_home_input(input::Action action);
    void handle_grid_input(input::Action action);
    void handle_detail_input(input::Action action);
    void handle_playing_input(input::Action action);

    // Navigation helpers
    void go_to_screen(Screen s);
    void go_back();
    void load_home();
    void load_library(const std::string& key);
    void load_detail(const plex::MediaItem& item);
    void play_item(const plex::MediaItem& item);
    void select_server(int index);

    // Session persistence
    void save_session();
    bool load_session();
    std::string config_path() const;

    ui::Renderer renderer_;
    ui::TextRenderer text_;
    ui::ImageCache images_;
    input::RemoteInput input_;
    media::Player player_;
    plex::PlexClient plex_;

    Screen screen_ = Screen::Login;
    Screen prev_screen_ = Screen::Login;
    FocusState focus_;

    // Auth state
    std::unique_ptr<plex::PlexAuth> auth_;
    plex::PinResponse current_pin_;
    int pin_poll_timer_ = 0;

    // Server selection
    std::vector<plex::ServerInfo> servers_;
    int server_focus_ = 0;

    // Data
    std::vector<plex::Library> libraries_;
    std::vector<plex::Hub> hubs_;
    std::vector<plex::MediaItem> grid_items_;
    plex::MediaItem detail_item_;
    std::string current_library_title_;

    // Scroll state
    float scroll_y_ = 0;

    bool running_ = false;
};
