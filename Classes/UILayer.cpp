#include "UILayer.h"
#include "UILayerDelegate.h"

bool UILayer::init() {
	if (!CCLayer::init()) return false;

	auto dir = CCDirector::sharedDirector();
	auto winSize = dir->getWinSize();

	auto btnMenu = CCMenu::create();
	btnMenu->setPosition({
		winSize.width / 2.0f + 10.0f,
		winSize.height / 2.0f - 150.0f
	});

	auto upTxt = CCLabelBMFont::create("UP", "goldFont.fnt");
	auto downTxt = CCLabelBMFont::create("DOWN", "goldFont.fnt");
	auto leftTxt = CCLabelBMFont::create("LEFT", "goldFont.fnt");
	auto rightTxt = CCLabelBMFont::create("RIGHT", "goldFont.fnt");

	upTxt->setScale(0.6f);
	downTxt->setScale(0.6f);
	leftTxt->setScale(0.6f);
	rightTxt->setScale(0.6f);

	auto upBtn = CCMenuItemSprite::create(upTxt, upTxt, this, menu_selector(UILayer::onUp));
	auto downBtn = CCMenuItemSprite::create(downTxt, downTxt, this, menu_selector(UILayer::onDown));
	auto leftBtn = CCMenuItemSprite::create(leftTxt, leftTxt, this, menu_selector(UILayer::onLeft));
	auto rightBtn = CCMenuItemSprite::create(rightTxt, rightTxt, this, menu_selector(UILayer::onRight));

	btnMenu->addChild(upBtn);
	btnMenu->addChild(downBtn);
	btnMenu->addChild(leftBtn);
	btnMenu->addChild(rightBtn);

	// dpad logic
	auto dpad = CCSprite::createWithSpriteFrameName("Dpad_Btn.png");
	
	btnMenu->addChild(dpad);

	addChild(btnMenu, 100);

	return true;
}

void UILayer::setDelegate(UILayerDelegate* delegate) {
	m_delegate = delegate;
}

void UILayer::onUp(CCObject* pSender) {
	if (m_delegate) m_delegate->uiUp();
}
void UILayer::onDown(CCObject* pSender) {
	if (m_delegate) m_delegate->uiDown();
}
void UILayer::onLeft(CCObject* pSender) {
	if (m_delegate) m_delegate->uiLeft();
}
void UILayer::onRight(CCObject* pSender) {
	if (m_delegate) m_delegate->uiRight();
}