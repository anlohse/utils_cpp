/*
 * window.cpp
 *
 *  Created on: 01/05/2015
 *      Author: ALAN
 */

#include <utils/ui/window.hpp>
#include <utils/ui/event.hpp>

namespace utils {

namespace ui{

INIT_CONSTANT(WindowType,WINDOW,WindowType(0));
INIT_CONSTANT(WindowType,POPUP,WindowType(1));
INIT_CONSTANT(WindowType,TOOL,WindowType(2));
INIT_CONSTANT(WindowType,DIALOG,WindowType(3));

INIT_CONSTANT(CloseAction,DO_NOTHING,CloseAction(0));
INIT_CONSTANT(CloseAction,HIDE,CloseAction(1));
INIT_CONSTANT(CloseAction,DESTROY,CloseAction(2));
INIT_CONSTANT(CloseAction,QUIT,CloseAction(3));

static WindowUIController windowUIController;

Window::Window(): _modal(false),_notopen(true), _defCloseAction(CloseAction::DO_NOTHING), _icon(), _smallIcon(), _window_listeners(), _command_listeners() {

}
Window::~Window() {
	// _icon and _smallIcon release themselves.
}

UIController* Window::getDefaultUIController() const {
	return &windowUIController;
}

void Window::setCloseAction(const CloseAction& ca) {
	_defCloseAction = ca;
}
CloseAction Window::getCloseAction() const {
	return _defCloseAction;
}
void Window::setId(int id) {
}
int Window::getId() const {
	return 0;
}

void Window::addWindowListener(EventListener* listener) {
	_window_listeners.push_back(listener);
}
void Window::removeWindowListener(EventListener* listener) {
	_window_listeners.remove(listener);
}
Window::listener_iterator Window::getWindowListeners() const {
	return _window_listeners.cbegin();
}

void Window::setModal(bool modal) {
	_modal = modal;
}
bool Window::isModal() const {
	return _modal;
}

WindowUIController::WindowUIController() {
}
WindowUIController::~WindowUIController() {
}

bool WindowUIController::handleEvent(Component* component, Event* event) {
	int hiid = event->id() & 0xffff0000;
	if (hiid == 0x00060000) {
		Window* wnd = dynamic_cast<Window*>(component);
		if (wnd)
			wnd->callListeners(wnd->_window_listeners, event);
		else
			return false;
		return true;
	}
	return ContainerUIController::handleEvent(component, event);
}


} // ui

} // utils


