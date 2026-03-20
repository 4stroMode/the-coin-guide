#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include "Database.hpp"

using namespace geode::prelude;

class CoinRatingPopup : public FLAlertLayer {
protected:
    GJGameLevel* m_level;
    CCLayer* m_mainLayer;
    int m_selectedRating = -1;
    CCMenuItemSpriteExtra* m_submitBtn = nullptr;
    std::vector<CCMenuItemSpriteExtra*> m_ratingBtns;

    bool init(GJGameLevel* level);
    void onRatingToggled(CCObject* sender);
    void onSubmit(CCObject* sender);
    void onClose(CCObject* sender);

public:
    static CoinRatingPopup* create(GJGameLevel* level);
    void show() override;
};
