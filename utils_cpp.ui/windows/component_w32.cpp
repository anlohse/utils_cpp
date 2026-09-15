/*
 * component.cpp
 *
 *  Created on: 03/08/2014
 *      Author: Alan
 */

#include <utils/ui/event.hpp>
#include <utils/ui/component.hpp>
#include <utils/ui/container.hpp>
#include <utils/ui/window.hpp>
#include <utils/time.hpp>
#include "gdi_graphics.h"
#include "int_datat.h"
#include "utils_w32.h"
#include <utils/exception.hpp>
#include <iostream>
#include <cstdio>
#include <set>
#include <commctrl.h>

namespace utils {

namespace ui {

LRESULT CALLBACK Component::componentWindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam
) {
	void* usr_data = getWindowPointer(hwnd, GWLP_USERDATA);
	if (usr_data == NULL) {
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}
	Component* comp = reinterpret_cast<Component*>(usr_data);
	bool consumed = false;

	switch (uMsg) {
		// -------------------------------------- Window Events --------------------------------------------------
		case WM_DESTROY: {
			ComponentEvent evt(comp, Time::milliseconds(), ComponentEvent::DESTROY);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_SHOWWINDOW: {
			ComponentEvent evt(comp, Time::milliseconds(), wParam == TRUE ? ComponentEvent::SHOW : ComponentEvent::HIDE);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_SIZE: {
			ResizeEvent evt(comp,Time::milliseconds(), ComponentEvent::RESIZE,Size(LOWORD(lParam),HIWORD(lParam)));
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_SIZING : {
			ResizingEvent evt(comp,Time::milliseconds(), ComponentEvent::RESIZING,reinterpret_cast<Rect*>(lParam), wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		return TRUE;
		case WM_MOVE: {
			MoveEvent evt(comp,Time::milliseconds(), ComponentEvent::MOVE,Point(LOWORD(lParam),HIWORD(lParam)));
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
 		break;
		case WM_MOVING: {
			RECT* lprect = reinterpret_cast<RECT*>(lParam);
			int oldleft =lprect->left;
			int oldtop =lprect->top;
			MovingEvent evt(comp,Time::milliseconds(), ComponentEvent::MOVING,reinterpret_cast<Point*>(lParam));
			consumed = comp->getUIController()->handleEvent(comp,&evt);
			lprect->right += lprect->left - oldleft;
			lprect->bottom += lprect->top - oldtop;
		}
 		return TRUE;
		case WM_PAINT: if (comp->getUIController()->defaultPaint(comp)) {
			consumed = false;
		} else {
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			Graphics* grap = createBackendGraphics(ps.hdc, false);
			if (grap != NULL) {
				PaintEvent evt(comp,Time::milliseconds(),ComponentEvent::PAINT, grap);
				comp->getUIController()->handleEvent(comp,&evt);
				destroyBackendGraphics(grap);
			}
			EndPaint(hwnd,&ps);
			consumed = true;
		}
		break;
		case WM_ERASEBKGND: {
			Graphics* gra = createBackendGraphics((HDC)wParam, false);
			if (gra != NULL) {
				PaintEvent evt(comp,Time::milliseconds(),ComponentEvent::ERASE_BACKGROUND, gra);
				consumed = comp->getUIController()->handleEvent(comp,&evt);
				destroyBackendGraphics(gra);
			}
		}
		return 1;
		// -------------------------------------- Mouse Events --------------------------------------------------
		case WM_MOUSEMOVE: {
			MouseEvent evt(comp, Time::milliseconds(), MouseEvent::MOUSE_MOVE, LOWORD(lParam), HIWORD(lParam), wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_LBUTTONDOWN: {
			MouseEvent evt(comp, Time::milliseconds(), MouseEvent::MOUSE_DOWN, LOWORD(lParam), HIWORD(lParam), InputEvent::MOUSE_BUTTON1, wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_RBUTTONDOWN: {
			MouseEvent evt(comp, Time::milliseconds(), MouseEvent::MOUSE_DOWN, LOWORD(lParam), HIWORD(lParam), InputEvent::MOUSE_BUTTON2, wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_MBUTTONDOWN: {
			MouseEvent evt(comp, Time::milliseconds(), MouseEvent::MOUSE_DOWN, LOWORD(lParam), HIWORD(lParam), InputEvent::MOUSE_MBUTTON, wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_LBUTTONUP: {
			t_bigint etime = Time::milliseconds();
			comp->_proccessClicks(etime);
			MouseEvent evt(comp, etime, MouseEvent::CLICK, LOWORD(lParam), HIWORD(lParam), InputEvent::MOUSE_BUTTON1,comp->_click_count, wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
			MouseEvent evt2(comp, etime, MouseEvent::MOUSE_UP, LOWORD(lParam), HIWORD(lParam), InputEvent::MOUSE_BUTTON1, wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt2);
		}
		break;
		case WM_RBUTTONUP: {
			t_bigint etime = Time::milliseconds();
			comp->_proccessClicks(etime);
			MouseEvent evt(comp, etime, MouseEvent::CLICK, LOWORD(lParam), HIWORD(lParam), InputEvent::MOUSE_BUTTON2,comp->_click_count, wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
			MouseEvent evt2(comp, etime, MouseEvent::MOUSE_UP, LOWORD(lParam), HIWORD(lParam), InputEvent::MOUSE_BUTTON2,wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt2);
		}
		break;
		case WM_MBUTTONUP: {
			t_bigint etime = Time::milliseconds();
			comp->_proccessClicks(etime);
			MouseEvent evt(comp, etime, MouseEvent::CLICK, LOWORD(lParam), HIWORD(lParam), InputEvent::MOUSE_MBUTTON,comp->_click_count, wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
			MouseEvent evt2(comp, etime, MouseEvent::MOUSE_UP, LOWORD(lParam), HIWORD(lParam), InputEvent::MOUSE_MBUTTON, wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt2);
		}
		break;
		case WM_XBUTTONDOWN: {
			MouseEvent evt(comp, Time::milliseconds(), MouseEvent::MOUSE_DOWN, LOWORD(lParam), HIWORD(lParam), (HIWORD(wParam) == XBUTTON1) ? InputEvent::MOUSE_XBUTTON1 : InputEvent::MOUSE_XBUTTON2,LOWORD(wParam));
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_XBUTTONUP: {
			t_bigint etime = Time::milliseconds();
			comp->_proccessClicks(etime);
			MouseEvent evt(comp, etime, MouseEvent::CLICK, LOWORD(lParam), HIWORD(lParam), (HIWORD(wParam) == XBUTTON1) ? InputEvent::MOUSE_XBUTTON1 : InputEvent::MOUSE_XBUTTON2,comp->_click_count, LOWORD(wParam));
			consumed = comp->getUIController()->handleEvent(comp,&evt);
			MouseEvent evt2(comp, etime, MouseEvent::MOUSE_UP, LOWORD(lParam), HIWORD(lParam), (HIWORD(wParam) == XBUTTON1) ? InputEvent::MOUSE_XBUTTON1 : InputEvent::MOUSE_XBUTTON2, LOWORD(wParam));
			consumed = comp->getUIController()->handleEvent(comp,&evt2);
		}
		break;
		case WM_MOUSEHOVER: {
			MouseEvent evt(comp, Time::milliseconds(), MouseEvent::MOUSE_ENTER, LOWORD(lParam), HIWORD(lParam),(int)wParam);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_MOUSELEAVE: {
			MouseEvent evt(comp, Time::milliseconds(), MouseEvent::MOUSE_LEAVE, 0, 0, 0);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_MOUSEWHEEL: {
			int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			int keyState = GET_KEYSTATE_WPARAM(wParam);
			MouseWheelEvent evt(comp, Time::milliseconds(), MouseEvent::MOUSE_ENTER, LOWORD(lParam), HIWORD(lParam),
					0,0,keyState,delta,MouseWheelEvent::MODE_UNIT);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		// -------------------------------------- Keyboard Events --------------------------------------------------
		case WM_CHAR:
		case WM_UNICHAR : {
			KeyEvent evt(comp,Time::milliseconds(), KeyEvent::KEY_CHAR, (int)wParam, (int)wParam, 0);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_KEYDOWN: {
			KeyEvent evt(comp,Time::milliseconds(), KeyEvent::KEY_DOWN, (int)wParam, 0, 0);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_KEYUP: {
			KeyEvent evt(comp,Time::milliseconds(), KeyEvent::KEY_UP, (int)wParam, 0, 0);
			consumed = comp->getUIController()->handleEvent(comp,&evt);
		}
		break;
		case WM_SETFOCUS:
		case WM_KILLFOCUS: {
			Component* _oc = reinterpret_cast<Component*>(wParam != 0 ? getWindowPointer((HWND)wParam, GWLP_USERDATA) : NULL);
			FocusEvent evt(comp,Time::milliseconds(),uMsg == WM_SETFOCUS ? FocusEvent::GOT_FOCUS : FocusEvent::LOST_FOCUS,_oc);
		}
		break;

	}
	if (consumed) {
		return 0;
	} else {
		if (comp->internal_data.wndProc)
			return comp->internal_data.wndProc(hwnd,uMsg,wParam,lParam);
		else
			return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}
}

void Component::_proccessClicks(t_qword etime) {
	if (etime - _last_click_time <= GetDoubleClickTime())
		_click_count++;
	else
		_click_count = 0;
	_last_click_time = etime;
}
typedef std::set<std::string, std::less<std::string>, DEFAULT_ALLOCATOR<std::string> > classes_list;

const char* _default_classes[] = {
		"BUTTON", "EDIT"
};

classes_list initClasses() {
	InitCommonControls();
	return classes_list(_default_classes,_default_classes+2);
}

static classes_list registred_classes = initClasses();

void Component::registerClass(UINT clsStyle, Icon* icon, Icon* smIcon, Cursor* cursor, const char* menuResName, WNDPROC classProc) {
	WNDCLASSEXA wndc;
	wndc.cbSize = sizeof(WNDCLASSEXA);
	wndc.style = clsStyle;
	wndc.lpfnWndProc = classProc ? classProc : &DefWindowProc;
	wndc.cbClsExtra = 0;
	wndc.cbWndExtra = 0;
	wndc.hInstance = GetModuleHandleA(NULL);
	wndc.hIcon = icon ? (HICON)icon->internal_data : NULL;
	wndc.hCursor = _cursor ? (HCURSOR)_cursor->internal_data : NULL;
	wndc.hbrBackground = NULL;
	wndc.lpszMenuName = menuResName;
	wndc.lpszClassName = getClassName();
	wndc.hIconSm = smIcon ? (HICON)smIcon->internal_data : NULL;

	if (RegisterClassExA(&wndc)) {
		registred_classes.insert(getClassName());
	}
}
bool Component::isClassRegistred(const char* className) {
	classes_list::const_iterator it = registred_classes.find(className);
	if (it != registred_classes.end())
		return true;
	return false;
}
void Component::unRegisterClass(const char* className) {
	if (isClassRegistred(className)) {
		UnregisterClassA(className,GetModuleHandle(NULL));
		registred_classes.erase(className);
	}
}
void Component::createWindow(Container* parent, DWORD exStyle, const char* wndTitle, DWORD style, int left, int top, int width, int height, WNDPROC subclassProc) {

	if (left < 0) left = CW_USEDEFAULT;
	if (top < 0) top = CW_USEDEFAULT;
	if (width < 0) width = CW_USEDEFAULT;
	if (height < 0) height = CW_USEDEFAULT;
	internal_data.hWnd = CreateWindowExA(exStyle,getClassName(), wndTitle, style,
			left, top, width, height, parent ? parent->internal_data.hWnd : NULL, NULL, GetModuleHandle(NULL), (void*)this);
	_parent = parent;
	if (internal_data.hWnd) {
		setWindowPointer(internal_data.hWnd, GWLP_USERDATA, (void*)this);
		if (subclassProc)
			internal_data.wndProc = (WNDPROC) setWindowPointer(internal_data.hWnd, GWLP_WNDPROC, (void*)subclassProc);
		ComponentEvent evt(this, Time::milliseconds(), ComponentEvent::CREATE);
		getUIController()->handleEvent(this,&evt);
	}
	if (_parent)
		_parent->addChild(this);
}

HWND Component::getHWnd() {
	return internal_data.hWnd;
}


Component::Component() :
		internal_data(),
		_parent(),
		_cursor(),
		_font(),
		_background(),
		_graphics(),
		_ui_controller(),
		_last_click_time((t_qword)-1),
		_click_count(0),
		_MaximumSize(),
		_MinimumSize(),
		_DefaultSize(),
		_BorderRect(),
		_mouse_listeners(),
		_mousewheel_listeners(),
		_key_listeners(),
		_focus_listeners(),
		_component_listeners(),
		_resizing_listeners(),
		_moving_listeners(),
		_paint_listeners(),
		_command_listeners() {

}

Component::Component(Container* parent, int left, int top, int width, int height) :
				internal_data(),
				_parent(),
				_cursor(),
				_font(),
				_background(),
				_graphics(),
				_ui_controller(),
				_last_click_time((t_qword)-1),
				_click_count(0),
				_MaximumSize(),
				_MinimumSize(),
				_DefaultSize(),
				_BorderRect(),
				_mouse_listeners(),
				_mousewheel_listeners(),
				_key_listeners(),
				_focus_listeners(),
				_component_listeners(),
				_resizing_listeners(),
				_moving_listeners(),
				_paint_listeners(),
				_command_listeners() {

	_cursor= Cursor::getSystemCursor(SystemCursor::ARROW);
	_cursor->add_reference();

	if (!isClassRegistred(getClassName()))
		registerClass(CS_PARENTDC | CS_VREDRAW | CS_HREDRAW, NULL, NULL, _cursor, NULL, &Component::componentWindowProc);

	_font = GdiFont::createFontImpl("Sans Serif",Font::PLAIN,11);
	_font->add_reference();
	_background = new GdiColorBrush((HBRUSH)(COLOR_BTNFACE+1), false);
	_background->add_reference();

	createWindow(parent,0,"", (parent ? WS_CHILD : WS_POPUP) | WS_TABSTOP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			left, top, width, height, NULL);

	int err = GetLastError();
	if (err != 0)
		std::cout << __FILE__ << "(" << __LINE__ << ") " << get_error_message(err) << std::endl;

}
Component::~Component() {
	if (_font && _font->rem_reference()->get_references() < 1)
		Font::destroyFont(_font);
	if (_cursor && _cursor->rem_reference()->get_references() < 1)
		Cursor::deleteCursor(_cursor);
	if (_background && _background->rem_reference()->get_references() < 1)
		delete _background;
	if (_graphics) {
		delete _graphics;
		if (Graphics::getGraphicsImplementation() == GRAPHICS_GDI) {
			ReleaseDC(internal_data.hWnd,internal_data.hdc);
		}
	}
	DestroyWindow(internal_data.hWnd);
}

void Component::setSize(int width, int height) {
	Size min = getMinimumSize();
	Size max = getMaximumSize();
	if (min.width > 0 && width < min.width)
		width = min.width;
	if (max.width > 0 && width > max.width)
		width = max.width;
	if (min.height > 0 && height < min.height)
		height = min.height;
	if (max.height > 0 && height > max.height)
		height = max.height;
	SetWindowPos(internal_data.hWnd,(HWND)0,0,0,width,height,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
}
Size Component::getSize() const {
	// The CLIENT area, not the window rect. This is the space a container can
	// lay children into and the area paint() draws over -- eraseBackground
	// already assumes it by filling from (0,0). GetWindowRect included the
	// title bar and borders, so both overshot on any framed window.
	RECT rc;
	GetClientRect(internal_data.hWnd,&rc);
	return Size(rc.right-rc.left,rc.bottom-rc.top);
}

/**
 * The window rectangle in the coordinate space setPosition/setRect expect:
 * the parent's client area, or the screen for a top-level window.
 *
 * GetWindowRect alone reports screen coordinates, so getPosition() and
 * setPosition() disagreed for every child window and geometry did not round
 * trip.
 */
static void getRectInParentSpace(HWND hWnd, RECT* out) {
	::GetWindowRect(hWnd, out);
	HWND parent = ::GetParent(hWnd);
	if (parent != NULL)
		::MapWindowPoints(HWND_DESKTOP, parent, reinterpret_cast<POINT*>(out), 2);
}

void Component::setPosition(int left, int top) {
	SetWindowPos(internal_data.hWnd,(HWND)0,left, top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}
Point Component::getPosition() const {
	RECT rc;
	getRectInParentSpace(internal_data.hWnd,&rc);
	return Point(rc.left,rc.top);
}

void Component::setRect(const Rect& rect) {
	// right/bottom are exclusive, matching getRect() and Win32 RECT. The
	// previous +1 meant setRect(getRect()) grew the window by a pixel each
	// time it was applied.
	SetWindowPos(internal_data.hWnd,(HWND)0,rect.left, rect.top,
			rect.right-rect.left,rect.bottom-rect.top,SWP_NOZORDER|SWP_NOACTIVATE);
}
Rect Component::getRect() const {
	RECT rc;
	getRectInParentSpace(internal_data.hWnd,&rc);
	return Rect(rc.left,rc.top,rc.right,rc.bottom);
}

void Component::setEnabled(bool enabled) {
	EnableWindow(internal_data.hWnd,enabled);
}
bool Component::isEnabled() const {
	return IsWindowEnabled(internal_data.hWnd);
}

void Component::setFocus() {
	SetFocus(internal_data.hWnd);
}
bool Component::isFocused() const {
	return GetFocus() == internal_data.hWnd;
}

void Component::setFocusable(bool focus) {
}
bool Component::isFocusable() {
	return false;
}

void Component::setVisible(bool visible) {
	ShowWindow(internal_data.hWnd, visible ? SW_SHOW : SW_HIDE);
	UpdateWindow(internal_data.hWnd);
}
bool Component::isVisible() const {
	return IsWindowVisible(internal_data.hWnd);
}

void Component::setTransparent(bool transparent) {
}
bool Component::isTransparent() {
	return false;
}

void Component::setTabOrder(int order) {

}
int Component::isTabOrder() const {
	return -1;
}

void Component::setCursor(Cursor* cursor) {
	SAFE_DELETE(_cursor);
	_cursor = cursor;
	if (cursor) {
		cursor->add_reference();
		setClassPointer(internal_data.hWnd, GCLP_HCURSOR, (void*)cursor->internal_data);
	} else {
		setClassPointer(internal_data.hWnd, GCLP_HCURSOR, NULL);
	}
}
Cursor* Component::getCursor() const {
	return _cursor;
}

void Component::repaint() {
	RECT rc;
	GetWindowRect(internal_data.hWnd, &rc);
	RedrawWindow(internal_data.hWnd, &rc, NULL, RDW_ERASE | RDW_ALLCHILDREN);
}
void Component::paint(Graphics* graphics) {
}

void Component::invalidate() {
	InvalidateRect(internal_data.hWnd, NULL, TRUE);
}
void Component::invalidate(const Rect& rect) {
	RECT rc = {rect.left, rect.top, rect.right, rect.bottom};
	InvalidateRect(internal_data.hWnd, &rc, TRUE);
}

void Component::setBackground(FillStyle* color) {
	SAFE_DELETE(_background);
	_background = color;
	if (_background)
		_background->add_reference();
	repaint();
}
FillStyle* Component::getBackground() const {
	return _background;
}

void Component::setId(int id) {
	// Widen to pointer size explicitly; a bare (void*) cast from int is a
	// narrowing-to-wider conversion the compiler rightly flags on 64-bit.
	setWindowPointer(internal_data.hWnd, GWLP_ID, reinterpret_cast<void*>(static_cast<ptrdiff_t>(id)));
}
int Component::getId() const {
	return (int) reinterpret_cast<ptrdiff_t>(getWindowPointer(internal_data.hWnd, GWLP_ID));
}

void Component::setText(const std::string& text) {
	SetWindowTextA(internal_data.hWnd, text.c_str());
}
std::string Component::getText() const {
	char buf[65536];
	GetWindowTextA(internal_data.hWnd,buf,65536);
	return buf;
}

void Component::setTextW(const std::wstring& text) {
	SetWindowTextW(internal_data.hWnd, text.c_str());
}
std::wstring Component::getTextW() const {
	wchar_t buf[65536];
	GetWindowTextW(internal_data.hWnd,buf,65536);
	return buf;
}

void Component::setColor(const Color& color) {
}
Color Component::getColor() const {
	return Color();
}

void Component::setParent(Container* parent) {
	if (_parent == parent)
		return;
	if (parent == NULL) {
		SetParent(internal_data.hWnd,NULL);
		_parent = NULL;
		return;
	}
	if (_parent) {
		_parent->removeChild(this);
	}
	SetParent(internal_data.hWnd,parent->internal_data.hWnd);
//	int err = GetLastError();
//	if (err != 0)
//		std::cout << __FILE__ << "(" << __LINE__ << ") " << get_error_message(err) << std::endl;
	if (dynamic_cast<Window*>(this) == NULL) {
		long stl = GetWindowLong(internal_data.hWnd,GWL_STYLE);
		if (HAS_VALUE(stl,WS_POPUP)) {
			TOGGLE_VALUE(stl,WS_POPUP, false);
			TOGGLE_VALUE(stl,WS_CHILD, true);
			SetWindowLong(internal_data.hWnd,GWL_STYLE,stl);
		}
	}
	setVisible(true);
	_parent = parent;
}
Container* Component::getParent() const {
	return _parent;
}

void Component::setFont(Font* font) {
	if (font) {
		font->add_reference();
		SAFE_DELETE(_font);
		_font = font;
		SendMessageA(internal_data.hWnd, WM_SETFONT, (WPARAM)((GdiFont*)_font)->getHandle(), 0);
	}
}
Font* Component::getFont() const {
	return _font;
}

Graphics* Component::getGraphics() {
	if (_graphics == NULL) {
		internal_data.hdc = GetDC(internal_data.hWnd);
		_graphics = createBackendGraphics(internal_data.hdc, false);
	}
	return _graphics;
}

//--------------------------------------------------------------------------------------------------------------------------



}

}
