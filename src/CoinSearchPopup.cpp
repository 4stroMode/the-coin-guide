#include "CoinSearchPopup.hpp"
#include "CoinRatingFilterPopup.hpp"
#include "Database.hpp"
#include <algorithm>
#include <random>

CoinSearchPopup* CoinSearchPopup::create() {
    auto ret = new CoinSearchPopup();
    if (ret && ret->init(380.f, 260.f, "GJ_square05.png")) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CoinSearchPopup::init(float width, float height, const char* bg) {
    if (!FLAlertLayer::init(150)) return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    m_size = CCSize{width, height};

    auto bgSpr = cocos2d::extension::CCScale9Sprite::create(bg);
    bgSpr->setContentSize(m_size);
    bgSpr->setPosition(winSize / 2);
    this->addChild(bgSpr);

    m_mainLayer = CCLayer::create();
    m_mainLayer->setPosition((winSize - m_size) / 2);
    this->addChild(m_mainLayer);

    auto closeBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this, menu_selector(CoinSearchPopup::keyBackClicked)
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

void CoinSearchPopup::keyBackClicked(CCObject* sender) {
    keyBackClicked();
}

void CoinSearchPopup::keyBackClicked() {
    this->setKeyboardEnabled(false);
    this->removeFromParentAndCleanup(true);
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
    float startY = m_size.height - 50.f;
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

    auto container = CCNode::create();
    container->setContentSize({40.f, 40.f});

    auto faceNode = CCSprite::createWithSpriteFrameName("diffIcon_00_btn_001.png");
    faceNode->setPosition({20.f, 20.f});
    faceNode->setID("face");
    container->addChild(faceNode, 1);

    m_qualityBtn = CCMenuItemSpriteExtra::create(container, this, menu_selector(CoinSearchPopup::onQualityToggle));
    m_qualityBtn->setPosition({m_size.width - 50.f, m_size.height - 80.f});
    menu->addChild(m_qualityBtn);
    updateQualitySprite();

    float toggleY = 130.f;
    float toggleX = 40.f;
    float spacingY = 26.f;

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
    
    auto randomToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(CoinSearchPopup::onRandomListToggle), 0.6f);
    randomToggle->setPosition({toggleX, toggleY - spacingY});
    menu->addChild(randomToggle);
    auto randomLbl = CCLabelBMFont::create("Random list", "bigFont.fnt");
    randomLbl->setPosition({toggleX + 20.f, toggleY - spacingY});
    randomLbl->setAnchorPoint({0, 0.5f});
    randomLbl->setScale(0.5f);
    m_mainLayer->addChild(randomLbl);

    auto randInfoSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    randInfoSpr->setScale(0.6f);
    auto randInfoBtn = CCMenuItemSpriteExtra::create(randInfoSpr, this, menu_selector(CoinSearchPopup::onRandomInfoBtn));
    randInfoBtn->setPosition({toggleX + randomLbl->getScaledContentSize().width + 35.f, toggleY - spacingY});
    menu->addChild(randInfoBtn);

    auto uncompToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(CoinSearchPopup::onOnlyUncompletedToggle), 0.6f);
    uncompToggle->setPosition({toggleX, toggleY - spacingY * 2});
    menu->addChild(uncompToggle);
    auto uncompLbl = CCLabelBMFont::create("Only Uncompleted", "bigFont.fnt");
    uncompLbl->setPosition({toggleX + 20.f, toggleY - spacingY * 2});
    uncompLbl->setAnchorPoint({0, 0.5f});
    uncompLbl->setScale(0.5f);
    m_mainLayer->addChild(uncompLbl);

    auto guideToggle = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(CoinSearchPopup::onOnlyWithGuideToggle), 0.6f);
    guideToggle->setPosition({toggleX, toggleY - spacingY * 3});
    menu->addChild(guideToggle);
    auto guideLbl = CCLabelBMFont::create("Only levels with guide", "bigFont.fnt");
    guideLbl->setPosition({toggleX + 20.f, toggleY - spacingY * 3});
    guideLbl->setAnchorPoint({0, 0.5f});
    guideLbl->setScale(0.5f);
    m_mainLayer->addChild(guideLbl);

    auto plusSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    plusSpr->setScale(0.7f);
    auto filterBtnNode = CCMenuItemSpriteExtra::create(plusSpr, this, menu_selector(CoinSearchPopup::onFilterRatingBtn));
    filterBtnNode->setPosition({toggleX, toggleY - spacingY * 4});
    menu->addChild(filterBtnNode);
    auto filterLbl = CCLabelBMFont::create("Filter Coins Rating", "bigFont.fnt");
    filterLbl->setPosition({toggleX + 20.f, toggleY - spacingY * 4});
    filterLbl->setAnchorPoint({0, 0.5f});
    filterLbl->setScale(0.5f);
    m_mainLayer->addChild(filterLbl);

    auto searchSpr = CCSprite::createWithSpriteFrameName("gj_findBtn_001.png");
    auto searchBtn = CCMenuItemSpriteExtra::create(searchSpr, this, menu_selector(CoinSearchPopup::onSearchBtn));
    searchBtn->setPosition({m_size.width - 45.f, 35.f});
    menu->addChild(searchBtn);

    return true;
}

