#pragma once
#include "cocos2d.h"
#include "cocos-ext.h"

USING_NS_CC;
USING_NS_CC_EXT;

class MainMenu : public CCLayer {
public:
	virtual bool init();
	virtual void keyBackClicked();

	void onExitGame(CCObject*);
	void onPlay(CCObject*);

	CREATE_FUNC(MainMenu);
};