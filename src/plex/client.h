#pragma once

#include <string>
#include <vector>
#include <functional>
#include "models.h"

namespace plex {

class PlexClient {
public:
    PlexClient();
    ~PlexClient();

    void set_server(const std::string& url, const std::string& token);
    bool is_connected() const { return !server_url_.empty() && !token_.empty(); }

    const std::string& server_url() const { return server_url_; }
    const std::string& token() const { return token_; }
    const std::string& client_id() const { return client_id_; }

    std::vector<Library> get_libraries();
    std::vector<MediaItem> get_library_items(const std::string& key);
    std::vector<Hub> get_hubs();
    std::vector<MediaItem> get_children(const std::string& rating_key);
    std::vector<MediaItem> search(const std::string& query);

    std::string stream_url(const std::string& part_key) const;
    std::string poster_url(const std::string& thumb, int w = 200, int h = 300) const;
    std::string art_url(const std::string& art, int w = 400, int h = 225) const;

    std::vector<ServerInfo> get_servers(const std::string& auth_token);

private:
    std::string http_get(const std::string& url);
    std::string http_post(const std::string& url, const std::string& body = "");
    std::string build_url(const std::string& path) const;
    void add_headers(void* curl) const;
    static std::string url_encode(const std::string& s);

    std::string server_url_;
    std::string token_;
    std::string client_id_;
};

} // namespace plex
