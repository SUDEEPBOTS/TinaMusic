/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#include "tinaproto/glibc_compat.h"
#include "ntgcalls.h"
#include <iostream>
#include <mutex>
#include <condition_variable>

struct AsyncContext {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready{false};
    int error_code{0};
    char* error_message{nullptr};
};

static void AsyncCallback(void* user_data) {
    auto* ctx = static_cast<AsyncContext*>(user_data);
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->ready = true;
    ctx->cv.notify_one();
}

static ntg_async_struct MakeFuture(AsyncContext* ctx) {
    ntg_async_struct f{};
    f.userData = ctx;
    f.promise = AsyncCallback;
    f.errorCode = &ctx->error_code;
    f.errorMessage = &ctx->error_message;
    return f;
}

int main() {
    std::cout << "[TEST] Initializing libntgcalls..." << std::endl;
    uintptr_t handle = ntg_init();
    if (!handle) {
        std::cerr << "Failed to init ntgcalls" << std::endl;
        return 1;
    }
    std::cout << "[TEST] ntgcalls initialized! Handle: " << handle << std::endl;

    char* version = nullptr;
    ntg_get_version(&version);
    if (version) {
        std::cout << "[TEST] NTgCalls version: " << version << std::endl;
    }

    int64_t chat_id = -1002324064857LL;
    char* create_buffer = nullptr;
    AsyncContext ctx{};
    ntg_async_struct fut = MakeFuture(&ctx);

    std::cout << "[TEST] Calling ntg_create for chat " << chat_id << "..." << std::endl;
    int res = ntg_create(handle, chat_id, &create_buffer, fut);
    std::cout << "[TEST] ntg_create returned code: " << res << std::endl;

    {
        std::unique_lock<std::mutex> lock(ctx.mtx);
        ctx.cv.wait(lock, [&] { return ctx.ready; });
    }

    std::cout << "[TEST] Async future finished! ErrorCode: " << ctx.error_code << std::endl;
    if (create_buffer) {
        std::string sdp(create_buffer);
        std::cout << "[TEST] Local SDP JSON size: " << sdp.size() << " bytes" << std::endl;
        std::cout << "[TEST] Local SDP prefix: " << sdp.substr(0, std::min<size_t>(60, sdp.size())) << "..." << std::endl;
    } else {
        std::cout << "[TEST] Buffer is null, error: " << (ctx.error_message ? ctx.error_message : "none") << std::endl;
    }

    ntg_destroy(handle);
    std::cout << "[TEST] SUCCESS: NTgCalls native C++ binding verified!" << std::endl;
    return 0;
}
