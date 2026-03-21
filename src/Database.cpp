#include "Database.hpp"
#include <algorithm>

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
            try {
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
            } catch (std::exception& e) {
                log::error("Exception in fetchFreeCoinLevels: {}", e.what());
            }
        }
        geode::queueInMainThread([callback]() { callback(std::nullopt); });
    }).detach();
}

void Database::fetchFilteredLevels(std::vector<int> ratings, std::vector<int> coinRatings, int quality, bool freeCoins, bool recentlyAdded, bool onlyWithGuide, std::function<void(std::optional<std::vector<int>>)> callback) {
    std::string url = SUPABASE_URL_LEVELS + "?select=levelid";
    
    std::string crUrlParams = "";
    
    if (!ratings.empty()) {
        std::vector<int> finalRatings = ratings;
        // If 10 is present, add 11, 12, 13, 14
        if (std::find(finalRatings.begin(), finalRatings.end(), 10) != finalRatings.end()) {
            finalRatings.push_back(11);
            finalRatings.push_back(12);
            finalRatings.push_back(13);
            finalRatings.push_back(14);
        }
        std::string ratingStr = "&rating=in.(";
        crUrlParams += "&levelrating=in.(";
        for (size_t i = 0; i < finalRatings.size(); ++i) {
            ratingStr += std::to_string(finalRatings[i]);
            crUrlParams += std::to_string(finalRatings[i]);
            if (i < finalRatings.size() - 1) {
                ratingStr += ",";
                crUrlParams += ",";
            }
        }
        ratingStr += ")";
        crUrlParams += ")";
        url += ratingStr;
    }
    
    if (quality > 0) {
        std::string qualStr = "";
        if (quality == 1) qualStr = "star";
        else if (quality == 2) qualStr = "featured";
        else if (quality == 3) qualStr = "epic";
        else if (quality == 4) qualStr = "legendary";
        else if (quality == 5) qualStr = "mythic";
        
        if (!qualStr.empty()) {
            url += "&cuality=eq." + qualStr;
            crUrlParams += "&levelcuality=eq." + qualStr;
        }
    }
    
    if (freeCoins) url += "&freecoin=eq.1";
    if (recentlyAdded) url += "&order=created_at.desc";
    else url += "&order=id.desc";

    std::thread([url, crUrlParams, ratings, quality, freeCoins, coinRatings, onlyWithGuide, callback]() {
        web::WebRequest req;
        req.header("apikey", SUPABASE_KEY);
        req.header("Authorization", "Bearer " + SUPABASE_KEY);

        std::set<int> uniqueLevels;
        bool success = false;
        
        bool hasLevelsFilters = (!ratings.empty() || quality > 0 || freeCoins);

        auto response = req.sendSync("GET", url);
        if (response.code() >= 200 && response.code() < 300) {
            try {
                auto text = response.string().unwrapOr("[]");
                auto res = matjson::parse(text);
                if (res.isOk()) {
                    auto json = res.unwrap();
                    if (json.isArray()) {
                        success = true;
                        auto arr = json.asArray().unwrap();
                        for (auto item : arr) {
                            uniqueLevels.insert(item["levelid"].asInt().unwrapOr(0));
                        }
                    }
                }
            } catch (std::exception& e) {
                log::error("Exception in fetchFilteredLevels (levels): {}", e.what());
            }
        }

        std::set<int> validCoinRatingLevels;
        bool fetchedCoinRatings = false;

        if (!onlyWithGuide || !coinRatings.empty() || !crUrlParams.empty()) {
            std::string coinsRatingUrl = SUPABASE_URL_LEVELS.substr(0, SUPABASE_URL_LEVELS.find("/rest/v1/levels")) + "/rest/v1/coinsrating?select=*" + crUrlParams;
            
            web::WebRequest reqCR;
            reqCR.header("apikey", SUPABASE_KEY);
            reqCR.header("Authorization", "Bearer " + SUPABASE_KEY);
            auto responseCR = reqCR.sendSync("GET", coinsRatingUrl);
            if (responseCR.code() >= 200 && responseCR.code() < 300) {
                try {
                    auto textCR = responseCR.string().unwrapOr("[]");
                    auto resCR = matjson::parse(textCR);
                    if (resCR.isOk()) {
                        auto jsonCR = resCR.unwrap();
                        if (jsonCR.isArray()) {
                            fetchedCoinRatings = true;
                            success = true; // AT least one table worked
                            auto arrCR = jsonCR.asArray().unwrap();
                            for (auto item : arrCR) {
                                int totalSum = 0;
                                int totalCount = 0;
                                for (int i = 1; i <= 10; ++i) {
                                    std::string colName = fmt::format("ranking{}", i);
                                    if (item.contains(colName)) {
                                        int votes = 0;
                                        if (item[colName].isNumber()) {
                                            votes = item[colName].asInt().unwrapOr(0);
                                        } else if (item[colName].isString()) {
                                            try {
                                                votes = std::stoi(item[colName].asString().unwrapOr("0"));
                                            } catch (...) {}
                                        }
                                        totalSum += (i * votes);
                                        totalCount += votes;
                                    }
                                }
                                int crLevelId = item["levelid"].asInt().unwrapOr(0);
                                if (totalCount > 0) {
                                    int average = std::round(static_cast<float>(totalSum) / totalCount);
                                    if (coinRatings.empty() || std::find(coinRatings.begin(), coinRatings.end(), average) != coinRatings.end()) {
                                        validCoinRatingLevels.insert(crLevelId);
                                    }
                                } else if (coinRatings.empty()) {
                                    validCoinRatingLevels.insert(crLevelId);
                                }
                            }
                        }
                    }
                } catch (std::exception& e) {
                    log::error("Exception in fetchFilteredLevels (coinsrating): {}", e.what());
                }
            }
        }

        if (success) {
            std::vector<int> resultLevels;
            
            if (!coinRatings.empty()) {
                // If filtering by Coin Rating, it MUST be in validCoinRatingLevels.
                for (int lvl : validCoinRatingLevels) {
                    if (onlyWithGuide || freeCoins) {
                        // Requires guide (or freecoins flag), so it MUST also be in uniqueLevels
                        if (uniqueLevels.find(lvl) != uniqueLevels.end()) {
                            resultLevels.push_back(lvl);
                        }
                    } else {
                        // All validCoinRatingLevels already matched `ratings` and `quality` filters internally!
                        resultLevels.push_back(lvl);
                    }
                }
            } else {
                // No Coin Rating filter
                if (onlyWithGuide || freeCoins) {
                    // Only what's in uniqueLevels
                    resultLevels.assign(uniqueLevels.begin(), uniqueLevels.end());
                } else {
                    // Union of both tables which have both been filtered internally by `ratings` and `quality`
                    std::set<int> allLevels = uniqueLevels;
                    allLevels.insert(validCoinRatingLevels.begin(), validCoinRatingLevels.end());
                    resultLevels.assign(allLevels.begin(), allLevels.end());
                }
            }
            
            geode::queueInMainThread([callback, resultLevels]() { callback(resultLevels); });
        } else {
            geode::queueInMainThread([callback]() { callback(std::nullopt); });
        }
    }).detach();
}

