#include "PlayLayer.h"
#include "JsonUtils.h"
#include "MainMenu.h"
#include "UILayer.h"
#include <cstring> 

#include "rapidjson/document.h"
#include "rapidjson/error/error.h" 

// re-edited by zyann
// from axmol to cocos

struct LevelStruct {
	struct SheetData {
		std::string texture;
		std::string type;

		bool operator<(const SheetData& b) const {
			if (texture != b.texture) return texture < b.texture;
			return type < b.type;
		}
	};

	std::map<std::string, Block::Data> blockContainer;
	std::map<std::string, Block::Data> bgContainer;
	std::map<SheetData, std::string> sheetContainer;
	CCPoint playerSpawn;
};

struct PlayLayer::Members {
	CCPoint playerPos;
	CCPoint spawnPos;
	float camSpeed;
	float zoom;

	bool up;
	bool down;
	bool left;
	bool right;

	std::map<LevelStruct::SheetData, CCSpriteBatchNode*> batchNodes;

	Members()
		: playerPos(ccp(0, 0))
		, spawnPos(ccp(0, 0))
		, camSpeed(7.0f)
		, zoom(1.0f)
		, up(false), down(false), left(false), right(false)
	{}
};

CCPoint parsePoint(const rapidjson::Value& value) {
	if (value.IsArray() && value.Size() == 2) {
		return ccp(value[(rapidjson::SizeType)0].GetDouble(),
			value[(rapidjson::SizeType)1].GetDouble());
	}
	CCLog("invalid pos in json.");
	return ccp(0, 0);
}

