#pragma once
#include "cocos2d.h"

USING_NS_CC;

class LoadingLayer : public CCLayer {
public:
	virtual bool init();

	void tryLoad();
	void loadAssets();
	void loadingFinished();
	static cocos2d::CCScene* scene();

	CREATE_FUNC(LoadingLayer);
};