#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/GJUserScore.hpp>
#include <Geode/binding/MenuLayer.hpp>

using namespace geode::prelude;

#include "Database.hpp"
#include "CoinGuidePopup.hpp"
#include "CoinSubmitPopup.hpp"
#include "CoinSearchPopup.hpp"

class AdminAddUserPopup : public FLAlertLayer {
protected:
    GJUserScore* m_targetScore;
    CCLayer* m_mainLayer;
    CCMenu* m_buttonMenu;

    bool init(GJUserScore* target) {
        if (!FLAlertLayer::init(150)) return false;
        m_targetScore = target;
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        m_mainLayer = CCLayer::create();
        this->addChild(m_mainLayer);

        m_buttonMenu = CCMenu::create();
        m_buttonMenu->setContentSize(winSize);
        m_buttonMenu->setPosition(0, 0);
        m_buttonMenu->setZOrder(10);
        m_mainLayer->addChild(m_buttonMenu);

        auto bg = extension::CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize({300.f, 200.f});
        bg->setPosition(winSize / 2);
        m_mainLayer->addChild(bg);

        auto title = CCLabelBMFont::create("Admin: Add User", "goldFont.fnt");
        title->setPosition({winSize.width / 2, winSize.height / 2 + 75.f});
        m_mainLayer->addChild(title);

        auto nameLabel = CCLabelBMFont::create(fmt::format("Target: {}", target->m_userName).c_str(), "chatFont.fnt");
        nameLabel->setPosition({winSize.width / 2, winSize.height / 2 + 40.f});
        m_mainLayer->addChild(nameLabel);

        std::string myRole = Mod::get()->getSavedValue<std::string>("user_role", "");

        if (myRole == "owner") {
            auto adminBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Give Admin", "goldFont.fnt", "GJ_button_01.png", 0.6f),
                this, menu_selector(AdminAddUserPopup::onGiveAdmin)
            );
            adminBtn->setPosition({winSize.width / 2 - 60.f, winSize.height / 2 - 10.f});
            m_buttonMenu->addChild(adminBtn);
        }

        auto modBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Give Mod", "goldFont.fnt", "GJ_button_01.png", 0.6f),
            this, menu_selector(AdminAddUserPopup::onGiveMod)
        );
        modBtn->setPosition({winSize.width / 2 + (myRole == "owner" ? 60.f : 0.f), winSize.height / 2 - 10.f});
        m_buttonMenu->addChild(modBtn);

        if (myRole == "owner" || myRole == "admin") {
            auto removeBtn = CCMenuItemSpriteExtra::create(
                ButtonSprite::create("Remove Role", "goldFont.fnt", "GJ_button_06.png", 0.6f),
                this, menu_selector(AdminAddUserPopup::onRemoveRole)
            );
            removeBtn->setPosition({winSize.width / 2, winSize.height / 2 - 45.f});
            m_buttonMenu->addChild(removeBtn);
        }

        auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        auto closeBtn = CCMenuItemSpriteExtra::create(closeSpr, this, menu_selector(AdminAddUserPopup::onClose));
        closeBtn->setPosition({winSize.width / 2 - 150.f, winSize.height / 2 + 100.f});
        m_buttonMenu->addChild(closeBtn);

        this->setKeypadEnabled(true);
        this->setTouchEnabled(true);
        return true;
    }

    void onGiveAdmin(CCObject*) { submitRole("admin"); }
    void onGiveMod(CCObject*) { submitRole("mod"); }

    void submitRole(std::string role) {
        Database::addModUser(m_targetScore->m_accountID, m_targetScore->m_userName.c_str(), role, [this](bool success) {
            if (success) {
                FLAlertLayer::create("Success", "User added successfully!", "OK")->show();
                this->onClose(nullptr);
            } else {
                FLAlertLayer::create("Error", "Failed to add user.", "OK")->show();
            }
        });
    }

    void onRemoveRole(CCObject*) {
        auto loading = FLAlertLayer::create("Loading", "Removing user role...", "OK");
        loading->show();

        Database::deleteUserRole(m_targetScore->m_accountID, [this, loading](bool success) {
            geode::queueInMainThread([this, loading, success]() {
                loading->keyBackClicked(); // close loading alert
                if (success) {
                    FLAlertLayer::create("Success", "User role has been removed.", "OK")->show();
                    this->onClose(nullptr);
                } else {
                    FLAlertLayer::create("Error", "Failed to remove user role.", "OK")->show();
                }
            });
        });
    }

    void onClose(CCObject*) {
        this->removeFromParent();
    }

