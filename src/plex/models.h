#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace plex {

struct Tag {
    std::string tag;
};

inline void from_json(const nlohmann::json& j, Tag& t) {
    t.tag = j.value("tag", "");
}

struct Part {
    std::string key;
    std::string file;
    std::string container;
};

inline void from_json(const nlohmann::json& j, Part& p) {
    p.key = j.value("key", "");
    p.file = j.value("file", "");
    p.container = j.value("container", "");
}

struct Media {
    int64_t duration = 0;
    int64_t bitrate = 0;
    int width = 0, height = 0;
    std::string video_codec;
    std::string audio_codec;
    std::string video_resolution;
    std::string container;
    std::vector<Part> parts;
};

inline void from_json(const nlohmann::json& j, Media& m) {
    m.duration = j.value("duration", int64_t(0));
    m.bitrate = j.value("bitrate", int64_t(0));
    m.width = j.value("width", 0);
    m.height = j.value("height", 0);
    m.video_codec = j.value("videoCodec", "");
    m.audio_codec = j.value("audioCodec", "");
    m.video_resolution = j.value("videoResolution", "");
    m.container = j.value("container", "");
    if (j.contains("Part") && j["Part"].is_array()) {
        m.parts = j["Part"].get<std::vector<Part>>();
    }
}

struct MediaItem {
    std::string rating_key;
    std::string title;
    std::string type;
    std::string summary;
    std::string thumb;
    std::string art;
    std::string content_rating;
    std::string tagline;
    std::string grandparent_title;
    std::string parent_title;
    int year = 0;
    int index = 0;
    int parent_index = 0;
    int64_t duration = 0;
    int64_t view_offset = 0;
    double rating = 0;
    double audience_rating = 0;
    int leaf_count = 0;
    int viewed_leaf_count = 0;
    std::vector<Tag> genre;
    std::vector<Tag> director;
    std::vector<Tag> role;
    std::vector<Media> media;

    std::string display_title() const {
        return title.empty() ? "Unknown" : title;
    }

    std::string stream_part_key() const {
        if (!media.empty() && !media[0].parts.empty()) {
            return media[0].parts[0].key;
        }
        return "";
    }
};

inline void from_json(const nlohmann::json& j, MediaItem& item) {
    item.rating_key = j.value("ratingKey", "");
    item.title = j.value("title", "");
    item.type = j.value("type", "");
    item.summary = j.value("summary", "");
    item.thumb = j.value("thumb", "");
    item.art = j.value("art", "");
    item.content_rating = j.value("contentRating", "");
    item.tagline = j.value("tagline", "");
    item.grandparent_title = j.value("grandparentTitle", "");
    item.parent_title = j.value("parentTitle", "");
    item.year = j.value("year", 0);
    item.index = j.value("index", 0);
    item.parent_index = j.value("parentIndex", 0);
    item.duration = j.value("duration", int64_t(0));
    item.view_offset = j.value("viewOffset", int64_t(0));
    item.rating = j.value("rating", 0.0);
    item.audience_rating = j.value("audienceRating", 0.0);
    item.leaf_count = j.value("leafCount", 0);
    item.viewed_leaf_count = j.value("viewedLeafCount", 0);

    if (j.contains("Genre") && j["Genre"].is_array())
        item.genre = j["Genre"].get<std::vector<Tag>>();
    if (j.contains("Director") && j["Director"].is_array())
        item.director = j["Director"].get<std::vector<Tag>>();
    if (j.contains("Role") && j["Role"].is_array())
        item.role = j["Role"].get<std::vector<Tag>>();
    if (j.contains("Media") && j["Media"].is_array())
        item.media = j["Media"].get<std::vector<Media>>();
}

struct Library {
    std::string key;
    std::string title;
    std::string type;
};

inline void from_json(const nlohmann::json& j, Library& lib) {
    lib.key = j.value("key", "");
    lib.title = j.value("title", "");
    lib.type = j.value("type", "");
}

struct Hub {
    std::string title;
    std::string hub_identifier;
    std::string type;
    std::vector<MediaItem> items;
};

inline void from_json(const nlohmann::json& j, Hub& hub) {
    hub.title = j.value("title", "");
    hub.hub_identifier = j.value("hubIdentifier", "");
    hub.type = j.value("type", "");
    if (j.contains("Metadata") && j["Metadata"].is_array()) {
        hub.items = j["Metadata"].get<std::vector<MediaItem>>();
    }
}

struct PinResponse {
    int id = 0;
    std::string code;
    std::string auth_token;
};

struct ServerInfo {
    std::string name;
    std::string address;
    int port = 32400;
    std::string access_token;
};

} // namespace plex
