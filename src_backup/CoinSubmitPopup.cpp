#include "CoinSubmitPopup.hpp"

CoinSubmitPopup* CoinSubmitPopup::create(GJGameLevel* level) {
    auto ret = new CoinSubmitPopup();
    if (ret && ret->init(level)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CoinSubmitPopup::init(GJGameLevel* level) {
    if (!FLAlertLayer::init(75)) return false;
    
    m_level = level;
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_mainLayer = CCLayer::create();
    this->addChild(m_mainLayer);

    auto loadingTick = CCLabelBMFont::create("...", "chatFont.fnt");
    loadingTick->setPosition({winSize.width / 2 + 170.f, winSize.height / 2 + 110.f});
    this->m_mainLayer->addChild(loadingTick);

    Database::fetchLevelData(level->m_levelID, [this, winSize, loadingTick](std::optional<CoinGuideData> data) {
        geode::queueInMainThread([this, winSize, loadingTick, data]() {
            loadingTick->removeFromParent();
            if (data.has_value()) {
                auto tick = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
                tick->setPosition({winSize.width / 2 + 170.f, winSize.height / 2 + 110.f});
                tick->setZOrder(15);
                this->m_mainLayer->addChild(tick);
            } else {
                auto cross = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
                cross->setScale(0.8f);
                cross->setPosition({winSize.width / 2 + 170.f, winSize.height / 2 + 110.f});
                cross->setZOrder(15);
                this->m_mainLayer->addChild(cross);
            }
        });
    });

    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setContentSize(winSize);
    m_buttonMenu->setPosition(0, 0);
    m_buttonMenu->setZOrder(10);
    m_mainLayer->addChild(m_buttonMenu);

    auto bg = extension::CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({420.f, 260.f});
    bg->setPosition(winSize / 2);
    m_mainLayer->addChild(bg, -1);
    
    auto title = CCLabelBMFont::create("Mod: Coins Guide", "bigFont.fnt");
    title->setPosition({winSize.width / 2, winSize.height / 2 + 110.f});
    m_mainLayer->addChild(title);
    
    auto closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this, menu_selector(CoinSubmitPopup::onClose)
    );
    closeBtn->setPosition({winSize.width / 2 - 210.f, winSize.height / 2 + 130.f});
    m_buttonMenu->addChild(closeBtn);

    int ncoins = std::min(m_level->m_coins, 3);
    float startY = winSize.height / 2 + 60.f;
    float gap = 45.f;

    m_coin1Input = nullptr;
    m_coin2Input = nullptr;
    m_coin3Input = nullptr;

    for (int i = 0; i < ncoins; ++i) {
        float yPos = startY - (i * gap);
        
        auto label = CCLabelBMFont::create(fmt::format("Coin {}", i + 1).c_str(), "goldFont.fnt");
        label->setScale(0.6f);
        label->setAnchorPoint({1.f, 0.5f});
        label->setPosition({winSize.width / 2 - 120.f, yPos});
        m_mainLayer->addChild(label);

        auto input = geode::TextInput::create(260.f, "Enter coin guide...", "chatFont.fnt");
        input->setPosition({winSize.width / 2 + 20.f, yPos});
        input->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .,!?'\"-()");
        input->setMaxCharCount(150);
        m_mainLayer->addChild(input);

        if (i == 0) m_coin1Input = input;
        if (i == 1) m_coin2Input = input;
        if (i == 2) m_coin3Input = input;
    }

    auto freecoinLabel = CCLabelBMFont::create("Free coin/s", "chatFont.fnt");
    freecoinLabel->setScale(0.5f);
    freecoinLabel->setPosition({winSize.width / 2 - 20.f, winSize.height / 2 - 70.f});
    m_mainLayer->addChild(freecoinLabel);

    m_freeCoinToggle = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        this, nullptr
    );
    m_freeCoinToggle->setScale(0.7f);
    m_freeCoinToggle->setPosition({winSize.width / 2 + 35.f, winSize.height / 2 - 70.f});
    m_buttonMenu->addChild(m_freeCoinToggle);

    auto submitSpr = ButtonSprite::create("Submit", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    auto submitBtn = CCMenuItemSpriteExtra::create(submitSpr, this, menu_selector(CoinSubmitPopup::onSubmit));
    submitBtn->setPosition({winSize.width / 2, winSize.height / 2 - 105.f});
    m_buttonMenu->addChild(submitBtn);

    std::string userRole = Mod::get()->getSavedValue<std::string>("user_role", "");
    if (userRole == "owner" || userRole == "admin" || userRole == "mod") {
        auto deleteSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
        deleteSpr->setScale(0.8f);
        auto deleteBtn = CCMenuItemSpriteExtra::create(deleteSpr, this, menu_selector(CoinSubmitPopup::onDelete));
        deleteBtn->setPosition({winSize.width / 2 + 185.f, winSize.height / 2 - 105.f});
        m_buttonMenu->addChild(deleteBtn);
    }

    return true;
}

void CoinSubmitPopup::onClose(CCObject*) {
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}


void CoinSubmitPopup::onSubmit(CCObject*) {
    if (!m_coin1Input || m_coin1Input->getString().empty()) {
        FLAlertLayer::create("Error", "You must at least provide a guide for Coin 1.", "OK")->show();
        return;
    }

    std::string accountName = GJAccountManager::sharedState()->m_username.c_str();

    int rating = 0;
    if (m_level->m_stars > 0) {
        rating = m_level->m_stars;
        if (m_level->m_demon > 0) {
            int demonDiff = m_level->m_demonDifficulty;
            if (demonDiff == 3) rating = 10;
            else if (demonDiff == 4) rating = 11;
            else if (demonDiff == 0 || demonDiff == 2) rating = 12;
            else if (demonDiff == 5) rating = 13;
            else if (demonDiff == 6) rating = 14;
            else rating = 10;
        }
    }
    
    std::string cuality = "";
    if (m_level->m_stars > 0) {
        cuality = "star";
        if (m_level->m_featured > 0) cuality = "featured";
        if (m_level->m_isEpic == 1) cuality = "epic";
        else if (m_level->m_isEpic == 2) cuality = "legendary";
        else if (m_level->m_isEpic == 3) cuality = "mythic";
    }

    CoinGuideData data;
    data.levelId = m_level->m_levelID;
    data.levelName = m_level->m_levelName;
    data.ncoins = m_level->m_coins;
    
    if (m_coin1Input) data.coin1 = m_coin1Input->getString();
    if (m_coin2Input) data.coin2 = m_coin2Input->getString();
    if (m_coin3Input) data.coin3 = m_coin3Input->getString();
    data.addedby = accountName;
    data.accountid = GJAccountManager::sharedState()->m_accountID;
    data.rating = rating;
    data.cuality = cuality;
    data.freecoin = (m_freeCoinToggle && m_freeCoinToggle->isToggled()) ? 1 : 0;

    auto loading = FLAlertLayer::create("Submitting", "Submitting your guide...", "OK");
    loading->show();

    Database::upsertLevelData(data, [this, loading](bool success) {
        geode::queueInMainThread([this, loading, success]() {
            loading->keyBackClicked(); // close loading alert
            if (success) {
                FLAlertLayer::create("Success", "Guide submitted successfully!", "OK")->show();
                this->onClose(nullptr);
            } else {
                FLAlertLayer::create("Error", "Failed to submit guide. Please try again later.", "OK")->show();
            }
        });
    });
}

void CoinSubmitPopup::onDelete(CCObject*) {
    geode::createQuickPopup("Delete Guide", "Are you sure you want to delete this guide?", "Cancel", "Delete", [this](auto a, bool btn2) {
        if (btn2) {
            auto loading = FLAlertLayer::create("Deleting", "Deleting your guide...", "OK");
            loading->show();
            Database::deleteLevelData(m_level->m_levelID, [this, loading](bool success) {
                loading->keyBackClicked();
                if (success) {
                    FLAlertLayer::create("Success", "Guide deleted successfully.", "OK")->show();
                    this->onClose(nullptr);
                } else {
                    FLAlertLayer::create("Error", "Failed to delete guide.", "OK")->show();
                }
            });
        }
    });
}
