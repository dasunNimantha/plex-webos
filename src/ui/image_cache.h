#pragma once

#include <string>
#include <unordered_map>
#include <list>
#include <mutex>
#include <thread>
#include <queue>
#include <functional>
#include <atomic>
#include <condition_variable>
#include "renderer.h"

namespace ui {

struct PendingImage {
    std::string url;
    unsigned char* pixels = nullptr;
    int w = 0, h = 0, channels = 0;
    bool ready = false;
    bool failed = false;
};

class ImageCache {
public:
    ImageCache(size_t max_entries = 200);
    ~ImageCache();

    void start_workers(int count = 2);
    void stop_workers();

    GLuint get_texture(const std::string& url);
    void request_image(const std::string& url);
    void upload_pending(Renderer& ren);

private:
    void worker_loop();
    static unsigned char* download_and_decode(const std::string& url,
                                               int& w, int& h, int& channels);

    size_t max_entries_;

    std::unordered_map<std::string, GLuint> cache_;
    std::list<std::string> lru_;

    std::mutex pending_mutex_;
    std::queue<std::string> download_queue_;
    std::vector<PendingImage> pending_results_;

    std::vector<std::thread> workers_;
    std::atomic<bool> running_{false};
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    void evict_if_needed();
};

} // namespace ui