void Database::submitCoinRating(int levelId, int rating, int levelRating, std::string levelCuality, std::function<void(bool, const std::string&)> callback) {
    std::string selectUrl = fmt::format("{}/rest/v1/coinsrating?levelid=eq.{}&select=*", SUPABASE_URL_LEVELS.substr(0, SUPABASE_URL_LEVELS.find("/rest/v1/levels")), levelId);
    
    std::thread([selectUrl, levelId, rating, levelRating, levelCuality, callback]() {
        web::WebRequest reqGet;
        reqGet.header("apikey", SUPABASE_KEY);
        reqGet.header("Authorization", "Bearer " + SUPABASE_KEY);
        
        auto resGet = reqGet.sendSync("GET", selectUrl);
        if (resGet.code() < 200 || resGet.code() >= 300) {
            geode::queueInMainThread([callback]() { callback(false, "Failed to fetch existing ratings"); });
            return;
        }
        
        bool exists = false;
        int currentVal = 0;
        std::string colName = fmt::format("ranking{}", rating);
        
        auto textGet = resGet.string().unwrapOr("[]");
        try {
            auto parseRes = matjson::parse(textGet);
            if (parseRes.isOk() && parseRes.unwrap().isArray()) {
                auto arr = parseRes.unwrap().asArray().unwrap();
                if (arr.size() > 0) {
                    exists = true;
                    auto row = arr[0];
                    if (row.contains(colName)) {
                        if (row[colName].isNumber()) {
                            currentVal = row[colName].asInt().unwrapOr(0);
                        } else if (row[colName].isString()) {
                            try {
                                currentVal = std::stoi(row[colName].asString().unwrapOr("0"));
                            } catch (...) {}
                        }
                    }
                }
            }
        } catch (std::exception& e) {
            log::error("Exception in submitCoinRating: {}", e.what());
        }

        int weight = 1;
        std::string role = Mod::get()->getSavedValue<std::string>("user_role");
        if (role == "admin" || role == "owner") weight = 10;
        else if (role == "mod") weight = 5;
        
        web::WebRequest reqWrite;
        reqWrite.header("apikey", SUPABASE_KEY);
        reqWrite.header("Authorization", "Bearer " + SUPABASE_KEY);
        reqWrite.header("Content-Type", "application/json");
        reqWrite.header("Prefer", "return=minimal");
        
        matjson::Value body;
        if (exists) {
            body = matjson::makeObject({
                {colName, currentVal + weight},
                {"levelrating", levelRating},
                {"levelcuality", levelCuality}
            });
            std::string patchUrl = fmt::format("{}/rest/v1/coinsrating?levelid=eq.{}", SUPABASE_URL_LEVELS.substr(0, SUPABASE_URL_LEVELS.find("/rest/v1/levels")), levelId);
            reqWrite.bodyJSON(body);
            auto resPatch = reqWrite.sendSync("PATCH", patchUrl);
            if (resPatch.code() < 200 || resPatch.code() >= 300) {
                geode::queueInMainThread([callback]() { callback(false, "Failed to update rating"); });
            } else {
                geode::queueInMainThread([callback]() { callback(true, ""); });
            }
        } else {
            body = matjson::makeObject({
                {"levelid", levelId},
                {colName, weight},
                {"levelrating", levelRating},
                {"levelcuality", levelCuality}
            });
            std::string postUrl = fmt::format("{}/rest/v1/coinsrating", SUPABASE_URL_LEVELS.substr(0, SUPABASE_URL_LEVELS.find("/rest/v1/levels")));
            reqWrite.bodyJSON(body);
            auto resPost = reqWrite.sendSync("POST", postUrl);
            if (resPost.code() < 200 || resPost.code() >= 300) {
                geode::queueInMainThread([callback]() { callback(false, "Failed to insert rating"); });
            } else {
                geode::queueInMainThread([callback]() { callback(true, ""); });
            }
        }
    }).detach();
}

