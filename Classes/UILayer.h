#pragma once
#include "cocos2d.h"

USING_NS_CC;

class UILayerDelegate;
class UILayer : public CCLayer {
public:
	virtual bool init();

	void setDelegate(UILayerDelegate* pDelegate);
	void onUp(CCObject* pSender);
	void onDown(CCObject* pSender);
	void onLeft(CCObject* pSender);
	void onRight(CCObject* pSender);

	// for dpad
	void onMoveLeft(CCObject* pSender);
	void onMoveRight(CCObject* pSender);
	void onJump(CCObject* pSender);
	void onAttack(CCObject* pSender);

	CREATE_FUNC(UILayer);

private:
	UILayerDelegate* m_delegate;

	CCSprite* m_dpadNormal;
	CCSprite* m_dpadLeft;
	CCSprite* m_dpadRight;


};