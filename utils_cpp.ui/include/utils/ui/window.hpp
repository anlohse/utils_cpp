/*
 * window.hpp
 *
 *  Created on: 01/05/2015
 *      Author: ALAN
 */

#ifndef _COMP_WINDOW_HPP_
#define _COMP_WINDOW_HPP_

#include <utils/ui/container.hpp>

namespace utils {

namespace ui {

class WindowType {
	MAKE_ENUMERATION(WindowType);
	DECLARE_CONSTANT(WindowType,WINDOW);
	DECLARE_CONSTANT(WindowType,POPUP);
	DECLARE_CONSTANT(WindowType,TOOL);
	DECLARE_CONSTANT(WindowType,DIALOG);
};

class CloseAction {
	MAKE_ENUMERATION(CloseAction);
	DECLARE_CONSTANT(CloseAction,DO_NOTHING);
	DECLARE_CONSTANT(CloseAction,HIDE);
	DECLARE_CONSTANT(CloseAction,DESTROY);
	DECLARE_CONSTANT(CloseAction,QUIT);
};

class Window : public Container {
private:
	friend class WindowUIController;
	bool _modal;
	bool _notopen;
	CloseAction _defCloseAction;
	Ref<Icon> _icon;
	Ref<Icon> _smallIcon;
	listener_list _window_listeners;
	listener_list _command_listeners;
protected:
#if defined(UTILS_WINDOWS)
	static LRESULT CALLBACK windowWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static bool _is_main_loop;
	static int _main_loop();
	int _modal_loop();
#else

#endif

	virtual UIController* getDefaultUIController() const;

	Window();
public:
	Window(Window* parent);
	Window(Window* parent, const char* title, int left, int top, int width, int height);
	virtual ~Window();

	virtual void setIcon(Icon* icon);
	virtual Icon* getIcon() const;

	virtual void setSmallIcon(Icon* icon);
	virtual Icon* getSmallIcon() const;

	virtual void setTitle(const char* title);
	virtual void getTitle(char* title, int maxlen) const;

	virtual void setTitle(const wchar_t* title);
	virtual void getTitle(wchar_t* title, int maxlen) const;

	virtual void setAlwaysOnTop(bool always);
	virtual bool isAlwaysOnTop() const;

	virtual void setResizable(bool value);
	virtual bool isResizable() const;

	virtual void setType(const WindowType& type);
	virtual WindowType getType() const;

	virtual void setMaximizable(bool maximizable);
	virtual bool isMaximizable() const;

	virtual void setShowCloseButton(bool button);
	virtual bool isShowCloseButton() const;

	virtual void setCloseAction(const CloseAction& ca);
	virtual CloseAction getCloseAction() const;

	virtual void setModal(bool modal);
	virtual bool isModal() const;

	virtual void setId(int id);
	virtual int getId() const;

	virtual void setVisible(bool visible);

	virtual void addWindowListener(EventListener* listener);
	virtual void removeWindowListener(EventListener* listener);
	virtual listener_iterator getWindowListeners() const;

	static void quit(int code);
};

class WindowUIController : public ContainerUIController {
public:
	WindowUIController();
	virtual ~WindowUIController();

	virtual bool handleEvent(Component* component, Event* event);
};


} // ui

} // utils


#endif /* _COMP_WINDOW_HPP_ */
