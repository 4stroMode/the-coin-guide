#include "CoinRatingFilterPopup.hpp"

CoinRatingFilterPopup* CoinRatingFilterPopup::create(std::vector<int>* selectedRatings) {
    auto ret = new CoinRatingFilterPopup();
    if (ret && ret->init(selectedRatings)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CoinRatingFilterPopup::init(std::vector<int>* selectedRatings) {
    if (!FLAlertLayer::init(150)) return false;
    m_selectedRatings = selectedRatings;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    m_size = CCSize{270.f, 150.f};

    auto bgSpr = cocos2d::extension::CCScale9Sprite::create("GJ_square01.png");
    bgSpr->setContentSize(m_size);
    bgSpr->setPosition(winSize / 2);
    this->addChild(bgSpr);

    m_mainLayer = CCLayer::create();
    m_mainLayer->setPosition((winSize - m_size) / 2);
    this->addChild(m_mainLayer);

    auto closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this, menu_selector(CoinRatingFilterPopup::onClose)
    );
    closeBtn->setPosition({5.f, m_size.height - 5.f});
    auto closeMenu = CCMenu::create();
    closeMenu->setPosition((winSize - m_size) / 2);
    closeMenu->addChild(closeBtn);
    this->addChild(closeMenu, 10);

    setup();
    
    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    return true;
}

void CoinRatingFilterPopup::setup() {
    auto title = CCLabelBMFont::create("Filter Coins Rating", "goldFont.fnt");
    title->setPosition({m_size.width / 2, m_size.height - 15.f});
    title->setScale(0.7f);
    m_mainLayer->addChild(title);

    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);

    float startX = 45.f;
    float startY = m_size.height - 60.f;

    for (int i = 1; i <= 10; ++i) {
        auto offSpr = ButtonSprite::create(std::to_string(i).c_str(), 20, true, "bigFont.fnt", "GJ_button_04.png", 30, 0.6f);
        auto onSpr = ButtonSprite::create(std::to_string(i).c_str(), 20, true, "bigFont.fnt", "GJ_button_02.png", 30, 0.6f);
        
        ccColor3B textColor;
        if (i <= 2) textColor = {50, 150, 255};      // Blue
        else if (i <= 4) textColor = {0, 255, 0};    // Green
        else if (i <= 6) textColor = {255, 255, 0};  // Yellow
        else if (i <= 8) textColor = {255, 150, 0};  // Orange
        else if (i == 9) textColor = {200, 100, 255};// Light Purple
        else textColor = {255, 0, 0};                // Red
        
        if (auto lbl = offSpr->getChildByType<CCLabelBMFont*>(0)) lbl->setColor(textColor);
        if (auto lbl = onSpr->getChildByType<CCLabelBMFont*>(0)) lbl->setColor(textColor);

        auto toggler = CCMenuItemToggler::create(offSpr, onSpr, this, menu_selector(CoinRatingFilterPopup::onRatingToggle));
        toggler->setTag(i);
        
        if (std::find(m_selectedRatings->begin(), m_selectedRatings->end(), i) != m_selectedRatings->end()) {
            toggler->toggle(true);
        }

        int row = (i - 1) / 5;
        int col = (i - 1) % 5;
        toggler->setPosition({startX + col * 45.f, startY - row * 40.f});
        menu->addChild(toggler);
    }
}

void CoinRatingFilterPopup::onRatingToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    int tag = toggler->getTag();
    if (!toggler->isToggled()) {
        m_selectedRatings->push_back(tag);
    } else {
        auto it = std::find(m_selectedRatings->begin(), m_selectedRatings->end(), tag);
        if (it != m_selectedRatings->end()) {
            m_selectedRatings->erase(it);
        }
    }
}

void CoinRatingFilterPopup::onClose(CCObject*) {
    keyBackClicked();
}

void CoinRatingFilterPopup::keyBackClicked() {
    this->setKeyboardEnabled(false);
    this->removeFromParentAndCleanup(true);
}
