/*
 * window_w32.cpp
 *
 *  Created on: 02/05/2015
 *      Author: ALAN
 */

#include <utils/ui/window.hpp>
#include <utils/ui/event.hpp>
#include <utils/time.hpp>
#include <winuser.h>
#include <cstdio>
#include "gdi_graphics.h"
#include "utils_w32.h"

namespace utils {
namespace ui {

LRESULT CALLBACK Window::windowWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {

	void* usr_data = getWindowPointer(hwnd, GWLP_USERDATA);
	if (usr_data == NULL) {
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}
	Window* comp = reinterpret_cast<Window*>(usr_data);
	switch (uMsg) {
	case WM_ACTIVATEAPP: {
		WindowEvent evt(comp,Time::milliseconds(),WindowEvent::ACTIVATED);
		comp->getUIController()->handleEvent(comp,&evt);
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}
	case WM_NCACTIVATE: {
		WindowEvent evt(comp,Time::milliseconds(),WindowEvent::DEACTIVATED);
		comp->getUIController()->handleEvent(comp,&evt);
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}
	case WM_CLOSE: {
		WindowEvent evt(comp,Time::milliseconds(),WindowEvent::CLOSE);
		comp->getUIController()->handleEvent(comp,&evt);
		if (!evt.defaultCanceled()) {
			if (comp->_defCloseAction == CloseAction::QUIT || comp->_modal)
				PostQuitMessage(0);
			else if (comp->_defCloseAction == CloseAction::DESTROY)
				DestroyWindow(comp->internal_data.hWnd);
			else if (comp->_defCloseAction == CloseAction::HIDE)
				comp->setVisible(false);
		}
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}
	case WM_SIZE: {
		if (wParam == SIZE_MINIMIZED) {
			WindowEvent evt(comp,Time::milliseconds(),WindowEvent::ICONIFIED);
			comp->getUIController()->handleEvent(comp,&evt);
			return DefWindowProc(hwnd,uMsg,wParam,lParam);
		} else if (wParam == SIZE_RESTORED) {
			WindowEvent evt(comp,Time::milliseconds(),WindowEvent::DEICONIFIED);
			comp->getUIController()->handleEvent(comp,&evt);
			return DefWindowProc(hwnd,uMsg,wParam,lParam);
		}
		break;
	}
	}
	return Component::componentWindowProc(hwnd,uMsg,wParam,lParam);
}

Window::Window(Window* parent) : _modal(false), _notopen(true), _defCloseAction(CloseAction::DO_NOTHING), _icon(), _smallIcon(), _window_listeners(), _command_listeners() {
	Window(parent, "", -1, -1, -1, -1);
}
Window::Window(Window* parent, const char* title, int left, int top, int width, int height) :
		_modal(false), _notopen(true), _defCloseAction(CloseAction::DO_NOTHING), _icon(), _smallIcon(), _window_listeners(), _command_listeners() {

	if (!isClassRegistred(getClassName()))
		registerClass(CS_OWNDC | CS_VREDRAW | CS_HREDRAW, NULL, NULL, NULL, NULL, &Window::windowWindowProc);

	createWindow(parent,WS_EX_CONTROLPARENT,title, WS_POPUPWINDOW | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			left, top, width, height, NULL);

	setCursor(Cursor::getSystemCursor(SystemCursor::ARROW));
	setFont(GdiFont::createFontImpl("Sans Serif",Font::PLAIN,11));
	setBackground(new GdiColorBrush((HBRUSH)(COLOR_WINDOW+1), false));
}

void Window::setIcon(Icon* icon) {
	_icon = icon;
	if (icon)
		setClassPointer(internal_data.hWnd, GCLP_HICON, icon->internal_data);
	else
		setClassPointer(internal_data.hWnd, GCLP_HICON, NULL);
}
Icon* Window::getIcon() const {
	return _icon;
}

void Window::setSmallIcon(Icon* icon) {
	_smallIcon = icon;
	if (icon)
		setClassPointer(internal_data.hWnd, GCLP_HICONSM, icon->internal_data);
	else
		setClassPointer(internal_data.hWnd, GCLP_HICONSM, NULL);
}
Icon* Window::getSmallIcon() const {
	return _smallIcon;
}

void Window::setTitle(const char* title) {
	SetWindowTextA(internal_data.hWnd, title);
}
void Window::getTitle(char* title, int maxlen) const {
	if (title)
		GetWindowTextA(internal_data.hWnd, title, maxlen);
}

void Window::setTitle(const wchar_t* title) {
	SetWindowTextW(internal_data.hWnd, title);
}
void Window::getTitle(wchar_t* title, int maxlen) const {
	if (title)
		GetWindowTextW(internal_data.hWnd, title, maxlen);
}

void Window::setAlwaysOnTop(bool always) {
	long v = GetWindowLong(internal_data.hWnd,GWL_EXSTYLE);
	TOGGLE_VALUE(v,WS_EX_TOOLWINDOW,always);
	SetWindowLong(internal_data.hWnd,GWL_EXSTYLE,v);
}
bool Window::isAlwaysOnTop() const {
	long v = GetWindowLong(internal_data.hWnd,GWL_EXSTYLE);
	return HAS_VALUE(v, WS_EX_TOOLWINDOW);
}

void Window::setResizable(bool value) {
	long v = GetWindowLong(internal_data.hWnd,GWL_STYLE);
	TOGGLE_VALUE(v,WS_SIZEBOX,value);
	SetWindowLong(internal_data.hWnd,GWL_STYLE,v);
}
bool Window::isResizable() const {
	long v = GetWindowLong(internal_data.hWnd,GWL_STYLE);
	return HAS_VALUE(v, WS_SIZEBOX);
}

#define WINDOW_ST (WS_OVERLAPPEDWINDOW)
#define DIALOG_ST (WS_DLGFRAME | WS_CAPTION)
#define POPUP_ST (WS_POPUPWINDOW)

void Window::setType(const WindowType& type) {
	long st = GetWindowLong(internal_data.hWnd,GWL_STYLE);
	long ex = GetWindowLong(internal_data.hWnd,GWL_EXSTYLE);
	if (type == WindowType::WINDOW) {
		TOGGLE_VALUE(st,POPUP_ST,false);
		TOGGLE_VALUE(ex,WS_EX_TOOLWINDOW,false);
		TOGGLE_VALUE(st,DIALOG_ST,false);
		TOGGLE_VALUE(st,WINDOW_ST,true);
	} else if (type == WindowType::POPUP) {
		TOGGLE_VALUE(ex,WS_EX_TOOLWINDOW,false);
		TOGGLE_VALUE(st,DIALOG_ST,false);
		TOGGLE_VALUE(st,WINDOW_ST,false);
		TOGGLE_VALUE(st,POPUP_ST,true);
	} else if (type == WindowType::TOOL) {
		TOGGLE_VALUE(ex,WS_EX_TOOLWINDOW,true);
		TOGGLE_VALUE(st,DIALOG_ST,false);
		TOGGLE_VALUE(st,WINDOW_ST,false);
		TOGGLE_VALUE(st,POPUP_ST,false);
	} else if (type == WindowType::DIALOG) {
		TOGGLE_VALUE(ex,WS_EX_TOOLWINDOW,false);
		TOGGLE_VALUE(st,WINDOW_ST,false);
		TOGGLE_VALUE(st,POPUP_ST,false);
		TOGGLE_VALUE(st,DIALOG_ST,true);
	}
	SetWindowLong(internal_data.hWnd,GWL_STYLE, st);
	SetWindowLong(internal_data.hWnd,GWL_EXSTYLE, ex);
}
WindowType Window::getType() const {
	long st = GetWindowLong(internal_data.hWnd,GWL_STYLE);
	long ex = GetWindowLong(internal_data.hWnd,GWL_EXSTYLE);
	if (HAS_VALUE(st,WINDOW_ST))
		return WindowType::WINDOW;
	if (HAS_VALUE(st,POPUP_ST))
		return WindowType::POPUP;
	if (HAS_VALUE(ex,WS_EX_TOOLWINDOW))
		return WindowType::TOOL;
	if (HAS_VALUE(st,DIALOG_ST))
		return WindowType::DIALOG;
	return WindowType::WINDOW;
}

void Window::setMaximizable(bool maximizable) {
	long v = GetWindowLong(internal_data.hWnd,GWL_STYLE);
	TOGGLE_VALUE(v,WS_MAXIMIZEBOX,maximizable);
	SetWindowLong(internal_data.hWnd,GWL_STYLE,v);
}
bool Window::isMaximizable() const {
	long v = GetWindowLong(internal_data.hWnd,GWL_STYLE);
	return HAS_VALUE(v, WS_MAXIMIZEBOX);
}

void Window::setShowCloseButton(bool button) {
	long v = GetWindowLong(internal_data.hWnd,GWL_STYLE);
	TOGGLE_VALUE(v,WS_SYSMENU,button);
	SetWindowLong(internal_data.hWnd,GWL_STYLE,v);
}
bool Window::isShowCloseButton() const {
	long v = GetWindowLong(internal_data.hWnd,GWL_STYLE);
	return HAS_VALUE(v, WS_SYSMENU);
}

void Window::quit(int code) {
	PostQuitMessage(code);
}

void Window::setVisible(bool visible) {
	if (_notopen && visible) {
		_notopen = false;
		WindowEvent evt(this,Time::milliseconds(),WindowEvent::OPEN);
		getUIController()->handleEvent(this,&evt);
	}
	if (_is_main_loop && _modal && !isVisible()) {
		_modal_loop();
	} else {
		Component::setVisible(visible);
		setFocus();
		_main_loop();
	}
}

bool Window::_is_main_loop = false;
int Window::_main_loop() {
	MSG msg;
	int bRet;
	if (_is_main_loop)
		return 0;
	_is_main_loop = true;
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			int err = GetLastError();
			if (err != 0) {
				std::string r = get_error_message(err);
				MessageBoxA(NULL, r.c_str(), "Error", MB_OK | MB_ICONERROR);
			}
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	_is_main_loop = false;
	return msg.wParam;
}

int Window::_modal_loop() {
	MSG msg;
	int bRet;
	if (getParent()) getParent()->setEnabled(false);
	ShowWindow(internal_data.hWnd, SW_SHOW);
	UpdateWindow(internal_data.hWnd);
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (IsDialogMessage(internal_data.hWnd, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	if (getParent()) getParent()->setEnabled(true);
	if (getParent()) getParent()->setVisible(true);
	return msg.wParam;
}


} // ui

} // utils

