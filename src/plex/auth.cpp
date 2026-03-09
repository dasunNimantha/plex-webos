#include "auth.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

namespace plex {

static size_t write_cb(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

PlexAuth::PlexAuth(const std::string& client_id) : client_id_(client_id) {}

std::string PlexAuth::http_post(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl init failed");

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers,
        ("X-Plex-Client-Identifier: " + client_id_).c_str());
    headers = curl_slist_append(headers, "X-Plex-Product: Plex webOS");
    headers = curl_slist_append(headers, "X-Plex-Version: 0.1.0");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(std::string("PIN request failed: ") + curl_easy_strerror(res));
    return response;
}

std::string PlexAuth::http_get(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl init failed");

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers,
        ("X-Plex-Client-Identifier: " + client_id_).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(std::string("PIN check failed: ") + curl_easy_strerror(res));
    return response;
}

PinResponse PlexAuth::request_pin() {
    auto body = http_post("https://plex.tv/api/v2/pins?strong=false");
    auto j = json::parse(body);
    PinResponse pin;
    pin.id = j.value("id", 0);
    pin.code = j.value("code", "");
    return pin;
}

PinResponse PlexAuth::check_pin(int pin_id) {
    auto body = http_get("https://plex.tv/api/v2/pins/" + std::to_string(pin_id));
    auto j = json::parse(body);
    PinResponse pin;
    pin.id = j.value("id", 0);
    pin.code = j.value("code", "");
    if (j.contains("authToken") && j["authToken"].is_string()) {
        pin.auth_token = j["authToken"].get<std::string>();
    }
    fprintf(stderr, "PIN check: id=%d, authToken=%s\n",
            pin.id, pin.auth_token.empty() ? "(empty)" : "GOT TOKEN");
    return pin;
}

} // namespace plex