void CoinSearchPopup::onFilterRatingBtn(CCObject* sender) {
    CoinRatingFilterPopup::create(&m_selectedCoinRatings)->show();
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
    auto container = m_qualityBtn->getNormalImage();
    auto faceNode = static_cast<CCSprite*>(container->getChildByID("face"));
    
    if (auto prevRing = faceNode->getChildByID("ring")) {
        prevRing->removeFromParent();
    }
    
    faceNode->setColor({80, 80, 80}); 

    const char* ringName = nullptr;

    if (m_qualityFilter == 0) {
        faceNode->setColor({80, 80, 80}); 
    } else if (m_qualityFilter == 1) {
        faceNode->setColor({255, 255, 255});
    } else if (m_qualityFilter == 2) {
        faceNode->setColor({255, 255, 255});
        ringName = "GJ_featuredCoin_001.png";
    } else if (m_qualityFilter == 3) {
        faceNode->setColor({255, 255, 255});
        ringName = "GJ_epicCoin_001.png";
    } else if (m_qualityFilter == 4) {
        faceNode->setColor({255, 255, 255});
        ringName = "GJ_epicCoin2_001.png";
    } else if (m_qualityFilter == 5) {
        faceNode->setColor({255, 255, 255});
        ringName = "GJ_epicCoin3_001.png";
    }

    if (ringName) {
        auto ring = CCSprite::createWithSpriteFrameName(ringName);
        if (ring) {
            ring->setPosition({faceNode->getContentSize().width / 2.f - 0.1f, faceNode->getContentSize().height / 2.f - 5.5f});
            ring->setID("ring");
            faceNode->addChild(ring, -1);
        }
    }
}

void CoinSearchPopup::onFreeCoinsToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    m_freeCoins = !toggler->isToggled();
}

void CoinSearchPopup::onRandomListToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    m_randomList = !toggler->isToggled();
}

void CoinSearchPopup::onOnlyUncompletedToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    m_onlyUncompleted = !toggler->isToggled();
}

void CoinSearchPopup::onOnlyWithGuideToggle(CCObject* sender) {
    auto toggler = static_cast<CCMenuItemToggler*>(sender);
    m_onlyWithGuide = !toggler->isToggled();
}

void CoinSearchPopup::onInfoBtn(CCObject* sender) {
    FLAlertLayer::create(
        "Free Coins info",
        "Only shows levels with free coins.",
        "OK"
    )->show();
}

void CoinSearchPopup::onRandomInfoBtn(CCObject* sender) {
    FLAlertLayer::create(
        "Random List info",
        "Put all the levels in random order.",
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
        m_selectedCoinRatings, // newly passed coin ratings filter
        m_qualityFilter, 
        m_freeCoins, 
        false, // Recently added is now deprecated in the UI, replaced by local random
        m_onlyWithGuide,
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
            
            if (m_randomList && !levels.empty()) {
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(levels.begin(), levels.end(), g);
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
