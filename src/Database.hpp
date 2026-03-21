#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

struct CoinGuideData {
    int levelId;
    std::string levelName;
    int ncoins;
    std::string coin1;
    std::string coin2;
    std::string coin3;
    std::string addedby;
    int accountid;
    int rating;
    std::string cuality;
    int freecoin;
};

class Database {
public:
    static void fetchLevelData(int levelId, std::function<void(std::optional<CoinGuideData>)> callback);
    static void upsertLevelData(CoinGuideData data, std::function<void(bool)> callback);
    static void deleteLevelData(int levelId, std::function<void(bool)> callback);
    static void submitCoinRating(int levelId, int rating, int levelRating, std::string levelCuality, std::function<void(bool, const std::string&)> callback);
    static void fetchAverageCoinRating(int levelId, std::function<void(std::optional<std::pair<int, int>>)> callback);

    static void fetchUserRole(int accountId, std::function<void(std::optional<std::string>)> callback);
    static void addModUser(int accountId, std::string const& name, std::string const& role, std::function<void(bool)> callback);
    static void deleteUserRole(int accountId, std::function<void(bool)> callback);
    static void fetchFreeCoinLevels(std::function<void(std::optional<std::vector<int>>)> callback);
    static void fetchFilteredLevels(std::vector<int> ratings, std::vector<int> coinRatings, int quality, bool freeCoins, bool recentlyAdded, bool onlyWithGuide, std::function<void(std::optional<std::vector<int>>)> callback);
};
