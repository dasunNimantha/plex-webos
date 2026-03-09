#pragma once

#include <string>
#include "models.h"

namespace plex {

class PlexAuth {
public:
    PlexAuth(const std::string& client_id);

    PinResponse request_pin();
    PinResponse check_pin(int pin_id);

private:
    std::string client_id_;
    std::string http_post(const std::string& url);
    std::string http_get(const std::string& url);
};

} // namespace plex
