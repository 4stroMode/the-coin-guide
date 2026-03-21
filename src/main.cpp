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
#include "CoinRatingPopup.hpp"

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

            CCLabelBMFont* totalVotesLbl = nullptr;
            if (auto practiceBar = this->getChildByID("practice-mode-bar")) {
                bool hasRated = Mod::get()->getSavedValue<bool>(fmt::format("rated_coins_{}", level->m_levelID));
                auto rateSpr = CCSprite::createWithSpriteFrameName("GJ_like2Btn_001.png");
                rateSpr->setScale(0.6f);
                if (hasRated) {
                    rateSpr->setColor({100, 100, 100});
                }
                auto rateBtnX = CCMenuItemSpriteExtra::create(rateSpr, this, menu_selector(CoinGuideLevelInfoLayer::onRateCoins));
                rateBtnX->setID("coin-rating-btn"_spr);

                auto rateMenuLayout = CCMenu::create();
                rateMenuLayout->setPosition({practiceBar->getPositionX() + practiceBar->getContentSize().width / 2.f * practiceBar->getScaleX() + 20.f, practiceBar->getPositionY()});
                rateMenuLayout->addChild(rateBtnX);

                totalVotesLbl = CCLabelBMFont::create("0", "bigFont.fnt");
                totalVotesLbl->setScale(0.45f);
                totalVotesLbl->setPosition({rateBtnX->getPositionX() + rateBtnX->getScaledContentSize().width / 2.f + 16.f, rateBtnX->getPositionY()});
                totalVotesLbl->setVisible(false); // Hide until fetched
                rateMenuLayout->addChild(totalVotesLbl);

                rateMenuLayout->setZOrder(practiceBar->getZOrder());
                this->addChild(rateMenuLayout);
            }

            geode::Ref<CCLabelBMFont> votesLblRef = totalVotesLbl;

            if (auto diffSprite = this->getChildByID("difficulty-sprite")) {
                    auto label = CCLabelBMFont::create("...", "bigFont.fnt");
                    label->setScale(0.6f);

                    auto avgBtn = CCMenuItemSpriteExtra::create(label, this, menu_selector(CoinGuideLevelInfoLayer::onAverageRatingHelp));
                    avgBtn->setID("coin-avg-btn"_spr);

                    auto avgMenu = CCMenu::create();
                    avgMenu->setPosition({diffSprite->getPositionX() - 29.f, diffSprite->getPositionY() - 43.f});
                    avgMenu->addChild(avgBtn);
                    avgMenu->setZOrder(diffSprite->getZOrder());
                    this->addChild(avgMenu);

                    geode::Ref<CCLabelBMFont> labelRef = label;
                    Database::fetchAverageCoinRating(level->m_levelID, [labelRef, votesLblRef](std::optional<std::pair<int, int>> avgData) {
                        if (labelRef && votesLblRef) {
                            if (avgData.has_value()) {
                                int val = avgData.value().first;
                                int total = avgData.value().second;
                                
                                labelRef->setString(std::to_string(val).c_str());
                                
                                std::string formattedTotal;
                                if (total >= 1000000) {
                                    int m = total / 1000000;
                                    int k = (total % 1000000) / 100000;
                                    formattedTotal = k > 0 ? fmt::format("{}.{}M", m, k) : fmt::format("{}M", m);
                                } else if (total >= 1000) {
                                    int k = total / 1000;
                                    int h = (total % 1000) / 100;
                                    formattedTotal = h > 0 ? fmt::format("{}.{}k", k, h) : fmt::format("{}k", k);
                                } else {
                                    formattedTotal = std::to_string(total);
                                }
                                votesLblRef->setString(formattedTotal.c_str());
                                votesLblRef->setVisible(true);
                            
                            if (val <= 2) labelRef->setColor({50, 150, 255});       // Blue
                            else if (val <= 4) labelRef->setColor({0, 255, 0});     // Green
                            else if (val <= 6) labelRef->setColor({255, 255, 0});   // Yellow
                            else if (val <= 8) labelRef->setColor({255, 150, 0});   // Orange
                            else if (val == 9) labelRef->setColor({200, 100, 255}); // Light Purple
                            else labelRef->setColor({255, 0, 0});                   // Red
                            } else {
                            labelRef->setString("N/A");
                            labelRef->setScale(0.5f);
                            labelRef->setColor({150, 150, 150});
                        }
                    }
                });
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

    void onRateCoins(CCObject* sender) {
        if (Mod::get()->getSavedValue<bool>(fmt::format("rated_coins_{}", m_level->m_levelID))) {
            FLAlertLayer::create("Already Evaluated", "You have already evaluated the coins for this level.", "OK")->show();
            return;
        }
        CoinRatingPopup::create(m_level)->show();
    }

    void onAverageRatingHelp(CCObject* sender) {
        FLAlertLayer::create(
            "Coins Rating",
            "This rating is based on user opinions to indicate the difficulty of the coins.\n"
            "<cg>1-2</c> Easy Coins\n"
            "<cl>3-4</c> Intermediate Coins\n"
            "<cy>4-5</c> Hard Coins\n"
            "<co>6-7</c> Difficult Coins\n"
            "<cp>8-9</c> Insane Coins\n"
            "<cr>10</c> Extreme Coins",
            "OK"
        )->show();
    }

    void onCoinGuideProfilePage(CCObject* sender) {
        ProfilePage::create(GJAccountManager::sharedState()->m_accountID, true)->show();
    }
};

