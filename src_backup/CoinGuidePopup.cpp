#include "CoinGuidePopup.hpp"
#include <Geode/binding/ProfilePage.hpp>

CoinGuidePopup* CoinGuidePopup::create(GJGameLevel* level) {
    auto ret = new CoinGuidePopup();
    if (ret && ret->init(level)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CoinGuidePopup::init(GJGameLevel* level) {
    if (!FLAlertLayer::init(75)) return false;
    
    m_level = level;
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_mainLayer = CCLayer::create();
    this->addChild(m_mainLayer);

    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setContentSize(winSize);
    m_buttonMenu->setPosition(0, 0);
    m_buttonMenu->setZOrder(10);
    m_mainLayer->addChild(m_buttonMenu);

    auto bg = extension::CCScale9Sprite::create("GJ_square01.png");
    bg->setContentSize({420.f, 260.f});
    bg->setPosition(winSize / 2);
    m_mainLayer->addChild(bg);
    
    auto title = CCLabelBMFont::create("Coin Guide", "bigFont.fnt");
    title->setPosition({winSize.width / 2, winSize.height / 2 + 110.f});
    m_mainLayer->addChild(title);
    
    auto closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this, menu_selector(CoinGuidePopup::onClose)
    );
    closeBtn->setPosition({winSize.width / 2 - 210.f, winSize.height / 2 + 130.f});
    m_buttonMenu->addChild(closeBtn);

    m_contentMenu = CCMenu::create();
    m_contentMenu->setContentSize(winSize);
    m_contentMenu->setPosition(0, 0);
    m_contentMenu->setZOrder(10);
    m_mainLayer->addChild(m_contentMenu);

    m_loadingLabel = CCLabelBMFont::create("Loading...", "goldFont.fnt");
    m_loadingLabel->setScale(0.6f);
    m_loadingLabel->setPosition(winSize / 2);
    m_mainLayer->addChild(m_loadingLabel);

    // Discord Report Button (exclamation icon)
    auto discordSpr = CCSprite::createWithSpriteFrameName("GJ_reportBtn_001.png");
    discordSpr->setScale(0.8f);
    auto discordBtn = CCMenuItemSpriteExtra::create(discordSpr, this, menu_selector(CoinGuidePopup::onDiscord));
    discordBtn->setPosition({winSize.width / 2 + 185.f, winSize.height / 2 - 105.f});
    m_buttonMenu->addChild(discordBtn);

    // Fetch from database
    Database::fetchLevelData(m_level->m_levelID, [this](std::optional<CoinGuideData> data) {
        geode::queueInMainThread([this, data]() {
            if (this && m_loadingLabel) {
                m_loadingLabel->removeFromParent();
                m_loadingLabel = nullptr;
                this->populateGuide(data);
            }
        });
    });

    return true;
}

void CoinGuidePopup::onDiscord(CCObject*) {
    geode::createQuickPopup("Discord", "Join our Discord server to report invalid guides!", "Cancel", "Join", [](auto, bool btn2) {
        if (btn2) {
            geode::utils::web::openLinkInBrowser("https://discord.gg/jbZEQbcMtQ");
        }
    });
}

void CoinGuidePopup::onClose(CCObject*) {
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
}

void CoinGuidePopup::populateGuide(std::optional<CoinGuideData> data) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (!data.has_value()) {
        auto noDataLabel = CCLabelBMFont::create("No guides found for this level.", "chatFont.fnt");
        noDataLabel->setScale(0.6f);
        noDataLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
        noDataLabel->setPosition(winSize / 2);
        m_mainLayer->addChild(noDataLabel);
        return;
    }

    m_authorAccountId = data->accountid;

    int ncoins = std::min(m_level->m_coins, 3);
    float startY = winSize.height / 2 + 50.f;
    float gap = 60.f;

    for (int i = 0; i < ncoins; ++i) {
        float yPos = startY - (i * gap);
        
        // Coin Icon (uncollected user coin)
        auto coinSpr = CCSprite::createWithSpriteFrameName("secretCoin_2_01_001.png"); 
        coinSpr->setScale(0.8f);
        coinSpr->setPosition({winSize.width / 2 - 170.f, yPos});
        m_mainLayer->addChild(coinSpr);

        // Coin Title
        auto titleLabel = CCLabelBMFont::create(fmt::format("Coin {}", i + 1).c_str(), "goldFont.fnt");
        titleLabel->setScale(0.6f);
        titleLabel->setAnchorPoint({0.f, 0.5f});
        titleLabel->setPosition({winSize.width / 2 - 140.f, yPos + 15.f});
        m_mainLayer->addChild(titleLabel);

        // Coin Text
        std::string text = "";
        if (i == 0) text = data->coin1;
        if (i == 1) text = data->coin2;
        if (i == 2) text = data->coin3;
        
        if (text.empty()) text = "No guide available.";

        auto textLabel = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
        textLabel->setScale(0.6f);
        textLabel->setAnchorPoint({0.f, 0.5f});
        textLabel->setPosition({winSize.width / 2 - 140.f, yPos - 5.f});
        textLabel->limitLabelWidth(300.f, 0.6f, 0.1f);
        m_mainLayer->addChild(textLabel);
    }

    auto creditLabel = CCLabelBMFont::create(fmt::format("Added by: {}", data->addedby).c_str(), "goldFont.fnt");
    creditLabel->setScale(0.5f);
    creditLabel->setAnchorPoint({0.f, 0.f});
    creditLabel->setPosition({winSize.width / 2 - 195.f, winSize.height / 2 - 110.f});
    creditLabel->setZOrder(15);
    m_mainLayer->addChild(creditLabel);
}
