#include "locales.hpp"

LocalizationManager& LocalizationManager::Instance() {
    static LocalizationManager instance;
    return instance;
}

LocalizationManager::LocalizationManager() {
    InitLocales();
}

void LocalizationManager::InitLocales() {
    strings_["en"] = {
        {"welcome", "<blockquote>🎵 <b>Welcome to YukkiMusic (C++ Engine)</b></blockquote>\n\n<blockquote>High-performance, ultra-low latency Telegram music engine built directly in native C++20.\n\n• Send <code>/play [song name]</code> to start playing.\n• Send <code>/help</code> for all commands list.</blockquote>"},
        {"help", "<blockquote>📖 <b>YukkiMusic C++ Commands:</b></blockquote>\n\n<blockquote>• <code>/play [name/url]</code> — Search & play track\n• <code>/pause</code> — Pause current audio stream\n• <code>/resume</code> — Resume playback\n• <code>/skip</code> — Skip to next song in queue\n• <code>/stop</code> — Clear queue and stop player\n• <code>/queue</code> — View current active queue\n• <code>/seek [seconds]</code> — Seek playback position\n• <code>/speed [0.5-2.0]</code> — Adjust playback speed\n• <code>/loop</code> — Toggle repeat mode on/off\n• <code>/shuffle</code> — Shuffle pending queue\n• <code>/stats</code> — Server telemetry & performance\n• <code>/ping</code> — Server latency & ping</blockquote>"},
        {"now_playing", "▶️ <b>Now Playing:</b>"},
        {"added_queue", "📋 <b>Added to Queue:</b>"},
        {"paused", "⏸ <b>Playback Paused</b>"},
        {"resumed", "▶️ <b>Playback Resumed</b>"},
        {"stopped", "⏹ <b>Playback Stopped and Queue Cleared</b>"},
        {"skipped", "⏭ <b>Track Skipped</b>"},
        {"empty_queue", "📜 <b>Audio Queue is currently empty.</b>"},
        {"not_found", "❌ <b>Track Not Found. Please try another query.</b>"}
    };

    strings_["hi"] = {
        {"welcome", "<blockquote>🎵 <b>युक्कीम्यूजिक (C++ इंजन) में आपका स्वागत है</b></blockquote>\n\n<blockquote>अत्यंत तीव्र और कुशल टेलीग्राम संगीत बॉट जो मूल C++20 में निर्मित है।\n\n• गाना बजाने के लिए <code>/play [गाने का नाम]</code> भेजें।\n• सभी कमांड देखने के लिए <code>/help</code> भेजें।</blockquote>"},
        {"help", "<blockquote>📖 <b>युक्कीम्यूजिक C++ कमांड्स:</b></blockquote>\n\n<blockquote>• <code>/play [नाम/लिंक]</code> — गाना खोजें और बजाएं\n• <code>/pause</code> — संगीत रोकें\n• <code>/resume</code> — संगीत पुनः चालू करें\n• <code>/skip</code> — अगला गाना बजाएं\n• <code>/stop</code> — संगीत बंद करें और कतार खाली करें\n• <code>/queue</code> — वर्तमान संगीत कतार देखें\n• <code>/seek [सेकंड]</code> — आगे/पीछे करें\n• <code>/speed [0.5-2.0]</code> — गति नियंत्रित करें\n• <code>/loop</code> — लूप मोड चालू/बंद करें\n• <code>/shuffle</code> — कतार को शफल करें\n• <code>/stats</code> — सर्वर आंकड़े देखें\n• <code>/ping</code> — पिंग और लेटेंसी जांचें</blockquote>"},
        {"now_playing", "▶️ <b>अब बज रहा है:</b>"},
        {"added_queue", "📋 <b>कतार में जोड़ा गया:</b>"},
        {"paused", "⏸ <b>संगीत रोक दिया गया है</b>"},
        {"resumed", "▶️ <b>संगीत पुनः चालू हो गया है</b>"},
        {"stopped", "⏹ <b>संगीत बंद और कतार साफ</b>"},
        {"skipped", "⏭ <b>गाना स्किप किया गया</b>"},
        {"empty_queue", "📜 <b>वर्तमान में कोई गाना कतार में नहीं है।</b>"},
        {"not_found", "❌ <b>गाना नहीं मिला। कृपया दोबारा प्रयास करें।</b>"}
    };

    strings_["ar"] = {
        {"welcome", "<blockquote>🎵 <b>أهلاً بك في يوكي ميوزك (محرك C++)</b></blockquote>\n\n<blockquote>محرك موسيقى فائق السرعة والأداء للتيليجرام مبني بلغة C++20.\n\n• أرسل <code>/play [اسم الأغنية]</code> للتشغيل.\n• أرسل <code>/help</code> لقائمة الأوامر.</blockquote>"},
        {"help", "<blockquote>📖 <b>أوامر يوكي ميوزك:</b></blockquote>\n\n<blockquote>• <code>/play [الاسم/الرابط]</code> — تشغيل مقطع صوتي\n• <code>/pause</code> — إيقاف مؤقت\n• <code>/resume</code> — استئناف التشغيل\n• <code>/skip</code> — تخطي المقطع الحالي\n• <code>/stop</code> — إيقاف ومسح القائمة\n• <code>/queue</code> — عرض قائمة الانتظار\n• <code>/stats</code> — إحصائيات الخادم\n• <code>/ping</code> — فحص سرعة الاستجابة</blockquote>"},
        {"now_playing", "▶️ <b>يتم التشغيل الآن:</b>"},
        {"added_queue", "📋 <b>تمت الإضافة إلى قائمة الانتظار:</b>"},
        {"paused", "⏸ <b>تم الإيقاف المؤقت</b>"},
        {"resumed", "▶️ <b>تم استئناف التشغيل</b>"},
        {"stopped", "⏹ <b>تم إيقاف التشغيل</b>"},
        {"skipped", "⏭ <b>تم التخطي</b>"},
        {"empty_queue", "📜 <b>قائمة الانتظار فارغة حالياً.</b>"},
        {"not_found", "❌ <b>لم يتم العثور على المسار المطلوب.</b>"}
    };

    strings_["tr"] = {
        {"welcome", "<blockquote>🎵 <b>YukkiMusic (C++ Motoru)'na Hoş Geldiniz</b></blockquote>\n\n<blockquote>Yerel C++20 ile oluşturulmuş yüksek performanslı Telegram müzik motoru.\n\n• Oynatmak için <code>/play [şarkı adı]</code> gönderin.\n• Komutlar için <code>/help</code> yazın.</blockquote>"},
        {"help", "<blockquote>📖 <b>YukkiMusic C++ Komutları:</b></blockquote>\n\n<blockquote>• <code>/play [ad/link]</code> — Şarkı ara ve çal\n• <code>/pause</code> — Oynatmayı duraklat\n• <code>/resume</code> — Devam ettir\n• <code>/skip</code> — Sonraki şarkıya geç\n• <code>/stop</code> — Sırayı temizle ve durdur\n• <code>/queue</code> — Çalma listesini gör\n• <code>/stats</code> — Sunucu istatistikleri\n• <code>/ping</code> — Gecikme kontrolü</blockquote>"},
        {"now_playing", "▶️ <b>Şimdi Çalıyor:</b>"},
        {"added_queue", "📋 <b>Sıraya Eklendi:</b>"},
        {"paused", "⏸ <b>Durduruldu</b>"},
        {"resumed", "▶️ <b>Devam Ediyor</b>"},
        {"stopped", "⏹ <b>Durduruldu ve Temizlendi</b>"},
        {"skipped", "⏭ <b>Atlandı</b>"},
        {"empty_queue", "📜 <b>Çalma listesi boş.</b>"},
        {"not_found", "❌ <b>Şarkı bulunamadı.</b>"}
    };
}

std::string LocalizationManager::Get(const std::string& lang, const std::string& key) const {
    std::lock_guard<std::mutex> lock(lang_mutex_);
    auto itLang = strings_.find(lang);
    if (itLang == strings_.end()) {
        itLang = strings_.find("en");
    }

    auto itKey = itLang->second.find(key);
    if (itKey != itLang->second.end()) {
        return itKey->second;
    }

    auto enIt = strings_.at("en").find(key);
    if (enIt != strings_.at("en").end()) {
        return enIt->second;
    }

    return key;
}

void LocalizationManager::SetChatLang(int64_t chat_id, const std::string& lang) {
    std::lock_guard<std::mutex> lock(lang_mutex_);
    chat_langs_[chat_id] = lang;
}

std::string LocalizationManager::GetChatLang(int64_t chat_id) const {
    std::lock_guard<std::mutex> lock(lang_mutex_);
    auto it = chat_langs_.find(chat_id);
    if (it != chat_langs_.end()) {
        return it->second;
    }
    return "en";
}
