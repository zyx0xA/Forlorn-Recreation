#include "AppDelegate.h"
#include "LoadingLayer.h"
#include "Sound.h"

USING_NS_CC;

AppDelegate::AppDelegate() { }
AppDelegate::~AppDelegate() { }

bool AppDelegate::applicationDidFinishLaunching() {
    CCDirector* pDirector = CCDirector::sharedDirector();
    CCEGLView* pEGLView = CCEGLView::sharedOpenGLView();

    pDirector->setOpenGLView(pEGLView);
    pDirector->setDisplayStats(true);
    pDirector->setAnimationInterval(1.0 / 60);

    CCScene *pScene = LoadingLayer::scene();
    pDirector->runWithScene(pScene);

    return true;
}

void AppDelegate::applicationDidEnterBackground() {
    CCDirector::sharedDirector()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground() {
    CCDirector::sharedDirector()->startAnimation();
}
