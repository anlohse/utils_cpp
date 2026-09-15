/*
 * button_w32.cpp
 *
 *  Created on: 23/01/2016
 *      Author: Alan
 */

#include <utils/ui/button.hpp>
#include "gdi_graphics.h"
#include <windows.h>
#include "utils_w32.h"

namespace utils {

namespace ui {

const char* Button::getClassName() const {
	return "BUTTON";
}

void Button::createButton(Container* parent, int left, int top, int width, int height) {

	createWindow(parent,0,"", parent ? WS_CHILD | WS_TABSTOP | WS_VISIBLE : WS_POPUP,
			left, top, width, height, &Component::componentWindowProc);

	setCursor(Cursor::getSystemCursor(SystemCursor::ARROW));
	setFont(GdiFont::createFontImpl("Sans Serif",Font::PLAIN,13));
	setBackground(new GdiColorBrush((HBRUSH)(COLOR_BTNFACE+1), false));
}

Button::Button(Container* parent) :
		Component(), _image(), _disabled_image(), _pressed_image() {
	createButton(parent,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT);
}

Button::Button() :
		Component(), _image(), _disabled_image(), _pressed_image() {
	createButton(NULL,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT);
}

void Button::click() {
	PostMessageA(getHWnd(), WM_RBUTTONDOWN, MK_LBUTTON, 0);
	PostMessageA(getHWnd(), WM_RBUTTONUP, MK_LBUTTON, 0);
}

void ButtonUIController::paint(Component* component, Graphics* graphics) {
	if (component == NULL || graphics == NULL)
		return;
	const Size sz = component->getSize();
	const float w = (float) sz.width;
	const float h = (float) sz.height;

	// DrawFrameControl needs an HDC, which only the GDI backend can supply.
	// This used to dynamic_cast straight to GdiGraphics and dereference the
	// result -- fine while GDI was the only implementation, a null dereference
	// the moment any other backend was selected. Use it when it is available
	// and fall back to drawing the bevel through the Graphics interface.
	GdiGraphics* gdi = dynamic_cast<GdiGraphics*>(graphics);
	if (gdi != NULL && gdi->hdc != NULL) {
		RECT rc = {0, 0, sz.width, sz.height};
		DrawFrameControl(gdi->hdc, &rc, DFC_BUTTON, DFCS_BUTTONPUSH);
	} else {
		graphics->setFillColor(Color(0xf0, 0xf0, 0xf0));
		graphics->fillRect(0, 0, w, h);
		// A two-tone bevel: light along the top and left, dark elsewhere.
		const bool wasAA = graphics->getAntialias();
		graphics->setAntialias(false);
		graphics->setFillColor(Color(0xff, 0xff, 0xff));
		graphics->fillRect(0, 0, w, 1);
		graphics->fillRect(0, 0, 1, h);
		graphics->setFillColor(Color(0x70, 0x70, 0x70));
		graphics->fillRect(0, h - 1, w, 1);
		graphics->fillRect(w - 1, 0, 1, h);
		graphics->setAntialias(wasAA);
	}

	Button* btn = dynamic_cast<Button*>(component);
	if (btn != NULL && btn->_image)
		graphics->drawImage(btn->_image.get(), 0, 0);
}

} // ui

} // utils
