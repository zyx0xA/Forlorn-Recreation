#pragma once
#include "cocos2d.h"
#include "Block.h" 
#include "UILayerDelegate.h"

#include <map>
#include <string>
#include <vector>

USING_NS_CC;

class PlayLayer : public CCLayer, public UILayerDelegate {
public:
	struct Args {
		int levelID;
		Args() : levelID(1) {}
	};

	static CCScene* scene(const Args& args);
	static PlayLayer* create(const Args& args);

	virtual bool init(const Args& args);
	virtual ~PlayLayer();
	PlayLayer();

	virtual void registerWithTouchDispatcher();
	virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event);
	virtual void ccTouchMoved(CCTouch* touch, CCEvent* event);
	virtual void ccTouchEnded(CCTouch* touch, CCEvent* event);

	virtual void onKeyPressed(int keyCode);
	virtual void onKeyReleased(int keyCode);

	virtual void keyBackClicked();
	virtual void keyMenuClicked();

	virtual void uiUp() override;
	virtual void uiDown() override;
	virtual void uiLeft() override;
	virtual void uiRight() override;

	void onLeft();
	void onRight();
	void onUp();
	void onDown();

	virtual void onEnter();
	virtual void onExit();
	virtual void update(float dt);

private:
	struct Members;
	Members* m;
};