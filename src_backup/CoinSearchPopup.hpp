#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CoinSearchPopup : public Popup<> {
protected:
    bool setup() override;

    // Filter state
    std::vector<int> m_selectedRatings;
    int m_qualityFilter = 0; // 0=any, 1=star, 2=featured, 3=epic, 4=legendary, 5=mythic
    bool m_freeCoins = false;
    bool m_recentlyAdded = false;
    bool m_onlyUncompleted = false;

    // UI Nodes
    CCMenuItemSpriteExtra* m_qualityBtn;

    void onRatingToggle(CCObject* sender);
    void onQualityToggle(CCObject* sender);
    void onFreeCoinsToggle(CCObject* sender);
    void onRecentlyAddedToggle(CCObject* sender);
    void onOnlyUncompletedToggle(CCObject* sender);
    void onSearchBtn(CCObject* sender);
    void onInfoBtn(CCObject* sender);

    void updateQualitySprite();

public:
    static CoinSearchPopup* create();
};
