#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include "Database.hpp"

using namespace geode::prelude;

#include <algorithm>

class CoinGuidePopup : public FLAlertLayer {
protected:
    GJGameLevel* m_level;
    CCLayer* m_mainLayer;
    CCMenu* m_buttonMenu;
    CCMenu* m_contentMenu;
    CCLabelBMFont* m_loadingLabel;
    int m_authorAccountId = 0;

    bool init(GJGameLevel* level);
    void onDiscord(CCObject*);
    void onClose(CCObject*);
    void populateGuide(std::optional<CoinGuideData> data);

public:
    static CoinGuidePopup* create(GJGameLevel* level);
};
