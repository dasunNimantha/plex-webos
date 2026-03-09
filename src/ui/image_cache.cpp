#include "image_cache.h"
#include <SDL2/SDL_image.h>
#include <curl/curl.h>
#include <cstdio>
#include <cstring>

namespace ui {

ImageCache::ImageCache(size_t max_entries) : max_entries_(max_entries) {}

ImageCache::~ImageCache() {
    stop_workers();
}

void ImageCache::start_workers(int count) {
    running_ = true;
    for (int i = 0; i < count; ++i) {
        workers_.emplace_back(&ImageCache::worker_loop, this);
    }
}

void ImageCache::stop_workers() {
    running_ = false;
    queue_cv_.notify_all();
    for (auto& t : workers_) {
        if (t.joinable()) t.join();
    }
    workers_.clear();
}

GLuint ImageCache::get_texture(const std::string& url) {
    auto it = cache_.find(url);
    if (it != cache_.end()) {
        lru_.remove(url);
        lru_.push_front(url);
        return it->second;
    }
    return 0;
}

void ImageCache::request_image(const std::string& url) {
    if (cache_.count(url)) return;
    std::lock_guard<std::mutex> lock(queue_mutex_);
    download_queue_.push(url);
    queue_cv_.notify_one();
}

void ImageCache::upload_pending(Renderer& ren) {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    for (auto& p : pending_results_) {
        if (p.ready && p.pixels && !p.failed) {
            GLuint tex = ren.create_texture(p.pixels, p.w, p.h, p.channels);
            cache_[p.url] = tex;
            lru_.push_front(p.url);
            evict_if_needed();
        }
        if (p.pixels) {
            free(p.pixels);
            p.pixels = nullptr;
        }
    }
    pending_results_.clear();
}

void ImageCache::evict_if_needed() {
    while (cache_.size() > max_entries_ && !lru_.empty()) {
        auto& oldest = lru_.back();
        auto it = cache_.find(oldest);
        if (it != cache_.end()) {
            glDeleteTextures(1, &it->second);
            cache_.erase(it);
        }
        lru_.pop_back();
    }
}

struct DownloadBuffer {
    unsigned char* data = nullptr;
    size_t size = 0;
};

static size_t dl_write(char* ptr, size_t size, size_t nmemb, DownloadBuffer* buf) {
    size_t total = size * nmemb;
    buf->data = (unsigned char*)realloc(buf->data, buf->size + total);
    memcpy(buf->data + buf->size, ptr, total);
    buf->size += total;
    return total;
}

unsigned char* ImageCache::download_and_decode(const std::string& url,
                                                int& w, int& h, int& channels) {
    CURL* curl = curl_easy_init();
    if (!curl) return nullptr;

    DownloadBuffer buf;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || !buf.data) {
        free(buf.data);
        return nullptr;
    }

    SDL_RWops* rw = SDL_RWFromMem(buf.data, (int)buf.size);
    SDL_Surface* surface = IMG_Load_RW(rw, 1);
    free(buf.data);

    if (!surface) return nullptr;

    SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    if (!rgba) return nullptr;

    w = rgba->w;
    h = rgba->h;
    channels = 4;

    size_t pixel_size = (size_t)w * h * channels;
    auto* pixels = (unsigned char*)malloc(pixel_size);
    memcpy(pixels, rgba->pixels, pixel_size);
    SDL_FreeSurface(rgba);

    return pixels;
}

void ImageCache::worker_loop() {
    while (running_) {
        std::string url;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] {
                return !download_queue_.empty() || !running_;
            });
            if (!running_) break;
            url = download_queue_.front();
            download_queue_.pop();
        }

        PendingImage pi;
        pi.url = url;
        pi.pixels = download_and_decode(url, pi.w, pi.h, pi.channels);
        pi.ready = true;
        pi.failed = (pi.pixels == nullptr);

        {
            std::lock_guard<std::mutex> lock(pending_mutex_);
            pending_results_.push_back(std::move(pi));
        }
    }
}

} // namespace ui
