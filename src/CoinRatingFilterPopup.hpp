#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class CoinRatingFilterPopup : public FLAlertLayer {
protected:
    CCLayer* m_mainLayer;
    CCSize m_size;
    std::vector<int>* m_selectedRatings;

    bool init(std::vector<int>* selectedRatings);
    void setup();
    void onRatingToggle(CCObject* sender);
    void onClose(CCObject* sender);
    void keyBackClicked() override;

public:
    static CoinRatingFilterPopup* create(std::vector<int>* selectedRatings);
};
