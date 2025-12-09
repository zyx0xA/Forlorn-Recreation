#pragma once

class UILayerDelegate {
public:
	virtual ~UILayerDelegate() {}

	virtual void uiUp() = 0;
	virtual void uiDown() = 0;
	virtual void uiLeft() = 0;
	virtual void uiRight() = 0;
};