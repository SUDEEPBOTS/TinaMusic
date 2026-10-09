#include "bot_engine.hpp"
#include "logger.hpp"
#include "platform_yt.hpp"
#include <thread>
#include <chrono>
#include <sstream>

BotEngine::BotEngine(const AppConfig& config)
    : config_(config), tg_(std::make_unique<TelegramClient>(config.bot_token)) {}

void BotEngine::Stop() {
    is_running_ = false;
    Logger::Info("BOT", "Stopping polling loop...");
}

void BotEngine::Run() {
    is_running_ = true;
    Logger::Info("BOT", "Starting Telegram polling loop (C++20 Engine)...");

    while (is_running_) {
        std::vector<TgUpdate> updates = tg_->GetUpdates(last_update_id_ + 1, 15);
        for (const auto& u : updates) {
            if (u.update_id > last_update_id_) {
                last_update_id_ = u.update_id;
            }

            if (u.has_message) {
                ProcessMessage(u.message);
            } else if (u.has_callback) {
                ProcessCallback(u.callback_query);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void BotEngine::ProcessMessage(const TgMessage& msg) {
    if (msg.text.empty() || msg.text[0] != '/') return;

    std::stringstream ss(msg.text);
    std::string cmd;
    ss >> cmd;

    auto at_pos = cmd.find('@');
    if (at_pos != std::string::npos) {
        cmd = cmd.substr(0, at_pos);
    }

    std::string args;
    std::getline(ss, args);
    auto start = args.find_first_not_of(" \t");
    if (start != std::string::npos) {
        args = args.substr(start);
    } else {
        args = "";
    }

    if (cmd == "/start") {
        HandleStart(msg);
    } else if (cmd == "/help") {
        HandleHelp(msg);
    } else if (cmd == "/ping") {
        HandlePing(msg);
    } else if (cmd == "/play" || cmd == "/p") {
        HandlePlay(msg, args);
    } else if (cmd == "/pause") {
        HandlePause(msg);
    } else if (cmd == "/resume") {
        HandleResume(msg);
    } else if (cmd == "/skip") {
        HandleSkip(msg);
    } else if (cmd == "/stop") {
        HandleStop(msg);
    } else if (cmd == "/queue" || cmd == "/q") {
        HandleQueue(msg);
    } else if (cmd == "/loop") {
        HandleLoop(msg);
    } else if (cmd == "/shuffle") {
        HandleShuffle(msg);
    }
}

nlohmann::json BotEngine::MakePlayerControlsMarkup(int64_t chat_id) {
    return {
        {"inline_keyboard", {
            {
                {{"text", "⏸ Pause"}, {"callback_data", "btn_pause"}},
                {{"text", "▶️ Resume"}, {"callback_data", "btn_resume"}},
                {{"text", "⏭ Skip"}, {"callback_data", "btn_skip"}}
            },
            {
                {{"text", "⏹ Stop"}, {"callback_data", "btn_stop"}},
                {{"text", "📜 Queue"}, {"callback_data", "btn_queue"}}
            }
        }}
    };
}

void BotEngine::HandleStart(const TgMessage& msg) {
    std::string welcome = "<blockquote>🎵 <b>Welcome to YukkiMusic (C++ Engine)</b></blockquote>\n\n"
                          "<blockquote>High-performance, ultra-low latency Telegram music engine built directly in native C++20.\n\n"
                          "• Send <code>/play [song name]</code> to start playing.\n"
                          "• Send <code>/help</code> for all commands list.</blockquote>";
    tg_->SendMessage(msg.chat.id, welcome);
}

void BotEngine::HandleHelp(const TgMessage& msg) {
    std::string help = "<blockquote>📖 <b>YukkiMusic C++ Commands:</b></blockquote>\n\n"
                       "<blockquote>• <code>/play [name/url]</code> — Search & play song\n"
                       "• <code>/pause</code> — Pause current audio stream\n"
                       "• <code>/resume</code> — Resume playback\n"
                       "• <code>/skip</code> — Skip to next song in queue\n"
                       "• <code>/stop</code> — Clear queue and stop player\n"
                       "• <code>/queue</code> — View current active queue\n"
                       "• <code>/loop</code> — Toggle repeat mode on/off\n"
                       "• <code>/shuffle</code> — Shuffle pending queue\n"
                       "• <code>/ping</code> — Check server ping and uptime</blockquote>";
    tg_->SendMessage(msg.chat.id, help);
}

void BotEngine::HandlePing(const TgMessage& msg) {
    auto start = std::chrono::steady_clock::now();
    int msg_id = tg_->SendMessage(msg.chat.id, "<blockquote>⚡ <i>Pinging server...</i></blockquote>");
    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::string res = "<blockquote>🏓 <b>Pong!</b> <code>" + std::to_string(ms) + "ms</code>\n"
                      "<b>Engine:</b> <code>Native C++20 / Low-Latency</code></blockquote>";
    if (msg_id > 0) {
        tg_->EditMessageText(msg.chat.id, msg_id, res);
    } else {
        tg_->SendMessage(msg.chat.id, res);
    }
}

void BotEngine::HandlePlay(const TgMessage& msg, const std::string& query) {
    if (query.empty()) {
        tg_->SendMessage(msg.chat.id, "<blockquote>⚠️ <i>Please provide a song title or link:</i>\n<code>/play Faded Alan Walker</code></blockquote>");
        return;
    }

    int wait_msg = tg_->SendMessage(msg.chat.id, "<blockquote>🔍 <b>Searching track:</b> <code>" + query + "</code>...</blockquote>");

    std::string requester = msg.from.first_name.empty() ? "User" : msg.from.first_name;
    auto trackOpt = PlatformYouTube::SearchTrack(query, requester, msg.from.id);

    if (!trackOpt.has_value()) {
        std::string errText = "<blockquote>❌ <b>Track Not Found:</b> <code>" + query + "</code>\n<i>Please try another query or direct link.</i></blockquote>";
        if (wait_msg > 0) {
            tg_->EditMessageText(msg.chat.id, wait_msg, errText);
        } else {
            tg_->SendMessage(msg.chat.id, errText);
        }
        return;
    }

    Track track = trackOpt.value();
    auto& qm = QueueManager::Instance();
    size_t prevSize = qm.QueueSize(msg.chat.id);
    qm.Enqueue(msg.chat.id, track);

    if (prevSize == 0) {
        AudioEngine::Instance().StartPlayback(msg.chat.id, track.stream_url);

        std::string nowPlaying = "<blockquote>▶️ <b>Now Playing:</b>\n"
                                 "• <b>Title:</b> <a href=\"" + track.url + "\">" + track.title + "</a>\n"
                                 "• <b>Duration:</b> <code>" + track.duration + "</code>\n"
                                 "• <b>Requested By:</b> <code>" + track.requester_name + "</code></blockquote>";
        auto markup = MakePlayerControlsMarkup(msg.chat.id);
        if (wait_msg > 0) {
            tg_->EditMessageText(msg.chat.id, wait_msg, nowPlaying, markup);
        } else {
            tg_->SendMessage(msg.chat.id, nowPlaying, markup);
        }
    } else {
        std::string queued = "<blockquote>📋 <b>Added to Queue (#" + std::to_string(prevSize + 1) + "):</b>\n"
                             "• <b>Title:</b> <a href=\"" + track.url + "\">" + track.title + "</a>\n"
                             "• <b>Duration:</b> <code>" + track.duration + "</code>\n"
                             "• <b>Requested By:</b> <code>" + track.requester_name + "</code></blockquote>";
        if (wait_msg > 0) {
            tg_->EditMessageText(msg.chat.id, wait_msg, queued);
        } else {
            tg_->SendMessage(msg.chat.id, queued);
        }
    }
}

void BotEngine::HandlePause(const TgMessage& msg) {
    auto& qm = QueueManager::Instance();
    if (!qm.GetCurrent(msg.chat.id).has_value()) {
        tg_->SendMessage(msg.chat.id, "<blockquote>⚠️ <i>Nothing is currently playing.</i></blockquote>");
        return;
    }
    AudioEngine::Instance().PausePlayback(msg.chat.id);
    qm.SetStatus(msg.chat.id, PlaybackStatus::PAUSED);
    tg_->SendMessage(msg.chat.id, "<blockquote>⏸ <b>Playback Paused</b></blockquote>");
}

void BotEngine::HandleResume(const TgMessage& msg) {
    auto& qm = QueueManager::Instance();
    if (!qm.GetCurrent(msg.chat.id).has_value()) {
        tg_->SendMessage(msg.chat.id, "<blockquote>⚠️ <i>Nothing is currently playing.</i></blockquote>");
        return;
    }
    AudioEngine::Instance().ResumePlayback(msg.chat.id);
    qm.SetStatus(msg.chat.id, PlaybackStatus::PLAYING);
    tg_->SendMessage(msg.chat.id, "<blockquote>▶️ <b>Playback Resumed</b></blockquote>");
}

void BotEngine::HandleSkip(const TgMessage& msg) {
    auto& qm = QueueManager::Instance();
    auto nextOpt = qm.Next(msg.chat.id);

    if (nextOpt.has_value()) {
        Track nextTrack = nextOpt.value();
        AudioEngine::Instance().StartPlayback(msg.chat.id, nextTrack.stream_url);

        std::string text = "<blockquote>⏭ <b>Skipped! Now Playing:</b>\n"
                           "• <b>Title:</b> <a href=\"" + nextTrack.url + "\">" + nextTrack.title + "</a>\n"
                           "• <b>Duration:</b> <code>" + nextTrack.duration + "</code>\n"
                           "• <b>Requested By:</b> <code>" + nextTrack.requester_name + "</code></blockquote>";
        auto markup = MakePlayerControlsMarkup(msg.chat.id);
        tg_->SendMessage(msg.chat.id, text, markup);
    } else {
        AudioEngine::Instance().StopPlayback(msg.chat.id);
        tg_->SendMessage(msg.chat.id, "<blockquote>⏹ <b>Queue finished. Playback stopped.</b></blockquote>");
    }
}

void BotEngine::HandleStop(const TgMessage& msg) {
    auto& qm = QueueManager::Instance();
    qm.Clear(msg.chat.id);
    AudioEngine::Instance().StopPlayback(msg.chat.id);
    tg_->SendMessage(msg.chat.id, "<blockquote>⏹ <b>Queue cleared and playback terminated.</b></blockquote>");
}

void BotEngine::HandleQueue(const TgMessage& msg) {
    auto& qm = QueueManager::Instance();
    auto current = qm.GetCurrent(msg.chat.id);
    auto pending = qm.GetPending(msg.chat.id);

    if (!current.has_value() && pending.empty()) {
        tg_->SendMessage(msg.chat.id, "<blockquote>📜 <b>Queue is currently empty.</b></blockquote>");
        return;
    }

    std::stringstream ss;
    ss << "<blockquote>📜 <b>Current Audio Queue:</b></blockquote>\n\n";

    if (current.has_value()) {
        ss << "<blockquote>▶️ <b>Currently Playing:</b>\n"
           << "• <b>Title:</b> " << current.value().title << "\n"
           << "• <b>Duration:</b> <code>" << current.value().duration << "</code></blockquote>\n\n";
    }

    if (!pending.empty()) {
        ss << "<blockquote expandable><b>Up Next:</b>\n";
        for (size_t i = 0; i < pending.size() && i < 10; ++i) {
            ss << std::to_string(i + 1) << ". " << pending[i].title << " (<code>" << pending[i].duration << "</code>)\n";
        }
        if (pending.size() > 10) {
            ss << "<i>... and " << std::to_string(pending.size() - 10) << " more tracks</i>\n";
        }
        ss << "</blockquote>";
    }

    tg_->SendMessage(msg.chat.id, ss.str());
}

void BotEngine::HandleLoop(const TgMessage& msg) {
    auto& qm = QueueManager::Instance();
    bool currentLoop = qm.IsLooping(msg.chat.id);
    qm.SetLoop(msg.chat.id, !currentLoop);

    std::string status = !currentLoop ? "ENABLED ✅" : "DISABLED ❌";
    tg_->SendMessage(msg.chat.id, "<blockquote>🔂 <b>Repeat Loop:</b> <code>" + status + "</code></blockquote>");
}

void BotEngine::HandleShuffle(const TgMessage& msg) {
    auto& qm = QueueManager::Instance();
    if (qm.GetPending(msg.chat.id).empty()) {
        tg_->SendMessage(msg.chat.id, "<blockquote>⚠️ <i>No pending tracks to shuffle.</i></blockquote>");
        return;
    }
    qm.Shuffle(msg.chat.id);
    tg_->SendMessage(msg.chat.id, "<blockquote>🔀 <b>Queue shuffled successfully!</b></blockquote>");
}

void BotEngine::ProcessCallback(const TgCallbackQuery& cb) {
    tg_->AnswerCallbackQuery(cb.id, "Processing...");

    int64_t chat_id = cb.message.chat.id;
    auto& qm = QueueManager::Instance();

    if (cb.data == "btn_pause") {
        AudioEngine::Instance().PausePlayback(chat_id);
        qm.SetStatus(chat_id, PlaybackStatus::PAUSED);
        tg_->SendMessage(chat_id, "<blockquote>⏸ <b>Playback Paused via button</b></blockquote>");
    } else if (cb.data == "btn_resume") {
        AudioEngine::Instance().ResumePlayback(chat_id);
        qm.SetStatus(chat_id, PlaybackStatus::PLAYING);
        tg_->SendMessage(chat_id, "<blockquote>▶️ <b>Playback Resumed via button</b></blockquote>");
    } else if (cb.data == "btn_skip") {
        auto nextOpt = qm.Next(chat_id);
        if (nextOpt.has_value()) {
            Track nextTrack = nextOpt.value();
            AudioEngine::Instance().StartPlayback(chat_id, nextTrack.stream_url);
            std::string text = "<blockquote>⏭ <b>Skipped! Now Playing:</b>\n"
                               "• <b>Title:</b> " + nextTrack.title + "</blockquote>";
            tg_->SendMessage(chat_id, text, MakePlayerControlsMarkup(chat_id));
        } else {
            AudioEngine::Instance().StopPlayback(chat_id);
            tg_->SendMessage(chat_id, "<blockquote>⏹ <b>Queue finished. Stopped.</b></blockquote>");
        }
    } else if (cb.data == "btn_stop") {
        qm.Clear(chat_id);
        AudioEngine::Instance().StopPlayback(chat_id);
        tg_->SendMessage(chat_id, "<blockquote>⏹ <b>Playback stopped by user.</b></blockquote>");
    } else if (cb.data == "btn_queue") {
        TgMessage fakeMsg;
        fakeMsg.chat.id = chat_id;
        HandleQueue(fakeMsg);
    }
}
