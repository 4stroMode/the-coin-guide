#include "Database.hpp"

static const std::string SUPABASE_URL_LEVELS = "https://gubewqncutfkkutovmbu.supabase.co/rest/v1/levels";
static const std::string SUPABASE_URL_PLAYERS = "https://gubewqncutfkkutovmbu.supabase.co/rest/v1/players";
static const std::string SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Imd1YmV3cW5jdXRma2t1dG92bWJ1Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzM3NDY2MTgsImV4cCI6MjA4OTMyMjYxOH0.-MhblDKmWNJaoLwWG_pZJuaCFW1e59gJioyNzHwatDU";

void Database::fetchLevelData(int levelId, std::function<void(std::optional<CoinGuideData>)> callback) {
    std::string url = fmt::format("{}?levelid=eq.{}&select=*", SUPABASE_URL_LEVELS, levelId);
    
    std::thread([url, callback]() {
        web::WebRequest req;
        req.header("apikey", SUPABASE_KEY);
        req.header("Authorization", "Bearer " + SUPABASE_KEY);
        
        auto response = req.getSync(url);
        
        if (response.code() < 200 || response.code() >= 300) {
            geode::queueInMainThread([callback]() { callback(std::nullopt); });
            return;
        }

        try {
            auto resJson = response.json();
            if (resJson.isOk()) {
                auto json = resJson.unwrap();
                if (json.isArray() && json.asArray().unwrap().size() > 0) {
                    auto first = json.asArray().unwrap()[0];
                    CoinGuideData data;
                    data.levelId = first["levelid"].asInt().unwrapOr(0);
                    data.levelName = first["levelname"].asString().unwrapOr("");
                    data.ncoins = first["ncoins"].asInt().unwrapOr(0);
                    data.coin1 = first["coin1"].asString().unwrapOr("");
                    data.coin2 = first["coin2"].asString().unwrapOr("");
                    data.coin3 = first["coin3"].asString().unwrapOr("");
                    data.addedby = first["addedby"].asString().unwrapOr("");
                    geode::queueInMainThread([callback, data]() { callback(data); });
                } else {
                    geode::queueInMainThread([callback]() { callback(std::nullopt); });
                }
            } else {
                geode::queueInMainThread([callback]() { callback(std::nullopt); });
            }
        } catch (std::exception& e) {
            log::error("Error parsing Database response: {}", e.what());
            geode::queueInMainThread([callback]() { callback(std::nullopt); });
        }
    }).detach();
}

void Database::upsertLevelData(CoinGuideData data, std::function<void(bool)> callback) {
    std::thread([data, callback]() {
        matjson::Value body = matjson::makeObject({
            {"levelid", data.levelId},
            {"levelname", data.levelName},
            {"ncoins", data.ncoins},
            {"coin1", data.coin1},
            {"addedby", data.addedby},
            {"accountid", data.accountid},
            {"rating", data.rating},
            {"cuality", data.cuality},
            {"freecoin", data.freecoin}
        });

        if (!data.coin2.empty()) body["coin2"] = data.coin2;
        if (!data.coin3.empty()) body["coin3"] = data.coin3;

        web::WebRequest req;
        req.header("apikey", SUPABASE_KEY);
        req.header("Authorization", "Bearer " + SUPABASE_KEY);
        req.header("Content-Type", "application/json");
        req.header("Prefer", "resolution=merge-duplicates");
        req.bodyJSON(body);

        auto response = req.postSync(SUPABASE_URL_LEVELS);

        if (response.code() < 200 || response.code() >= 300) {
            log::error("Supabase upsert HTTP {}: {}", response.code(), response.string().unwrapOr("No error body"));
            geode::queueInMainThread([callback]() { callback(false); });
        } else {
            geode::queueInMainThread([callback]() { callback(true); });
        }
    }).detach();
}

void Database::deleteLevelData(int levelId, std::function<void(bool)> callback) {
    std::string url = fmt::format("{}?levelid=eq.{}", SUPABASE_URL_LEVELS, levelId);
    std::thread([url, callback]() {
        web::WebRequest req;
        req.header("apikey", SUPABASE_KEY);
        req.header("Authorization", "Bearer " + SUPABASE_KEY);

        auto response = req.sendSync("DELETE", url);

        if (response.code() < 200 || response.code() >= 300) {
            geode::queueInMainThread([callback]() { callback(false); });
        } else {
            geode::queueInMainThread([callback]() { callback(true); });
        }
    }).detach();
}

