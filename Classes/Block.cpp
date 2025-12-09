#include "Block.h"
#include <cstring>

USING_NS_CC;

// re-edit by zyann
// from axmol to cocos

Block* Block::create(const Data& data) {
	Block* pRet = new Block();
	if (pRet && pRet->init(data)) {
		pRet->autorelease();
		return pRet;
	}
	else {
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

bool Block::isSheetHD(const Data& d) {
	static const char* no_hd[] = {
		"ForestSheet_MG", "lightSheet", "FrostLevel_MGSheet", "FlameVillageMG"
	};

	int count = sizeof(no_hd) / sizeof(no_hd[0]);

	for (int i = 0; i < count; ++i) {
		if (d.spriteSheet == no_hd[i])
			return false;
	}

	return true;
}

bool Block::init(const Block::Data& d) {
	if (!CCSprite::initWithSpriteFrameName(d.texture.c_str())) {
		CCLog("Could not create block %s", d.texture.c_str());
		return false;
	}

	setPosition(d.position);

	CCPoint newscale = d.scale;

	// ⬇️ FIX: Apply 0.5x scale to HD assets 
	if (isSheetHD(d)) {
		// HD assets are typically loaded at 2x size and need 0.5x scaling
		// to fit the 1x coordinate system.
		newscale = ccpMult(newscale, 0.5f);
	}
	// ⬆️ END FIX

	setScaleX(newscale.x * (d.flippedX ? -1.0f : 1.0f));
	setScaleY(newscale.y * (d.flippedY ? -1.0f : 1.0f));

	if (d.hasRotation) setRotation(d.rotation);
	if (d.hasOpacity) setOpacity(d.opacity * 255.0f);
	if (d.hasColor) {
		ccColor3B color = ccc3(
			static_cast<GLubyte>(d.red * 255.0f),
			static_cast<GLubyte>(d.green * 255.0f),
			static_cast<GLubyte>(d.blue * 255.0f)
			);
		setColor(color);
	}

	if (!d.customAnim.empty()) {
		runCustomAnimation(d);
	}
	else if (d.animated) {
		runAnimationLooped(d);
	}


	return true;
}

void Block::runCustomAnimation(const Data& d) {
	auto a1 = CCMoveBy::create(1.0f, ccp(0, 10));
	auto a2 = a1->reverse();
	auto seq = CCSequence::create(a1, a2, NULL);
	auto rep = CCRepeatForever::create(seq);
	runAction(rep);
}

void Block::runAnimationLooped(const Data& d) {
	CCArray* frames = CCArray::create();
	std::string rawName = d.texture;
	size_t lastIndex = rawName.find_last_of(".");
	if (lastIndex != std::string::npos) {
		rawName = rawName.substr(0, lastIndex);
	}

	CCSpriteFrameCache* sfc = CCSpriteFrameCache::sharedSpriteFrameCache();
	int i = 1;

	// for generating sprite frame names
#define GET_FRAME(type, index) \
        CCString::createWithFormat("%s_%s_%03d.png", rawName.c_str(), type, index)->getCString()

	if (!d.skipStartAnim) {
		i = 1;
		while (CCSpriteFrame* frame = sfc->spriteFrameByName(GET_FRAME("start", i))) {
			frames->addObject(frame);
			i++;
		}
	}

	i = 1;
	while (CCSpriteFrame* frame = sfc->spriteFrameByName(GET_FRAME("looped", i))) {
		frames->addObject(frame);
		i++;
	}

	if (!d.skipEndAnim) {
		i = 1;
		while (CCSpriteFrame* frame = sfc->spriteFrameByName(GET_FRAME("end", i))) {
			frames->addObject(frame);
			i++;
		}
	}

	if (frames->count() > 0) {
		CCAnimation* animation = CCAnimation::createWithSpriteFrames(frames, 1.0f / 12.0f);
		CCAnimate* animate = CCAnimate::create(animation);
		CCRepeatForever* repeat = CCRepeatForever::create(animate);
		runAction(repeat);
	}
	else {
		CCLog("Skipping animation for block %s: No frames found.", d.texture.c_str());
	}
}

void Block::updateTweenAction(float value, const char* key) { }