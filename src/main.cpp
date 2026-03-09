#include "app.h"
#include <cstdio>

int main(int /*argc*/, char** /*argv*/) {
    printf("Plex webOS Client v0.1.0\n");

    App app;
    if (!app.init()) {
        fprintf(stderr, "Failed to initialize app\n");
        return 1;
    }

    app.run();
    app.shutdown();

    printf("Goodbye!\n");
    return 0;
}
