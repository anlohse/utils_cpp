/*
 * component.hpp
 *
 *  Created on: 03/08/2014
 *      Author: Alan
 */

#ifndef COMPONENT_HPP_
#define COMPONENT_HPP_

#include <utils/ui/graphics.hpp>
#include <utils/ui/cursor.hpp>
#include <utils/ui/icon.hpp>
#include <utils/ui/ui_allocator.hpp>
#include <utils/ui/ref.hpp>
#include <list>

#if defined(UTILS_WINDOWS)
#include <windows.h>
#else

#endif

namespace utils {

namespace ui {

// struct, matching the definition in event.hpp -- declaring it as `class` here
// made every translation unit that saw both spellings warn.
struct EventListener;
class Event;
class UIController;
class Container;

struct component_data {

#if defined(UTILS_WINDOWS)
	HWND hWnd;
	HDC hdc;
	WNDPROC wndProc;
	int tab_order;
	component_data() : hWnd(NULL), hdc(NULL), wndProc(NULL), tab_order(-1) { }

#else

#endif

};


class Component : public UIObject {
public:
	// std::list rather than std::forward_list: the container list needs
	// push_back, and keeping both list types the same keeps the iteration
	// idiom uniform across the UI.
	typedef std::list<EventListener*, ui_allocator<EventListener*> > listener_list;
	typedef listener_list::const_iterator listener_iterator;
private:
	friend class UIController;
	friend class WindowUIController;
	friend class Window;
	component_data internal_data;
	Container* _parent;
	Ref<Cursor> _cursor;
	Ref<Font> _font;
	Ref<FillStyle> _background;
	Graphics* _graphics;
	UIController* _ui_controller;
	t_qword _last_click_time;
	int _click_count;
	Size _MaximumSize;
	Size _MinimumSize;
	Size _DefaultSize;
	Rect _BorderRect;
	listener_list _mouse_listeners;
	listener_list _mousewheel_listeners;
	listener_list _key_listeners;
	listener_list _focus_listeners;
	listener_list _component_listeners;
	listener_list _resizing_listeners;
	listener_list _moving_listeners;
	listener_list _paint_listeners;
	listener_list _command_listeners;
	void _proccessClicks(t_qword etime);

protected:

	void callListeners(listener_list& list, Event* event);
	Component();

	virtual const char* getClassName() const;

	virtual UIController* getDefaultUIController() const;

#if defined(UTILS_WINDOWS)
	static LRESULT CALLBACK componentWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void registerClass(UINT clsStyle, Icon* icon, Icon* smIcon, Cursor* cursor, const char* menuResName, WNDPROC classProc);
	static void unRegisterClass(const char* className);
	static bool isClassRegistred(const char* className);
	void createWindow(Container* parent, DWORD exStyle, const char* wndTitle, DWORD style, int left, int top, int width, int height, WNDPROC subclassProc);
	HWND getHWnd();
#else

#endif

public:
	Component(Container* parent, int left, int top, int width, int height);
	virtual ~Component();

	virtual UIController* getUIController();
	virtual void setUIController(UIController* uiController);

	virtual void setSize(int width, int height);
	virtual Size getSize() const;

	virtual void setPosition(int left, int top);
	virtual Point getPosition() const;

	virtual void setRect(const Rect& rect);
	virtual Rect getRect() const;

	virtual void setEnabled(bool enabled);
	virtual bool isEnabled() const;

	virtual void setFocus();
	virtual bool isFocused() const;

	virtual void setFocusable(bool focus);
	virtual bool isFocusable();

	virtual void setVisible(bool visible);
	virtual bool isVisible() const;

	virtual void setTabOrder(int order);
	virtual int isTabOrder() const;

	virtual void setTransparent(bool transparent);
	virtual bool isTransparent();

	virtual void setCursor(Cursor* cursor);
	virtual Cursor* getCursor() const;

	virtual void repaint();
	virtual void paint(Graphics* graphics);

	virtual void invalidate();
	virtual void invalidate(const Rect& rect);

	virtual void setBackground(FillStyle* color);
	virtual FillStyle* getBackground() const;

	virtual void setColor(const Color& color);
	virtual Color getColor() const;

	virtual void setParent(Container* parent);
	virtual Container* getParent() const;

	virtual void setFont(Font* font);
	virtual Font* getFont() const;

	virtual Size getMaximumSize() const;
	virtual Size getMinimumSize() const;
	virtual Size getDefaultSize() const;
	virtual Rect getBorderRect() const;

	virtual void setMaximumSize(const Size& v);
	virtual void setMinimumSize(const Size& v);
	virtual void setDefaultSize(const Size& v);
	virtual void setBorderRect(const Rect& v);

	virtual void setId(int id);
	virtual int getId() const;

	virtual void setText(const std::string& text);
	virtual std::string getText() const;

	virtual void setTextW(const std::wstring& text);
	virtual std::wstring getTextW() const;

	virtual Graphics* getGraphics();

	virtual void addMouseListener(EventListener* listener);
	virtual void removeMouseListener(EventListener* listener);
	virtual listener_iterator getMouseListeners();

	virtual void addMouseWheelListener(EventListener* listener);
	virtual void removeMouseWheelListener(EventListener* listener);
	virtual listener_iterator getMouseWheelListeners();

	virtual void addKeyListener(EventListener* listener);
	virtual void removeKeyListener(EventListener* listener);
	virtual listener_iterator getKeyListeners();

	virtual void addFocusListener(EventListener* listener);
	virtual void removeFocusListener(EventListener* listener);
	virtual listener_iterator getFocusListeners();

	virtual void addComponentListener(EventListener* listener);
	virtual void removeComponentListener(EventListener* listener);
	virtual listener_iterator getComponentListeners();

	virtual void addResizingListener(EventListener* listener);
	virtual void removeResizingListener(EventListener* listener);
	virtual listener_iterator getResizingListeners();

	virtual void addMovingListener(EventListener* listener);
	virtual void removeMovingListener(EventListener* listener);
	virtual listener_iterator getMovingListeners();

	virtual void addPaintListener(EventListener* listener);
	virtual void removePaintListener(EventListener* listener);
	virtual listener_iterator getPaintListeners();

	virtual void addCommandListener(EventListener* listener);
	virtual void removeCommandListener(EventListener* listener);
	virtual listener_iterator getCommandListeners();

};

class UIController : public UIObject {
public:
	UIController();
	virtual ~UIController();

	virtual bool handleEvent(Component* component, Event* event);

	virtual void paint(Component* component, Graphics* graphics);

	virtual bool eraseBackground(Component* component, Graphics* graphics);

	virtual bool defaultPaint(Component* component);

public:

	static UIController* getUIController(Component* component);
	static bool registerUIController(UIController* controller, const char* className);
};

} // ui

} // utils

#endif /* COMPONENT_HPP_ */
