#include "CoinRatingPopup.hpp"
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

CoinRatingPopup* CoinRatingPopup::create(GJGameLevel* level) {
    auto ret = new CoinRatingPopup();
    if (ret && ret->init(level)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CoinRatingPopup::init(GJGameLevel* level) {
    if (!FLAlertLayer::init(150)) return false;
    
    m_level = level;
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_mainLayer = CCLayer::create();
    this->addChild(m_mainLayer);

    auto bg = extension::CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({340.f, 180.f});
    bg->setPosition(winSize / 2);
    m_mainLayer->addChild(bg, -1);
    
    // Title
    auto title = CCLabelBMFont::create("Coins Rating", "bigFont.fnt");
    title->setPosition({winSize.width / 2, winSize.height / 2 + 70.f});
    title->setScale(0.6f);
    m_mainLayer->addChild(title);

    // Subtitle
    auto subtitle = CCLabelBMFont::create("1 Easy - 10 Extreme", "chatFont.fnt");
    subtitle->setPosition({winSize.width / 2, winSize.height / 2 + 50.f});
    subtitle->setScale(0.5f);
    m_mainLayer->addChild(subtitle);
    
    // Create menu for radio buttons
    auto btnMenu = CCMenu::create();
    btnMenu->setPosition({winSize.width / 2, winSize.height / 2});
    m_mainLayer->addChild(btnMenu);
    
    float startX = -100.f;
    float y1 = 20.f;
    float y2 = -20.f;
    
    for (int i = 1; i <= 10; i++) {
        // Sprite - size 30 instead of 35, scale 0.5
        auto spr = ButtonSprite::create(std::to_string(i).c_str(), 30, true, "bigFont.fnt", "GJ_button_04.png", 0, 0.5f);
        
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(CoinRatingPopup::onRatingToggled));
        btn->setTag(i);
        
        float xPos = startX + ((i - 1) % 5) * 50.f;
        float yPos = (i <= 5) ? y1 : y2;
        btn->setPosition({xPos, yPos});
        
        btnMenu->addChild(btn);
        m_ratingBtns.push_back(btn);
    }
    
    // Submit button
    auto submitSpr = ButtonSprite::create("Submit", "bigFont.fnt", "GJ_button_01.png", 0.6f);
    m_submitBtn = CCMenuItemSpriteExtra::create(submitSpr, this, menu_selector(CoinRatingPopup::onSubmit));
    m_submitBtn->setPosition({50.f, -65.f});
    m_submitBtn->setEnabled(false);
    m_submitBtn->setColor({100, 100, 100}); // Grayed out initially
    btnMenu->addChild(m_submitBtn);

    // Cancel button
    auto cancelSpr = ButtonSprite::create("Cancel", "bigFont.fnt", "GJ_button_06.png", 0.6f);
    auto cancelBtn = CCMenuItemSpriteExtra::create(cancelSpr, this, menu_selector(CoinRatingPopup::onClose));
    cancelBtn->setPosition({-50.f, -65.f});
    btnMenu->addChild(cancelBtn);

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);
    
    return true;
}

void CoinRatingPopup::show() {
    FLAlertLayer::show();
    m_mainLayer->setScale(0.1f);
    m_mainLayer->runAction(CCEaseElasticOut::create(CCScaleTo::create(0.5f, 1.0f), 0.6f));
}

void CoinRatingPopup::onRatingToggled(CCObject* sender) {
    auto clickedBtn = static_cast<CCMenuItemSpriteExtra*>(sender);
    m_selectedRating = clickedBtn->getTag();
    
    // Update visuals
    for (auto btn : m_ratingBtns) {
        if (auto spr = static_cast<ButtonSprite*>(btn->getNormalImage())) {
            if (btn->getTag() == m_selectedRating) {
                spr->updateBGImage("GJ_button_02.png");
            } else {
                spr->updateBGImage("GJ_button_04.png");
            }
        }
    }
    
    m_submitBtn->setEnabled(true);
    m_submitBtn->setColor({255, 255, 255});
}

void CoinRatingPopup::onSubmit(CCObject* sender) {
    if (m_selectedRating == -1) return;
    
    m_submitBtn->setEnabled(false);
    
    auto loading = FLAlertLayer::create("Loading", "Evaluating coins...", "OK");
    loading->show();

    int levelRating = m_level->m_stars;
    if (m_level->m_demon > 0) {
        switch (m_level->m_demonDifficulty) {
            case 3: levelRating = 10; break;
            case 4: levelRating = 11; break;
            case 0: levelRating = 12; break; // Hard demon
            case 5: levelRating = 13; break;
            case 6: levelRating = 14; break;
            default: levelRating = 12; break;
        }
    }
    
    std::string levelCuality = "";
    if (m_level->m_isEpic == 3) levelCuality = "mythic";
    else if (m_level->m_isEpic == 2) levelCuality = "legendary";
    else if (m_level->m_isEpic == 1) levelCuality = "epic";
    else if (m_level->m_featured > 0) levelCuality = "featured";
    else if (m_level->m_stars > 0) levelCuality = "star";

    Database::submitCoinRating(m_level->m_levelID, m_selectedRating, levelRating, levelCuality, [this, loading](bool success, const std::string& error) {
        geode::queueInMainThread([this, loading, success, error]() {
            loading->keyBackClicked();
            if (success) {
                Mod::get()->setSavedValue<bool>(fmt::format("rated_coins_{}", m_level->m_levelID), true);
                
                if (auto btn = static_cast<CCMenuItemSpriteExtra*>(CCDirector::sharedDirector()->getRunningScene()->getChildByIDRecursive("coin-rating-btn"_spr))) {
                    if (auto spr = static_cast<CCSprite*>(btn->getNormalImage())) {
                        spr->setColor({100, 100, 100});
                    }
                }

                FLAlertLayer::create("Success", "Your coin difficulty rating has been submitted!", "OK")->show();
                this->onClose(nullptr);
            } else {
                FLAlertLayer::create("Error", fmt::format("Failed to submit rating: {}", error), "OK")->show();
                m_submitBtn->setEnabled(true);
            }
        });
    });
}

void CoinRatingPopup::onClose(CCObject* sender) {
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}