bool loadLevelData(LevelStruct& lvl, const std::string& filename) {
	unsigned long bufferSize = 0;

	char* buffer = (char*)CCFileUtils::sharedFileUtils()->getFileData(filename.c_str(), "rb", &bufferSize);

	if (!buffer || bufferSize == 0) {
		CCLog("could not load file %s. buf size: %lu", filename.c_str(), bufferSize);
		if (buffer) free(buffer);
		return false;
	}

	std::string jsonString(buffer, bufferSize);
	free(buffer);

	const char* jsonText = jsonString.c_str();
	size_t jsonTextLength = jsonString.length();

	if (jsonTextLength >= 3 &&
		(unsigned char)jsonText[0] == 0xEF &&
		(unsigned char)jsonText[1] == 0xBB &&
		(unsigned char)jsonText[2] == 0xBF) {
		jsonText += 3;
	}

	rapidjson::Document doc;
	doc.Parse<0>(jsonText);

	if (doc.HasParseError()) {
		CCLog("error parsing json file %s: rapidjson err no. %d (Offset %lu)",
			filename.c_str(), doc.GetParseError(), doc.GetErrorOffset());
		return false;
	}

	if (doc.HasMember("playerSpawn") && doc["playerSpawn"].IsArray()) {
		lvl.playerSpawn = parsePoint(doc["playerSpawn"]);
	}
	else {
		lvl.playerSpawn = ccp(300, 300);
	}

	size_t blockCount = 0;

	if (doc.HasMember("blockContainer")) {
		const rapidjson::Value& blockContainer = doc["blockContainer"];

		if (blockContainer.IsObject()) {
			for (rapidjson::Value::ConstMemberIterator itr = blockContainer.MemberBegin();
				itr != blockContainer.MemberEnd(); ++itr)

			{
				const rapidjson::Value& json = itr->value;

				if (!json.IsObject()) {
					CCLog("block skipped '%s': not a json object.", itr->name.GetString());
					continue;
				}

				Block::Data bd;

				if (!json.HasMember("spriteSheet") || !json.HasMember("texture") ||
					!json.HasMember("sheetType") || !json.HasMember("position") ||
					!json.HasMember("scale") || !json.HasMember("zValue")) {
					CCLog("block skipped '%s' due to missing mandatory field", itr->name.GetString());
					continue;
				}

				bd.spriteSheet = json["spriteSheet"].GetString();
				bd.texture = json["texture"].GetString();
				bd.sheetType = json["sheetType"].GetString();

				if (json["position"].IsArray()) {
					bd.position = parsePoint(json["position"]);
				}
				else {
					CCLog("block skipped '%s': 'position' is not an array.", itr->name.GetString());
					continue;
				}

				if (json["scale"].IsArray()) {
					bd.scale = parsePoint(json["scale"]);
				}
				else {
					CCLog("block skipped '%s': 'scale' is not an array.", itr->name.GetString());
					continue;
				}

				bd.zValue = json["zValue"].GetInt();

				if (json.HasMember("red") && json.HasMember("green") && json.HasMember("blue")) {
					bd.red = json["red"].GetDouble();
					bd.green = json["green"].GetDouble();
					bd.blue = json["blue"].GetDouble();
					bd.hasColor = true;
				}
				if (json.HasMember("opacity")) {
					bd.opacity = json["opacity"].GetDouble();
					bd.hasOpacity = true;
				}

				if (json.HasMember("flipped") && json["flipped"].IsArray() && json["flipped"].Size() >= 2) {
					const rapidjson::Value& flippedArr = json["flipped"];

					const rapidjson::Value& v0 = flippedArr[(rapidjson::SizeType)0];
					if (v0.IsBool()) bd.flippedX = v0.GetBool();
					else if (v0.IsInt()) bd.flippedX = v0.GetInt() != 0;

					const rapidjson::Value& v1 = flippedArr[(rapidjson::SizeType)1];
					if (v1.IsBool()) bd.flippedY = v1.GetBool();
					else if (v1.IsInt()) bd.flippedY = v1.GetInt() != 0;
				}

				if (json.HasMember("rotation")) {
					bd.rotation = json["rotation"].GetDouble();
					bd.hasRotation = true;
				}

				if (json.HasMember("customAnim")) {
					bd.customAnim = json["customAnim"].GetString();
				}

				if (json.HasMember("animated")) {
					bd.animated = GetBoolFlexible(json, "animated");
				}

				LevelStruct::SheetData sheetData;
				sheetData.texture = bd.spriteSheet;
				sheetData.type = bd.sheetType;

				lvl.sheetContainer.insert(std::make_pair(sheetData, bd.spriteSheet));
				lvl.blockContainer[itr->name.GetString()] = bd;
				blockCount++;
			}
		}
		CCLog("Parsed %u total blocks from blockContainer.", (unsigned int)blockCount);
	}


	size_t bgElementCount = 0;
	if (doc.HasMember("bgContainer") && doc["bgContainer"].IsObject()) {
		const rapidjson::Value& bgContainer = doc["bgContainer"];

		if (bgContainer.IsObject()) {
			for (rapidjson::Value::ConstMemberIterator itr = bgContainer.MemberBegin();
				itr != bgContainer.MemberEnd(); ++itr)
			{
				const rapidjson::Value& json = itr->value;

				if (!json.IsObject()) {
					CCLog("background element skipped '%s': not a json object.", itr->name.GetString());
					continue;
				}

				Block::Data bd;

				if (!json.HasMember("spriteSheet") || !json.HasMember("texture") ||
					!json.HasMember("bgType") || !json.HasMember("position") ||
					!json.HasMember("scale") || !json.HasMember("zValue")) {
					CCLog("background element skipped '%s' due to missing mandatory field", itr->name.GetString());
					continue;
				}

				bd.spriteSheet = json["spriteSheet"].GetString();
				bd.texture = json["texture"].GetString();
				bd.sheetType = json["bgType"].GetString();

				if (json["position"].IsArray()) {
					bd.position = parsePoint(json["position"]);
				}
				else { continue; }

				if (json["scale"].IsArray()) {
					bd.scale = parsePoint(json["scale"]);
				}
				else { continue; }

				bd.zValue = json["zValue"].GetInt();

				if (json.HasMember("red") && json.HasMember("green") && json.HasMember("blue")) {
					bd.red = json["red"].GetDouble();
					bd.green = json["green"].GetDouble();
					bd.blue = json["blue"].GetDouble();
					bd.hasColor = true;
				}
				if (json.HasMember("opacity")) {
					bd.opacity = json["opacity"].GetDouble();
					bd.hasOpacity = true;
				}

				if (json.HasMember("flipped") && json["flipped"].IsArray() && json["flipped"].Size() >= 2) {
					const rapidjson::Value& flippedArr = json["flipped"];

					const rapidjson::Value& v0 = flippedArr[(rapidjson::SizeType)0];
					if (v0.IsBool()) bd.flippedX = v0.GetBool();
					else if (v0.IsInt()) bd.flippedX = v0.GetInt() != 0;

					const rapidjson::Value& v1 = flippedArr[(rapidjson::SizeType)1];
					if (v1.IsBool()) bd.flippedY = v1.GetBool();
					else if (v1.IsInt()) bd.flippedY = v1.GetInt() != 0;
				}

				if (json.HasMember("rotation")) {
					bd.rotation = json["rotation"].GetDouble();
					bd.hasRotation = true;
				}

				LevelStruct::SheetData sheetData;
				sheetData.texture = bd.spriteSheet;
				sheetData.type = bd.sheetType;

				lvl.sheetContainer.insert(std::make_pair(sheetData, bd.spriteSheet));
				lvl.bgContainer[itr->name.GetString()] = bd;
				bgElementCount++;
			}
		}
		CCLog("Parsed %u total background elements from bgContainer.", (unsigned int)bgElementCount);
	}


	if (doc.HasMember("entityContainer")) {
		const rapidjson::Value& entityContainer = doc["entityContainer"];
		if (entityContainer.IsObject()) {
			if (entityContainer.HasMember("player_0") && entityContainer["player_0"].IsObject()) {
				const rapidjson::Value& player_0 = entityContainer["player_0"];
				if (player_0.HasMember("position") && player_0["position"].IsArray()) {
					lvl.playerSpawn = parsePoint(player_0["position"]);
				}
			}
		}
	}

	return true;
}

