#include "MainMenu.h"
#include "PlayLayer.h"

bool MainMenu::init() {
	if (!CCLayer::init()) return false;

	setKeypadEnabled(true);

	auto dir = CCDirector::sharedDirector();
	auto winSize = dir->getWinSize();

	auto logo = CCSprite::create("forlorn_logo_menu.png");
	logo->setScale(0.6f);
	logo->setPosition({
		winSize.width / 2.0f,
		winSize.height - 120.0f
	});
	addChild(logo);

	// play button
	// i dont have button sprite yet
	auto btnMenu = CCMenu::create();
	btnMenu->setPosition(winSize / 2.0f);
	addChild(btnMenu);

	auto play_on = CCLabelBMFont::create("PLAY", "bigFont.fnt");
	auto play_off = CCLabelBMFont::create("PLAY", "bigFont.fnt");
	play_off->setColor({ 200, 200, 200 });

	auto playBtn = CCMenuItemSprite::create(play_off, play_on, this, menu_selector(MainMenu::onPlay));
	playBtn->setPosition(btnMenu->convertToNodeSpace({
		winSize.width / 2.0f,
		winSize.height / 2.0f - 40.0f
	}));

	// exit btn
	auto exit_on = CCLabelBMFont::create("EXIT", "bigFont.fnt");
	auto exit_off = CCLabelBMFont::create("EXIT", "bigFont.fnt");
	exit_off->setColor({ 200, 200, 200 });

	auto exitBtn = CCMenuItemSprite::create(exit_off, exit_on, this, menu_selector(MainMenu::onExitGame));
	exitBtn->setPosition(btnMenu->convertToNodeSpace({
		winSize.width / 2.0f,
		winSize.height / 2.0f - 80.0f
	}));

	btnMenu->addChild(playBtn);
	btnMenu->addChild(exitBtn);

	return true;
}

void MainMenu::keyBackClicked() {
	CCDirector::sharedDirector()
		->end();
}

void MainMenu::onExitGame(CCObject* pSender) { keyBackClicked(); }
void MainMenu::onPlay(CCObject* pSender) {
	PlayLayer::Args args;
	args.levelID = 1;

	auto gameScene = PlayLayer::scene(args);
	CCDirector::sharedDirector()
		->replaceScene(CCTransitionFade::create(0.5f, gameScene));
}