void Database::fetchUserRole(int accountId, std::function<void(std::optional<std::string>)> callback) {
    std::string url = fmt::format("{}?playerid=eq.{}&select=rol", SUPABASE_URL_PLAYERS, accountId);
    
    std::thread([url, callback]() {
        web::WebRequest req;
        req.header("apikey", SUPABASE_KEY);
        req.header("Authorization", "Bearer " + SUPABASE_KEY);
        
        auto response = req.getSync(url);
        
        if (response.code() < 200 || response.code() >= 300) {
            geode::queueInMainThread([callback]() { callback(std::nullopt); });
            return;
        }

        try {
            auto resJson = response.json();
            if (resJson.isOk()) {
                auto json = resJson.unwrap();
                if (json.isArray() && json.asArray().unwrap().size() > 0) {
                    auto first = json.asArray().unwrap()[0];
                    std::string rol = first["rol"].asString().unwrapOr("");
                    geode::queueInMainThread([callback, rol]() { callback(rol); });
                } else {
                    geode::queueInMainThread([callback]() { callback(std::nullopt); });
                }
            } else {
                geode::queueInMainThread([callback]() { callback(std::nullopt); });
            }
        } catch (std::exception& e) {
            log::error("Error parsing Database response: {}", e.what());
            geode::queueInMainThread([callback]() { callback(std::nullopt); });
        }
    }).detach();
}

void Database::addModUser(int accountId, std::string const& name, std::string const& role, std::function<void(bool)> callback) {
    std::thread([accountId, name, role, callback]() {
        matjson::Value body = matjson::makeObject({
            {"playerid", accountId},
            {"playername", name},
            {"rol", role}
        });

        web::WebRequest req;
        req.header("apikey", SUPABASE_KEY);
        req.header("Authorization", "Bearer " + SUPABASE_KEY);
        req.header("Content-Type", "application/json");
        req.header("Prefer", "resolution=merge-duplicates");
        req.bodyJSON(body);

        auto response = req.postSync(SUPABASE_URL_PLAYERS);

        if (response.code() < 200 || response.code() >= 300) {
            geode::queueInMainThread([callback]() { callback(false); });
        } else {
            geode::queueInMainThread([callback]() { callback(true); });
        }
    }).detach();
}

void Database::deleteUserRole(int accountId, std::function<void(bool)> callback) {
    std::string url = fmt::format("{}?playerid=eq.{}", SUPABASE_URL_PLAYERS, accountId);
    std::thread([url, callback]() {
        web::WebRequest req;
        req.header("apikey", SUPABASE_KEY);
        req.header("Authorization", "Bearer " + SUPABASE_KEY);

        auto response = req.sendSync("DELETE", url);

        if (response.code() < 200 || response.code() >= 300) {
            geode::queueInMainThread([callback]() { callback(false); });
        } else {
            geode::queueInMainThread([callback]() { callback(true); });
        }
    }).detach();
}

void Database::fetchFreeCoinLevels(std::function<void(std::optional<std::vector<int>>)> callback) {
    std::string url = fmt::format("{}?freecoin=eq.1&select=levelid", SUPABASE_URL_LEVELS);
    std::thread([url, callback]() {
        web::WebRequest req;
        req.header("apikey", SUPABASE_KEY);
        req.header("Authorization", "Bearer " + SUPABASE_KEY);

        auto response = req.sendSync("GET", url);

        if (response.code() >= 200 && response.code() < 300) {
            auto text = response.string().unwrapOr("[]");
            auto res = matjson::parse(text);
            if (res.isOk()) {
                auto json = res.unwrap();
                if (json.isArray()) {
                    std::vector<int> levels;
                    auto arr = json.asArray().unwrap();
                    for (auto item : arr) {
                        levels.push_back(item["levelid"].asInt().unwrapOr(0));
                    }
                    geode::queueInMainThread([callback, levels]() { callback(levels); });
                    return;
                }
            }
        }
        geode::queueInMainThread([callback]() { callback(std::nullopt); });
    }).detach();
}
