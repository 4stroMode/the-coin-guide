#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/ui/TextInput.hpp>
#include "Database.hpp"

using namespace geode::prelude;

#include <algorithm>

class CoinSubmitPopup : public FLAlertLayer {
protected:
    GJGameLevel* m_level;
    CCLayer* m_mainLayer;
    CCMenu* m_buttonMenu;
    geode::TextInput* m_coin1Input;
    geode::TextInput* m_coin2Input;
    geode::TextInput* m_coin3Input;
    CCMenuItemToggler* m_freeCoinToggle;

    bool init(GJGameLevel* level);
    void onSubmit(CCObject*);
    void onDelete(CCObject*);
    void onClose(CCObject*);

public:
    static CoinSubmitPopup* create(GJGameLevel* level);
};