void Database::fetchAverageCoinRating(int levelId, std::function<void(std::optional<std::pair<int, int>>)> callback) {
    std::string selectUrl = fmt::format("{}/rest/v1/coinsrating?levelid=eq.{}&select=*", SUPABASE_URL_LEVELS.substr(0, SUPABASE_URL_LEVELS.find("/rest/v1/levels")), levelId);
    
    std::thread([selectUrl, callback]() {
        web::WebRequest req;
        req.header("apikey", SUPABASE_KEY);
        req.header("Authorization", "Bearer " + SUPABASE_KEY);
        
        auto res = req.sendSync("GET", selectUrl);
        if (res.code() >= 200 && res.code() < 300) {
            try {
                auto text = res.string().unwrapOr("[]");
                auto parseRes = matjson::parse(text);
                if (parseRes.isOk() && parseRes.unwrap().isArray()) {
                    auto arr = parseRes.unwrap().asArray().unwrap();
                    if (arr.size() > 0) {
                        auto row = arr[0];
                        int totalSum = 0;
                        int totalCount = 0;
                        for (int i = 1; i <= 10; ++i) {
                            std::string colName = fmt::format("ranking{}", i);
                            if (row.contains(colName)) {
                                int votes = 0;
                                if (row[colName].isNumber()) {
                                    votes = row[colName].asInt().unwrapOr(0);
                                } else if (row[colName].isString()) {
                                    try {
                                        votes = std::stoi(row[colName].asString().unwrapOr("0"));
                                    } catch (...) {}
                                }
                                totalSum += (i * votes);
                                totalCount += votes;
                            }
                        }
                        if (totalCount > 0) {
                            int average = std::round(static_cast<float>(totalSum) / totalCount);
                            geode::queueInMainThread([callback, average, totalCount]() { callback(std::make_pair(average, totalCount)); });
                            return;
                        }
                    }
                }
            } catch (std::exception& e) {
                log::error("Exception in fetchAverageCoinRating: {}", e.what());
            }
        }
        geode::queueInMainThread([callback]() { callback(std::nullopt); });
    }).detach();
}
