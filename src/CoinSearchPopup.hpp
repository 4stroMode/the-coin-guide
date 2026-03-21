#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class CoinSearchPopup : public FLAlertLayer {
protected:
    CCLayer* m_mainLayer;
    CCSize m_size;
    bool init(float width, float height, const char* bg);
    bool setup();

    // Filter state
    std::vector<int> m_selectedRatings;
    std::vector<int> m_selectedCoinRatings;
    int m_qualityFilter = 0; // 0=any, 1=star, 2=featured, 3=epic, 4=legendary, 5=mythic
    bool m_freeCoins = false;
    bool m_randomList = false;
    bool m_onlyUncompleted = false;
    bool m_onlyWithGuide = false;

    // UI Nodes
    CCMenuItemSpriteExtra* m_qualityBtn;

    void onFilterRatingBtn(CCObject* sender);
    void onQualityToggle(CCObject* sender);
    void onRatingToggle(CCObject* sender);
    void onFreeCoinsToggle(CCObject* sender);
    void onRandomListToggle(CCObject* sender);
    void onOnlyUncompletedToggle(CCObject* sender);
    void onOnlyWithGuideToggle(CCObject* sender);
    void onSearchBtn(CCObject* sender);
    void onInfoBtn(CCObject* sender);
    void onRandomInfoBtn(CCObject* sender);

    void updateQualitySprite();
    void keyBackClicked(CCObject* sender);
    void keyBackClicked() override;

public:
    static CoinSearchPopup* create();
};
