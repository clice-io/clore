#include "kota/zest/zest.h"

import http;

int main(int argc, char** argv) {
    struct HttpRuntimeShutdownGuard {
        ~HttpRuntimeShutdownGuard() {
            clore::net::shutdown_llm_rate_limit();
        }
    } http_runtime_shutdown_guard;

    return kota::zest::run_cli(argc, argv);
}
