#include "LoadingLayer.h"
#include "MainMenu.h"

bool LoadingLayer::init() {
	if (!CCLayer::init()) return false;

	auto dir = CCDirector::sharedDirector();
	auto winSize = dir->getWinSize();

	// why stored in unique function?
	// because i like it.
	tryLoad();

	auto sprite = CCSprite::create("Default.png");
	sprite->setPosition(winSize / 2.0f);

	addChild(sprite);

	return true;
}

void LoadingLayer::tryLoad() {
	auto seq = CCSequence::create(
		CCCallFunc::create(this, callfunc_selector(LoadingLayer::loadAssets)),
		CCDelayTime::create(.5f),
		CCCallFunc::create(this, callfunc_selector(LoadingLayer::loadingFinished)),
		nullptr
	);

	runAction(seq);
}

void LoadingLayer::loadAssets() {
	CCLog("[INFO] Loading Assets...");

	auto texture = CCTextureCache::sharedTextureCache();
	auto frame = CCSpriteFrameCache::sharedSpriteFrameCache();

	texture->addImage("UISheet.png");
	frame->addSpriteFramesWithFile("UISheet.plist");
}

void LoadingLayer::loadingFinished() {
	CCLog("[INFO] Loading Done!");

	auto layer = MainMenu::create();
	auto scene = CCScene::create();
	scene->addChild(layer);

	CCDirector::sharedDirector()
		->replaceScene(scene);
}

CCScene* LoadingLayer::scene() {
	auto layer = LoadingLayer::create();
	auto scene = CCScene::create();
	scene->addChild(layer);

	return scene;
}