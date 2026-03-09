#include "client.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <random>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

namespace plex {

static size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

static std::string generate_client_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    const char hex[] = "0123456789abcdef";
    std::string id;
    id.reserve(32);
    for (int i = 0; i < 32; ++i) id += hex[dis(gen)];
    return id;
}

PlexClient::PlexClient() : client_id_(generate_client_id()) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

PlexClient::~PlexClient() {
    curl_global_cleanup();
}

void PlexClient::set_server(const std::string& url, const std::string& token) {
    server_url_ = url;
    token_ = token;
    // Strip trailing slash
    while (!server_url_.empty() && server_url_.back() == '/')
        server_url_.pop_back();
}

void PlexClient::add_headers(void* curl) const {
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers,
        ("X-Plex-Client-Identifier: " + client_id_).c_str());
    headers = curl_slist_append(headers, "X-Plex-Product: Plex webOS");
    headers = curl_slist_append(headers, "X-Plex-Version: 0.1.0");
    headers = curl_slist_append(headers, "X-Plex-Platform: webOS");
    if (!token_.empty()) {
        headers = curl_slist_append(headers,
            ("X-Plex-Token: " + token_).c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
}

std::string PlexClient::http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    add_headers(curl);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("HTTP GET failed: ") + curl_easy_strerror(res));
    }
    return response;
}

std::string PlexClient::http_post(const std::string& url, const std::string& body) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    add_headers(curl);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("HTTP POST failed: ") + curl_easy_strerror(res));
    }
    return response;
}

std::string PlexClient::build_url(const std::string& path) const {
    return server_url_ + path;
}

std::string PlexClient::url_encode(const std::string& s) {
    CURL* curl = curl_easy_init();
    if (!curl) return s;
    char* encoded = curl_easy_escape(curl, s.c_str(), static_cast<int>(s.length()));
    std::string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return result;
}

std::vector<Library> PlexClient::get_libraries() {
    auto body = http_get(build_url("/library/sections"));
    auto j = json::parse(body);
    std::vector<Library> libs;
    auto& mc = j["MediaContainer"];
    if (mc.contains("Directory") && mc["Directory"].is_array()) {
        libs = mc["Directory"].get<std::vector<Library>>();
    }
    return libs;
}

std::vector<MediaItem> PlexClient::get_library_items(const std::string& key) {
    auto body = http_get(build_url("/library/sections/" + key + "/all"));
    auto j = json::parse(body);
    std::vector<MediaItem> items;
    auto& mc = j["MediaContainer"];
    if (mc.contains("Metadata") && mc["Metadata"].is_array()) {
        items = mc["Metadata"].get<std::vector<MediaItem>>();
    }
    return items;
}

std::vector<Hub> PlexClient::get_hubs() {
    auto body = http_get(build_url("/hubs"));
    auto j = json::parse(body);
    std::vector<Hub> hubs;
    auto& mc = j["MediaContainer"];
    if (mc.contains("Hub") && mc["Hub"].is_array()) {
        hubs = mc["Hub"].get<std::vector<Hub>>();
    }
    return hubs;
}

std::vector<MediaItem> PlexClient::get_children(const std::string& rating_key) {
    auto body = http_get(build_url("/library/metadata/" + rating_key + "/children"));
    auto j = json::parse(body);
    std::vector<MediaItem> items;
    auto& mc = j["MediaContainer"];
    if (mc.contains("Metadata") && mc["Metadata"].is_array()) {
        items = mc["Metadata"].get<std::vector<MediaItem>>();
    }
    return items;
}

std::vector<MediaItem> PlexClient::search(const std::string& query) {
    auto body = http_get(build_url("/hubs/search?query=" + url_encode(query)));
    auto j = json::parse(body);
    std::vector<MediaItem> results;
    auto& mc = j["MediaContainer"];
    if (mc.contains("Hub") && mc["Hub"].is_array()) {
        for (auto& hub : mc["Hub"]) {
            if (hub.contains("Metadata") && hub["Metadata"].is_array()) {
                auto items = hub["Metadata"].get<std::vector<MediaItem>>();
                results.insert(results.end(), items.begin(), items.end());
            }
        }
    }
    return results;
}

std::string PlexClient::stream_url(const std::string& part_key) const {
    return server_url_ + part_key + "?X-Plex-Token=" + token_;
}

std::string PlexClient::poster_url(const std::string& thumb, int w, int h) const {
    return server_url_ + "/photo/:/transcode?width=" + std::to_string(w)
        + "&height=" + std::to_string(h) + "&minSize=1&upscale=1"
        + "&url=" + url_encode(thumb)
        + "&X-Plex-Token=" + token_;
}

std::string PlexClient::art_url(const std::string& art, int w, int h) const {
    return poster_url(art, w, h);
}

std::vector<ServerInfo> PlexClient::get_servers(const std::string& auth_token) {
    std::string saved = token_;
    token_ = auth_token;
    auto body = http_get("https://plex.tv/api/v2/resources?includeHttps=1&includeRelay=1");
    token_ = saved;

    auto j = json::parse(body);
    std::vector<ServerInfo> servers;
    for (auto& res : j) {
        if (res.value("provides", "") != "server") continue;
        ServerInfo si;
        si.name = res.value("name", "");
        si.access_token = res.value("accessToken", "");
        if (res.contains("connections") && res["connections"].is_array()) {
            // Prefer direct local HTTP connection (fastest, no DNS/TLS issues)
            std::string local_http, local_any, remote_any;
            for (auto& conn : res["connections"]) {
                std::string uri = conn.value("uri", "");
                bool is_local = conn.value("local", false);
                fprintf(stderr, "  Connection: %s (local=%d)\n", uri.c_str(), is_local);

                if (is_local && uri.find("http://") == 0) {
                    local_http = uri;
                } else if (is_local && local_any.empty()) {
                    local_any = uri;
                } else if (!is_local && remote_any.empty()) {
                    remote_any = uri;
                }
            }
            if (!local_http.empty()) si.address = local_http;
            else if (!local_any.empty()) si.address = local_any;
            else si.address = remote_any;
        }
        fprintf(stderr, "  -> Using: %s\n", si.address.c_str());
        if (!si.address.empty()) servers.push_back(si);
    }
    return servers;
}

} // namespace plex