int getZValueForType(const std::string& type) {
	int ret = 0;
	if (type == "F1") ret = 160;
	else if (type == "F2") ret = 150;
	else if (type == "F3") ret = 140;
	else if (type == "F4") ret = 130;
	else if (type == "B1") ret = 50;
	else if (type == "B2") ret = 40;
	else if (type == "B3") ret = 30;
	else if (type == "B4") ret = 20;
	else if (type == "mg") ret = 10; // mg
	else if (type == "fg") ret = 170; // foreground
	else if (type == "BG") ret = 5; // bg
	else if (type == "NPC") ret = 90;
	else if (type == "particle") ret = 0;
	else if (type == "P1") ret = 120;

	if (type.find('+') != std::string::npos) ret++;
	else if (type.find('-') != std::string::npos) ret--;
	return ret;
}

CCScene* PlayLayer::scene(const Args& args) {
	auto scene = CCScene::create();
	auto playLayer = PlayLayer::create(args);
	auto uiLayer = UILayer::create();

	if (playLayer && uiLayer) {
		uiLayer->setDelegate(playLayer);
		uiLayer->addChild(playLayer, 10);

		scene->addChild(uiLayer);
		return scene;
	}

	return NULL;
}

PlayLayer* PlayLayer::create(const Args& args) {
	PlayLayer *pRet = new PlayLayer();
	if (pRet && pRet->init(args)) {
		pRet->autorelease();
		return pRet;
	}
	else {
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

PlayLayer::PlayLayer() : m(new Members()) {}
PlayLayer::~PlayLayer() { delete m; }

bool PlayLayer::init(const Args& args) {
	if (!CCLayer::init()) return false;

	setTouchEnabled(true);
	setKeypadEnabled(true);
	scheduleUpdate();

	LevelStruct lvl;

	// level 2 3 doesnt work.
	if (!loadLevelData(lvl, "Level001.json")) {
		return false;
	}

	m->spawnPos = lvl.playerSpawn;
	m->playerPos = lvl.playerSpawn;

	auto sprch = CCSpriteFrameCache::sharedSpriteFrameCache();

	std::map<LevelStruct::SheetData, std::string>::iterator sheetIt = lvl.sheetContainer.begin();

	while (sheetIt != lvl.sheetContainer.end()) {
		LevelStruct::SheetData sheetData = sheetIt->first;

		auto plist = CCString::createWithFormat("%s.plist", sheetData.texture.c_str());
		auto png = CCString::createWithFormat("%s.png", sheetData.texture.c_str());

		if (!CCFileUtils::sharedFileUtils()->isFileExist(plist->getCString()) ||
			!CCFileUtils::sharedFileUtils()->isFileExist(png->getCString())) {
			CCLog("sheet %s: .plist or .png file is missing", sheetData.texture.c_str());
			sheetIt = lvl.sheetContainer.erase(sheetIt);
			continue;
		}

		sprch->addSpriteFramesWithFile(plist->getCString());

		auto batch = CCSpriteBatchNode::create(png->getCString());
		m->batchNodes[sheetData] = batch;
		addChild(batch, getZValueForType(sheetData.type));

		++sheetIt;
	}

	std::map<std::string, Block::Data>::iterator bgIt;
	for (bgIt = lvl.bgContainer.begin(); bgIt != lvl.bgContainer.end(); ++bgIt) {
		Block::Data& bd = bgIt->second;

		CCSpriteBatchNode* bn = NULL;
		std::map<LevelStruct::SheetData, CCSpriteBatchNode*>::iterator batchIt;

		for (batchIt = m->batchNodes.begin(); batchIt != m->batchNodes.end(); ++batchIt) {
			if (batchIt->first.texture == bd.spriteSheet && batchIt->first.type == bd.sheetType) {
				bn = batchIt->second;
				break;
			}
		}

		if (!bn) {
			CCLog("BG ERROR: BatchNode NOT found for element '%s' (Sheet: %s, Type: %s). Skipping.",
				bgIt->first.c_str(), bd.spriteSheet.c_str(), bd.sheetType.c_str());
			continue;
		}

		auto bgElement = Block::create(bd);
		if (bgElement) {
			bn->addChild(bgElement, bd.zValue);
		}
		else {
			CCLog("BG ERROR: Failed to create element '%s' (Texture: %s). Check if sprite frame exists in the .plist.",
				bgIt->first.c_str(), bd.texture.c_str());
		}
	}


	// kinda messy, but hey it works.
	std::map<std::string, Block::Data>::iterator blockIt;
	for (blockIt = lvl.blockContainer.begin(); blockIt != lvl.blockContainer.end(); ++blockIt) {
		Block::Data& bd = blockIt->second;

		CCSpriteBatchNode* bn = NULL;
		std::map<LevelStruct::SheetData, CCSpriteBatchNode*>::iterator batchIt;

		for (batchIt = m->batchNodes.begin(); batchIt != m->batchNodes.end(); ++batchIt) {
			if (batchIt->first.texture == bd.spriteSheet && batchIt->first.type == bd.sheetType) {
				bn = batchIt->second;
				break;
			}
		}

		if (!bn) {
			CCLog("Block ERROR: BatchNode NOT found for element '%s' (Sheet: %s, Type: %s). Skipping.",
				blockIt->first.c_str(), bd.spriteSheet.c_str(), bd.sheetType.c_str());
			continue;
		}

		auto block = Block::create(bd);
		if (block) {
			bn->addChild(block, bd.zValue);
		}
		else {
			CCLog("Block ERROR: Failed to create element '%s' (Texture: %s). Check if sprite frame exists in the .plist.",
				blockIt->first.c_str(), bd.texture.c_str());
		}
	}

	return true;
}


void PlayLayer::registerWithTouchDispatcher() {
	CCDirector::sharedDirector()
		->getTouchDispatcher()
		->addStandardDelegate(this, 0);
}

void PlayLayer::onEnter() { CCLayer::onEnter(); }
void PlayLayer::onExit() { CCLayer::onExit(); }

void PlayLayer::update(float dt) {
	if (m->up) onUp();
	if (m->down) onDown();
	if (m->left) onLeft();
	if (m->right) onRight();

	CCSize winSize = CCDirector::sharedDirector()->getWinSize();

	float x = winSize.width / 2.0f - m->playerPos.x * m->zoom;
	float y = winSize.height / 2.0f - m->playerPos.y * m->zoom;

	setPosition(ccp(x, y));
	setScale(m->zoom);
}

void PlayLayer::onLeft() { m->playerPos.x -= m->camSpeed; }
void PlayLayer::onRight() { m->playerPos.x += m->camSpeed; }
void PlayLayer::onUp() { m->playerPos.y += m->camSpeed; }
void PlayLayer::onDown() { m->playerPos.y -= m->camSpeed; }

bool PlayLayer::ccTouchBegan(CCTouch* touch, CCEvent* event) { return true; }
void PlayLayer::ccTouchMoved(CCTouch* touch, CCEvent* event) { }
void PlayLayer::ccTouchEnded(CCTouch* touch, CCEvent* event) { }

void PlayLayer::keyBackClicked() {
	auto layer = MainMenu::create();
	auto scene = CCScene::create();
	scene->addChild(layer);

	CCDirector::sharedDirector()
		->replaceScene(scene);
}

void PlayLayer::keyMenuClicked() { }

void PlayLayer::uiUp() { m->up = !m->up; }
void PlayLayer::uiDown() { m->down = !m->down; }
void PlayLayer::uiLeft() { m->left = !m->left; }
void PlayLayer::uiRight() { m->right = !m->right; }

// key definition
// thanks gpt.
#define VK_A 0x41
#define VK_D 0x44
#define VK_W 0x57
#define VK_S 0x53

#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28

void PlayLayer::onKeyPressed(int keyCode) {
	switch (keyCode) {
	case VK_W:
	case VK_UP:
		m->up = true;
		break;
	case VK_S:
	case VK_DOWN:
		m->down = true;
		break;
	case VK_A:
	case VK_LEFT:
		m->left = true;
		break;
	case VK_D:
	case VK_RIGHT:
		m->right = true;
		break;
	default:
		break;
	}
}

void PlayLayer::onKeyReleased(int keyCode) {
	switch (keyCode) {
	case VK_W:
	case VK_UP:
		m->up = false;
		break;
	case VK_S:
	case VK_DOWN:
		m->down = false;
		break;
	case VK_A:
	case VK_LEFT:
		m->left = false;
		break;
	case VK_D:
	case VK_RIGHT:
		m->right = false;
		break;
	default:
		break;
	}
}