/*
 * ============================================================================
 *   Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 *   Architecture: Native C++20 Core + Pure C++20 TinaProto Voice Engine
 *   Copyright (C) 2026 SUDEEPBOTS / TinaMusic Contributors
 *   Support: https://t.me/Z0iiw
 *   License: GNU Affero General Public License v3.0
 * ============================================================================
 */

#include "tinaproto/voice_binding.hpp"
#include "ntgcalls.h"
#include <mutex>
#include <condition_variable>
#include <cstring>

namespace tinaproto {

namespace {

struct AsyncContext {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready{false};
    int error_code{0};
    char* error_message{nullptr};
};

void AsyncCallback(void* user_data) {
    auto* ctx = static_cast<AsyncContext*>(user_data);
    std::lock_guard<std::mutex> lock(ctx->mtx);
    ctx->ready = true;
    ctx->cv.notify_one();
}

ntg_async_struct MakeFuture(AsyncContext* ctx) {
    ntg_async_struct f{};
    f.userData = ctx;
    f.promise = AsyncCallback;
    f.errorCode = &ctx->error_code;
    f.errorMessage = &ctx->error_message;
    return f;
}

}

VoiceBinding::VoiceBinding() = default;

VoiceBinding::~VoiceBinding() {
    if (handle_ != 0) {
        ntg_destroy(handle_);
        handle_ = 0;
    }
}

bool VoiceBinding::Init() {
    if (handle_ != 0) {
        return true;
    }
    handle_ = ntg_init();
    if (handle_ == 0) {
        return false;
    }
    ntg_on_stream_end(handle_, reinterpret_cast<ntg_stream_callback>(StreamEndThunk), this);
    return true;
}

void VoiceBinding::StreamEndThunk(uintptr_t ptr, int64_t chat_id, int stream_type, int stream_device, void* user_data) {
    (void)ptr;
    (void)stream_device;
    if (stream_type != NTG_STREAM_AUDIO) {
        return;
    }
    auto* self = static_cast<VoiceBinding*>(user_data);
    if (!self) {
        return;
    }
    if (self->on_stream_end_cb_) {
        self->on_stream_end_cb_(chat_id);
    }
}

std::string VoiceBinding::CreateCall(int64_t chat_id) {
    if (!Init()) {
        return "";
    }
    char* create_buffer = nullptr;
    AsyncContext ctx{};
    ntg_async_struct fut = MakeFuture(&ctx);

    int res = ntg_create(handle_, chat_id, &create_buffer, fut);
    if (res < 0) {
        return "";
    }
    {
        std::unique_lock<std::mutex> lock(ctx.mtx);
        ctx.cv.wait(lock, [&] { return ctx.ready; });
    }
    if (ctx.error_code != 0 || !create_buffer) {
        return "";
    }
    std::string sdp(create_buffer);
    return sdp;
}

bool VoiceBinding::SetStreamSources(int64_t chat_id, const std::string& shell_cmd) {
    if (!Init()) {
        return false;
    }
    ntg_audio_description_struct audio_desc{};
    audio_desc.mediaSource = NTG_SHELL;
    audio_desc.input = const_cast<char*>(shell_cmd.c_str());
    audio_desc.sampleRate = 48000;
    audio_desc.channelCount = 2;
    audio_desc.keepOpen = false;

    ntg_media_description_struct media_desc{};
    media_desc.microphone = &audio_desc;

    AsyncContext ctx{};
    ntg_async_struct fut = MakeFuture(&ctx);

    int res = ntg_set_stream_sources(handle_, chat_id, NTG_STREAM_CAPTURE, media_desc, fut);
    if (res < 0) {
        return false;
    }
    {
        std::unique_lock<std::mutex> lock(ctx.mtx);
        ctx.cv.wait(lock, [&] { return ctx.ready; });
    }
    return (ctx.error_code == 0);
}

bool VoiceBinding::Connect(int64_t chat_id, const std::string& remote_params) {
    if (!Init()) {
        return false;
    }
    AsyncContext ctx{};
    ntg_async_struct fut = MakeFuture(&ctx);

    int res = ntg_connect(handle_, chat_id, const_cast<char*>(remote_params.c_str()), false, fut);
    if (res < 0) {
        return false;
    }
    {
        std::unique_lock<std::mutex> lock(ctx.mtx);
        ctx.cv.wait(lock, [&] { return ctx.ready; });
    }
    if (ctx.error_code == 0) {
        std::lock_guard<std::mutex> lock(calls_mtx_);
        active_calls_.insert(chat_id);
        return true;
    }
    return false;
}

bool VoiceBinding::Pause(int64_t chat_id) {
    if (handle_ == 0) return false;
    AsyncContext ctx{};
    ntg_async_struct fut = MakeFuture(&ctx);
    ntg_pause(handle_, chat_id, fut);
    {
        std::unique_lock<std::mutex> lock(ctx.mtx);
        ctx.cv.wait(lock, [&] { return ctx.ready; });
    }
    return (ctx.error_code == 0);
}

bool VoiceBinding::Resume(int64_t chat_id) {
    if (handle_ == 0) return false;
    AsyncContext ctx{};
    ntg_async_struct fut = MakeFuture(&ctx);
    ntg_resume(handle_, chat_id, fut);
    {
        std::unique_lock<std::mutex> lock(ctx.mtx);
        ctx.cv.wait(lock, [&] { return ctx.ready; });
    }
    return (ctx.error_code == 0);
}

bool VoiceBinding::Stop(int64_t chat_id) {
    if (handle_ == 0) return false;
    {
        std::lock_guard<std::mutex> lock(calls_mtx_);
        active_calls_.erase(chat_id);
    }
    AsyncContext ctx{};
    ntg_async_struct fut = MakeFuture(&ctx);
    ntg_stop(handle_, chat_id, fut);
    {
        std::unique_lock<std::mutex> lock(ctx.mtx);
        ctx.cv.wait(lock, [&] { return ctx.ready; });
    }
    return (ctx.error_code == 0);
}

int64_t VoiceBinding::Time(int64_t chat_id) {
    if (handle_ == 0) return 0;
    int64_t current_time = 0;
    AsyncContext ctx{};
    ntg_async_struct fut = MakeFuture(&ctx);
    ntg_time(handle_, chat_id, NTG_STREAM_CAPTURE, &current_time, fut);
    {
        std::unique_lock<std::mutex> lock(ctx.mtx);
        ctx.cv.wait(lock, [&] { return ctx.ready; });
    }
    return current_time;
}

void VoiceBinding::SetOnStreamEnd(std::function<void(int64_t)> cb) {
    on_stream_end_cb_ = std::move(cb);
}

bool VoiceBinding::HasActiveCall(int64_t chat_id) {
    std::lock_guard<std::mutex> lock(calls_mtx_);
    return active_calls_.count(chat_id) > 0;
}

}