#include <Geode/modify/CommentCell.hpp>
#include <Geode/binding/GJComment.hpp>

class $modify(CoinGuideCommentCell, CommentCell) {
    void loadFromComment(GJComment* comment) {
        CommentCell::loadFromComment(comment);
        
        if (this->m_accountComment) return;

        if (comment->m_accountID > 0) {
            auto accountId = comment->m_accountID;
            auto cell = this;
            Database::fetchUserRole(accountId, [cell, accountId, comment](std::optional<std::string> role) {
                if (role) {
                    std::string roleStr = role.value();
                    std::string tex = "";
                    if (roleStr == "owner" || roleStr == "admin") tex = "adminbadge.png"_spr;
                    else if (roleStr == "mod") tex = "modbadge.png"_spr;
                    
                    if (!tex.empty()) {
                        geode::queueInMainThread([cell, tex, comment]() {
                            if (cell && cell->m_mainLayer) {
                                auto spr = CCSprite::create(tex.c_str());
                                if (!spr) return;
                                spr->setScale(14.0f / spr->getContentSize().width);
                                spr->setAnchorPoint({0.f, 0.45f});
                                
                                CCLabelBMFont* nameLbl = nullptr;
                                auto findLabel = [&nameLbl](CCNode* parent, const std::string& name, auto& self) -> void {
                                    if (nameLbl) return;
                                    if (auto lbl = typeinfo_cast<CCLabelBMFont*>(parent)) {
                                        std::string str = lbl->getString();
                                        if (str.find(name) != std::string::npos && str.length() < name.length() + 10) {
                                            nameLbl = lbl;
                                            return;
                                        }
                                    }
                                    if (parent->getChildrenCount() > 0) {
                                        for (CCNode* child : parent->getChildrenExt()) {
                                            self(child, name, self);
                                        }
                                    }
                                };
                                findLabel(cell->m_mainLayer, comment->m_userName, findLabel);
                                
                                auto badgeMenu = CCMenu::create();
                                badgeMenu->setPosition({0, 0});
                                cell->m_mainLayer->addChild(badgeMenu);
                                
                                auto badgeMenuBtn = CCMenuItemSpriteExtra::create(spr, nullptr, nullptr);
                                badgeMenuBtn->setID("admin-mod-badge"_spr);
                                
                                if (nameLbl) {
                                    auto worldPos = nameLbl->convertToWorldSpace(CCPoint{nameLbl->getContentSize().width + 23.8f, nameLbl->getContentSize().height / 2.f});
                                    auto localPos = cell->m_mainLayer->convertToNodeSpace(worldPos);
                                    
                                    badgeMenuBtn->setPosition(localPos);

                                    CCLabelBMFont* pctLbl = nullptr;
                                    auto findPct = [&pctLbl](CCNode* parent, auto& self) -> void {
                                        if (pctLbl) return;
                                        if (auto lbl = typeinfo_cast<CCLabelBMFont*>(parent)) {
                                            std::string str = lbl->getString();
                                            if (str.find("%") != std::string::npos && str.length() <= 5) {
                                                pctLbl = lbl;
                                                return;
                                            }
                                        }
                                        if (parent->getChildrenCount() > 0) {
                                            for (CCNode* child : parent->getChildrenExt()) {
                                                self(child, self);
                                            }
                                        }
                                    };
                                    findPct(cell->m_mainLayer, findPct);
                                    
                                    if (pctLbl) {
                                        pctLbl->setPositionX(pctLbl->getPositionX() + 16.f);
                                    }
                                } else {
                                    badgeMenuBtn->setPosition({115.f, 30.f});
                                }
                                badgeMenu->addChild(badgeMenuBtn);
                            }
                        });
                    }
                }
            });
        }
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
        CoinSearchPopup::create()->show();
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
                            spr->setAnchorPoint({0.5f, 0.65f}); // Push graphic down slightly inside the button bounds
                            
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
        FLAlertLayer::create("Coin Guide Staff", "This user is an Staff on The Coin Guide, they help adding guides of every user coin.", "OK")->show();
    }

    void onAddUser(CCObject*) {
        AdminAddUserPopup::create(m_score)->show();
    }
};