public:
    static AdminAddUserPopup* create(GJUserScore* target) {
        auto ret = new AdminAddUserPopup();
        if (ret && ret->init(target)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class $modify(CoinGuideLevelInfoLayer, LevelInfoLayer) {

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) {
            return false;
        }

        if (level->m_stars > 0 && level->m_coins > 0) {
            
            auto readSpr = CCSprite::create("button1.png"_spr);
            if (!readSpr) readSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
            if (readSpr) readSpr->setScale(50.0f / readSpr->getContentSize().width);
            
            auto readBtn = CCMenuItemSpriteExtra::create(
                readSpr,
                this,
                menu_selector(CoinGuideLevelInfoLayer::onReadGuide)
            );
            readBtn->setID("read-guide-btn"_spr);

            auto writeSpr = CCSprite::create("modbutton1.png"_spr);
            if (!writeSpr) writeSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
            if (writeSpr) writeSpr->setScale(50.0f / writeSpr->getContentSize().width);

            auto writeBtn = CCMenuItemSpriteExtra::create(
                writeSpr,
                this,
                menu_selector(CoinGuideLevelInfoLayer::onWriteGuide)
            );
            writeBtn->setID("write-guide-btn"_spr);

            std::string userRole = Mod::get()->getSavedValue<std::string>("user_role", "");
            if (userRole == "owner" || userRole == "admin" || userRole == "mod") {
                writeBtn->setVisible(true);
            } else {
                writeBtn->setVisible(false);
            }

            if (auto leftMenu = this->getChildByID("left-side-menu")) {
                leftMenu->addChild(readBtn);
                if (writeBtn->isVisible()) leftMenu->addChild(writeBtn);
                leftMenu->updateLayout();
            } else {
                auto menu = CCMenu::create();
                menu->setID("coin-guide-menu"_spr);
                menu->addChild(readBtn);
                if (writeBtn->isVisible()) menu->addChild(writeBtn);
                menu->setLayout(
                    ColumnLayout::create()
                        ->setGap(5.0f)
                        ->setAxisAlignment(AxisAlignment::Center)
                );
                auto winSize = CCDirector::sharedDirector()->getWinSize();
                menu->setPosition(30.f, winSize.height / 2 - 20.f);
                menu->updateLayout();
                this->addChild(menu);
            }
        }

        return true;
    }

    void onReadGuide(CCObject* sender) {
        CoinGuidePopup::create(m_level)->show();
    }

    void onWriteGuide(CCObject* sender) {
        CoinSubmitPopup::create(m_level)->show();
    }

    void onCoinGuideProfilePage(CCObject* sender) {
        ProfilePage::create(GJAccountManager::sharedState()->m_accountID, true)->show();
    }
};

#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>

class $modify(CoinGuideLevelSearchLayer, LevelSearchLayer) {
    bool init(int p0) {
        if (!LevelSearchLayer::init(p0)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        
        auto btnSpr = CCSprite::create("logo.png"_spr);
        if (!btnSpr) btnSpr = CCSprite::createWithSpriteFrameName("GJ_coinsIcon_001.png");
        if (btnSpr) btnSpr->setScale(45.0f / btnSpr->getContentSize().width);
        
        auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(CoinGuideLevelSearchLayer::onFreeCoins));
        btn->setID("free-coins-btn"_spr);

        if (auto rightMenu = this->getChildByID("right-side-menu")) {
            btn->setPosition({0.f, -125.f}); 
            rightMenu->addChild(btn);
        } else {
            auto menu = CCMenu::create();
            menu->setPosition({winSize.width - 24.f, winSize.height / 2 - 125.f});
            menu->addChild(btn);
            menu->setZOrder(20);
            this->addChild(menu);
        }

        return true;
    }

    void onFreeCoins(CCObject*) {
                        }
                    }

                    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, browserScene));
                } else {
                    if (result.has_value()) {
                        FLAlertLayer::create("Notice", "No levels with free coins found.", "OK")->show();
                    } else {
                        FLAlertLayer::create("Error", "Failed to connect to the database.", "OK")->show();
                    }
                }
            });
        });
    }
};

class $modify(CoinGuideMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        
        auto accountId = GJAccountManager::sharedState()->m_accountID;
        if (accountId > 0) {
            Database::fetchUserRole(accountId, [](std::optional<std::string> role) {
                if (role) {
                    Mod::get()->setSavedValue("user_role", role.value());
                } else {
                    Mod::get()->setSavedValue("user_role", std::string(""));
                }
            });
        }
        return true;
    }
};

class $modify(CoinGuideProfilePage, ProfilePage) {
    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);

        auto myAccountId = GJAccountManager::sharedState()->m_accountID;
        std::string myRole = Mod::get()->getSavedValue<std::string>("user_role", "");

        if (myRole == "owner" || myRole == "admin") {
            if (auto menu = this->m_mainLayer->getChildByID("left-menu")) {
                if (!menu->getChildByID("add-user-btn"_spr)) {
                    auto addBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
                    addBtnSpr->setScale(0.7f);
                    auto addBtn = CCMenuItemSpriteExtra::create(
                        addBtnSpr,
                        this,
                        menu_selector(CoinGuideProfilePage::onAddUser)
                    );
                    addBtn->setID("add-user-btn"_spr);
                    menu->addChild(addBtn);
                    menu->updateLayout();
                }
            }
        }

        // Fetch user role for badges
        Database::fetchUserRole(score->m_accountID, [this](std::optional<std::string> role) {
            if (role) {
                std::string roleStr = role.value();
                std::string tex = "";
                if (roleStr == "owner" || roleStr == "admin") tex = "adminbadge.png"_spr;
                else if (roleStr == "mod") tex = "modbadge.png"_spr;
                
                if (!tex.empty()) {
                    if (auto menu = this->m_mainLayer->getChildByIDRecursive("username-menu")) {
                        if (!menu->getChildByID("admin-mod-badge"_spr)) {
                            auto spr = CCSprite::create(tex.c_str());
                            float targetWidth = 20.0f;
                            spr->setScale(targetWidth / spr->getContentSize().width);
                            
                            auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(CoinGuideProfilePage::onCustomBadge));
                            btn->setID("admin-mod-badge"_spr);
                            menu->addChild(btn);
                            menu->updateLayout();
                        }
                    }
                }
            }
        });
    }

    void onCustomBadge(CCObject*) {
        FLAlertLayer::create("Guide Badge", "This user is an Staff on The Coin Guide, they help adding guides of every user coin.", "OK")->show();
    }

    void onAddUser(CCObject*) {
        AdminAddUserPopup::create(m_score)->show();
    }
};
