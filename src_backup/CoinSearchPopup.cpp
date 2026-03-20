#include "CoinSearchPopup.hpp"
#include "Database.hpp"
#include <algorithm>

CoinSearchPopup* CoinSearchPopup::create() {
    auto ret = new CoinSearchPopup();
    if (ret && ret->initAnchored(380.f, 260.f, "GJ_square05.png")) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CoinSearchPopup::setup() {
    auto title = CCLabelBMFont::create("Search filters", "bigFont.fnt");
    title->setPosition({m_size.width / 2, m_size.height - 20.f});
    title->setScale(0.7f);
    m_mainLayer->addChild(title);

    auto topTitle = CCLabelBMFont::create("The Coin Guide", "goldFont.fnt");
    topTitle->setPosition({m_size.width / 2, m_size.height + 15.f});
    topTitle->setScale(0.9f);
    m_mainLayer->addChild(topTitle);

    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);

    float startX = 60.f;
    float startY = m_size.height - 70.f;
    for (int i = 1; i <= 10; ++i) {
        auto offSpr = ButtonSprite::create(std::to_string(i).c_str(), 20, true, "bigFont.fnt", "GJ_button_04.png", 30, 0.6f);
        auto onSpr = ButtonSprite::create(std::to_string(i).c_str(), 20, true, "bigFont.fnt", "GJ_button_02.png", 30, 0.6f);
        auto toggler = CCMenuItemToggler::create(offSpr, onSpr, this, menu_selector(CoinSearchPopup::onRatingToggle));
        toggler->setTag(i);
        int row = (i - 1) / 5;
        int col = (i - 1) % 5;
        toggler->setPosition({startX + col * 45.f, startY - row * 40.f});
        menu->addChild(toggler);
    }

    m_qualityBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("difficulty_00_btn_001.png"), this, menu_selector(CoinSearchPopup::onQualityToggle));
    m_qualityBtn->setPosition({m_size.width - 50.f, m_size.height - 90.f});
    menu->addChild(m_qualityBtn);
    updateQualitySprite();

    float toggleY = 100.f;
    float toggleX = 40.f;
    float spacingY = 32.f;

    auto freeCoinToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(CoinSearchPopup::onFreeCoinsToggle), 0.6f);
    freeCoinToggle->setPosition({toggleX, toggleY});
    menu->addChild(freeCoinToggle);
    auto freeLbl = CCLabelBMFont::create("Free coins", "bigFont.fnt");
    freeLbl->setPosition({toggleX + 20.f, toggleY});
    freeLbl->setAnchorPoint({0, 0.5f});
    freeLbl->setScale(0.5f);
    m_mainLayer->addChild(freeLbl);
    
    auto infoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    infoSpr->setScale(0.6f);
    auto infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(CoinSearchPopup::onInfoBtn));
    infoBtn->setPosition({toggleX + freeLbl->getScaledContentSize().width + 35.f, toggleY});
    menu->addChild(infoBtn);

    auto recentToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(CoinSearchPopup::onRecentlyAddedToggle), 0.6f);
    recentToggle->setPosition({toggleX, toggleY - spacingY});
    menu->addChild(recentToggle);
    auto recentLbl = CCLabelBMFont::create("Recently added", "bigFont.fnt");
    recentLbl->setPosition({toggleX + 20.f, toggleY - spacingY});
    recentLbl->setAnchorPoint({0, 0.5f});
    recentLbl->setScale(0.5f);
    m_mainLayer->addChild(recentLbl);

    auto uncompToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(CoinSearchPopup::onOnlyUncompletedToggle), 0.6f);
    uncompToggle->setPosition({toggleX, toggleY - spacingY * 2});
    menu->addChild(uncompToggle);
    auto uncompLbl = CCLabelBMFont::create("Only uncompleated", "bigFont.fnt");
    uncompLbl->setPosition({toggleX + 20.f, toggleY - spacingY * 2});
    uncompLbl->setAnchorPoint({0, 0.5f});
    uncompLbl->setScale(0.5f);
    m_mainLayer->addChild(uncompLbl);

    auto searchSpr = CCSprite::createWithSpriteFrameName("gj_findBtn_001.png");
    auto searchBtn = CCMenuItemSpriteExtra::create(searchSpr, this, menu_selector(CoinSearchPopup::onSearchBtn));
    searchBtn->setPosition({m_size.width - 45.f, 35.f});
    menu->addChild(searchBtn);

    return true;
}

void CoinSearchPopup::onRatingToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    int tag = toggler->getTag();
    if (!toggler->isToggled()) {
        m_selectedRatings.push_back(tag);
    } else {
        auto it = std::find(m_selectedRatings.begin(), m_selectedRatings.end(), tag);
        if (it != m_selectedRatings.end()) {
            m_selectedRatings.erase(it);
        }
    }
}

void CoinSearchPopup::onQualityToggle(CCObject* sender) {
    m_qualityFilter = (m_qualityFilter + 1) % 6;
    updateQualitySprite();
}

void CoinSearchPopup::updateQualitySprite() {
    auto spr = static_cast<CCSprite*>(m_qualityBtn->getNormalImage());
    spr->setColor({255, 255, 255});
    m_qualityBtn->removeAllChildren();
    
    spr->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("difficulty_00_btn_001.png"));
    
    if (m_qualityFilter == 0) {
        spr->setColor({100, 100, 100}); 
    } else if (m_qualityFilter == 1) {
    } else if (m_qualityFilter == 2) {
        auto featureSpr = CCSprite::createWithSpriteFrameName("GJ_featuredCoin_001.png");
        featureSpr->setPosition(spr->getContentSize() / 2);
        featureSpr->setZOrder(-1);
        m_qualityBtn->addChild(featureSpr);
    } else if (m_qualityFilter == 3) {
        auto epicSpr = CCSprite::createWithSpriteFrameName("GJ_epicCoin_001.png");
        epicSpr->setPosition(spr->getContentSize() / 2);
        epicSpr->setZOrder(-1);
        m_qualityBtn->addChild(epicSpr);
    } else if (m_qualityFilter == 4) {
        auto legSpr = CCSprite::createWithSpriteFrameName("GJ_legendaryCoin_001.png");
        legSpr->setPosition(spr->getContentSize() / 2);
        legSpr->setZOrder(-1);
        m_qualityBtn->addChild(legSpr);
    } else if (m_qualityFilter == 5) {
        auto mythicSpr = CCSprite::createWithSpriteFrameName("GJ_mythicCoin_001.png");
        mythicSpr->setPosition(spr->getContentSize() / 2);
        mythicSpr->setZOrder(-1);
        m_qualityBtn->addChild(mythicSpr);
    }
}

void CoinSearchPopup::onFreeCoinsToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    m_freeCoins = !toggler->isToggled();
}

void CoinSearchPopup::onRecentlyAddedToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    m_recentlyAdded = !toggler->isToggled();
}

void CoinSearchPopup::onOnlyUncompletedToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    m_onlyUncompleted = !toggler->isToggled();
}

void CoinSearchPopup::onInfoBtn(CCObject* sender) {
    FLAlertLayer::create(
        "Free Coins info",
        "Si marcas la opcion de free coins saldran solo niveles marcados como que tienen coins gratis",
        "OK"
    )->show();
}

void CoinSearchPopup::onSearchBtn(CCObject* sender) {
    auto loadingIndicator = LoadingSpinner::create(30.f);
    loadingIndicator->setPosition({m_size.width / 2, m_size.height / 2});
    loadingIndicator->setID("loading-spinner");
    m_mainLayer->addChild(loadingIndicator);
    
    Database::fetchFilteredLevels(
        m_selectedRatings, 
        m_qualityFilter, 
        m_freeCoins, 
        m_recentlyAdded, 
        [this](std::optional<std::vector<int>> levelsOpt) {
            if (auto spinner = m_mainLayer->getChildByID("loading-spinner")) {
                spinner->removeFromParent();
            }
            if (!levelsOpt.has_value()) {
                FLAlertLayer::create("Error", "Could not fetch levels from Database.", "OK")->show();
                return;
            }
            
            auto levels = levelsOpt.value();
            
            if (m_onlyUncompleted) {
                std::vector<int> filtered;
                for (int id : levels) {
                    auto saved = GameLevelManager::sharedState()->getSavedLevel(id);
                    if (!saved || saved->m_normalPercent < 100) {
                        filtered.push_back(id);
                    }
                }
                levels = filtered;
            }
            
            if (levels.empty()) {
                FLAlertLayer::create("Coin Guide", "No levels found matching those filters.", "OK")->show();
                return;
            }
            
            std::string searchIds = "";
            for (size_t i = 0; i < levels.size(); ++i) {
                searchIds += std::to_string(levels[i]);
                if (i < levels.size() - 1) searchIds += ",";
            }
            
            auto searchObj = GJSearchObject::create(SearchType::Type19, searchIds);
            auto scene = LevelBrowserLayer::scene(searchObj);
            CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
        }
    );
}
