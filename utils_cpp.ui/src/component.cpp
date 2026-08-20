/*
 * component.cpp
 *
 *  Created on: 23/04/2015
 *      Author: ALAN
 */

#include <utils/ui/event.hpp>
#include <utils/ui/component.hpp>
#include <utils/ui/container.hpp>
#include <typeinfo>
#include <string>
#include <unordered_map>

namespace utils {

namespace ui {

static UIController ui_component_controller;

const char* Component::getClassName() const {
	// typeid(*this), not typeid(this): the latter is the type of the POINTER,
	// resolved statically, so every subclass returned the same name and the
	// whole hierarchy collapsed onto a single shared UIController.
	return typeid(*this).name();
}

UIController* Component::getUIController() {
	if (_ui_controller)
		return _ui_controller;
	_ui_controller = UIController::getUIController(this);
	if (_ui_controller == NULL) {
		_ui_controller = getDefaultUIController();
		UIController::registerUIController(_ui_controller,this->getClassName());
	}
	return _ui_controller;
}

UIController* Component::getDefaultUIController() const {
	return &ui_component_controller;
}

void Component::setUIController(UIController* uiController) {
	_ui_controller = uiController;
}


Size Component::getMaximumSize() const {
	return _MaximumSize;
}
Size Component::getMinimumSize() const {
	return _MinimumSize;
}
Size Component::getDefaultSize() const {
	return _DefaultSize;
}
Rect Component::getBorderRect() const {
	return _BorderRect;
}

void Component::setMaximumSize(const Size& v) {
	_MaximumSize = v;
}
void Component::setMinimumSize(const Size& v) {
	_MinimumSize = v;
}
void Component::setDefaultSize(const Size& v) {
	_DefaultSize = v;
}
void Component::setBorderRect(const Rect& v) {
	_BorderRect = v;
}

void Component::addMouseListener(EventListener* listener) {
	_mouse_listeners.push_front(listener);
}
void Component::removeMouseListener(EventListener* listener) {
	_mouse_listeners.remove(listener);
}

void Component::addMouseWheelListener(EventListener* listener) {
	_mousewheel_listeners.push_front(listener);
}
void Component::removeMouseWheelListener(EventListener* listener) {
	_mousewheel_listeners.remove(listener);
}

void Component::addKeyListener(EventListener* listener) {
	_key_listeners.push_front(listener);
}
void Component::removeKeyListener(EventListener* listener) {
	_key_listeners.remove(listener);
}

void Component::addFocusListener(EventListener* listener) {
	_focus_listeners.push_front(listener);
}
void Component::removeFocusListener(EventListener* listener) {
	_focus_listeners.remove(listener);
}

void Component::addComponentListener(EventListener* listener) {
	_component_listeners.push_front(listener);
}
void Component::removeComponentListener(EventListener* listener) {
	_component_listeners.remove(listener);
}

void Component::addResizingListener(EventListener* listener) {
	_resizing_listeners.push_front(listener);
}
void Component::removeResizingListener(EventListener* listener) {
	_resizing_listeners.remove(listener);
}

void Component::addMovingListener(EventListener* listener) {
	_moving_listeners.push_front(listener);
}
void Component::removeMovingListener(EventListener* listener) {
	_moving_listeners.remove(listener);
}

void Component::addPaintListener(EventListener* listener) {
	_paint_listeners.push_front(listener);
}
void Component::removePaintListener(EventListener* listener) {
	_paint_listeners.remove(listener);
}

void Component::addCommandListener(EventListener* listener) {
	_command_listeners.push_front(listener);
}
void Component::removeCommandListener(EventListener* listener) {
	_command_listeners.remove(listener);
}

Component::listener_iterator Component::getMouseListeners() {
	return _mouse_listeners.cbegin();
}

Component::listener_iterator Component::getMouseWheelListeners() {
	return _mousewheel_listeners.cbegin();
}

Component::listener_iterator Component::getKeyListeners() {
	return _key_listeners.cbegin();
}

Component::listener_iterator Component::getFocusListeners() {
	return _focus_listeners.cbegin();
}

Component::listener_iterator Component::getComponentListeners() {
	return _component_listeners.cbegin();
}

Component::listener_iterator Component::getResizingListeners() {
	return _resizing_listeners.cbegin();
}

Component::listener_iterator Component::getMovingListeners() {
	return _moving_listeners.cbegin();
}

Component::listener_iterator Component::getPaintListeners() {
	return _paint_listeners.cbegin();
}

Component::listener_iterator Component::getCommandListeners() {
	return _command_listeners.cbegin();
}

void Component::callListeners(listener_list& list, Event* event) {
	// Standard iterators have no conversion to bool; compare against end().
	for (listener_list::iterator it = list.begin(); it != list.end(); ++it) {
		(*it)->operator ()(event);
	}
}

//--------------------------------------------------------------------------------------------------------------------------

UIController::UIController() {

}
UIController::~UIController() {

}

bool UIController::handleEvent(Component* component, Event* event) {
	int hiid = event->id() & 0xffff0000;
	if (hiid == 0x00010000) {
		if (event->id() == MouseWheelEvent::MOUSE_WHEEL)
			component->callListeners(component->_mousewheel_listeners,event);
		else
			component->callListeners(component->_mouse_listeners,event);
		if (event->id() == MouseWheelEvent::CLICK && !event->defaultCanceled()) {
			CommandEvent cmdevt = CommandEvent(component,event->time(), CommandEvent::COMMAND, component->getId());
			component->callListeners(component->_command_listeners,&cmdevt);
		}
	} else 	if (hiid == 0x00020000) {
		if (event->id() == FocusEvent::GOT_FOCUS || event->id() == FocusEvent::LOST_FOCUS)
			component->callListeners(component->_focus_listeners,event);
		else
			component->callListeners(component->_key_listeners,event);
	} else 	if (hiid == 0x00050000) {
		if (event->id() == ComponentEvent::RESIZING) {
			component->callListeners(component->_resizing_listeners,event);
			if (!event->defaultCanceled()) {
				ResizingEvent* evt = dynamic_cast<ResizingEvent*>(event);
				int w = evt->getRect()->right - evt->getRect()->left + 1;
				int h = evt->getRect()->bottom - evt->getRect()->top + 1;
				int dw = 0, dh = 0;
				Size maxSz = component->getMaximumSize();
				Size minSz = component->getMinimumSize();
				if (minSz.width > 0 && w < minSz.width)
					dw = minSz.width - w;
				if (maxSz.width > 0 && w > maxSz.width)
					dw = maxSz.width - w;
				if (minSz.height > 0 && h < minSz.height)
					dh = minSz.height - h;
				if (maxSz.height > 0 && h > maxSz.height)
					dh = maxSz.height - h;
				if (evt->getResizingDir() == ResizingEvent::DIR_LEFT ||
					evt->getResizingDir() == ResizingEvent::DIR_TOPLEFT ||
					evt->getResizingDir() == ResizingEvent::DIR_BOTTOMLEFT)
					evt->getRect()->left -= dw;
				if (evt->getResizingDir() == ResizingEvent::DIR_RIGHT ||
					evt->getResizingDir() == ResizingEvent::DIR_TOPRIGHT ||
					evt->getResizingDir() == ResizingEvent::DIR_BOTTOMRIGHT)
					evt->getRect()->right += dw;
				if (evt->getResizingDir() == ResizingEvent::DIR_TOP ||
					evt->getResizingDir() == ResizingEvent::DIR_TOPRIGHT ||
					evt->getResizingDir() == ResizingEvent::DIR_TOPLEFT)
					evt->getRect()->top -= dh;
				if (evt->getResizingDir() == ResizingEvent::DIR_BOTTOM ||
					evt->getResizingDir() == ResizingEvent::DIR_BOTTOMLEFT ||
					evt->getResizingDir() == ResizingEvent::DIR_BOTTOMRIGHT)
					evt->getRect()->bottom += dh;
			}
		} else if (event->id() == ComponentEvent::MOVING) {
			component->callListeners(component->_moving_listeners,event);
		} else if (event->id() == ComponentEvent::PAINT) {
			paint(component,dynamic_cast<PaintEvent*>(event)->graphics());
			component->callListeners(component->_paint_listeners,event);
		} else if (event->id() == ComponentEvent::ERASE_BACKGROUND) {
			if (eraseBackground(component,dynamic_cast<PaintEvent*>(event)->graphics()))
				component->callListeners(component->_paint_listeners,event);
			else
				return false;
		} else
			component->callListeners(component->_component_listeners,event);
	}
	return true;
}

void UIController::paint(Component* component, Graphics* graphics) {
	component->paint(graphics);
}

bool UIController::eraseBackground(Component* component, Graphics* graphics) {
	FillStyle* bkg = component->getBackground();
	if (bkg) {
		graphics->setFillStyle(bkg);
		Size sz = component->getSize();
		graphics->fillRect(0,0,sz.width,sz.height);
	}
	return true;
}

bool UIController::defaultPaint(Component* component) {
	return false;
}

typedef std::unordered_map<std::string, UIController*> uicontroller_map_type;

// A function-local static, for the same reason as the UI allocator: a
// namespace-scope map is dynamically initialised and could be read by another
// translation unit's static initialiser before its constructor had run.
static uicontroller_map_type& uicontroller_map() {
	static uicontroller_map_type map;
	return map;
}

UIController* UIController::getUIController(Component* component) {
	uicontroller_map_type& map = uicontroller_map();
	uicontroller_map_type::iterator it = map.find(component->getClassName());
	if (it != map.end())
		return it->second;
	return NULL;
}
bool UIController::registerUIController(UIController* controller, const char* className) {
	uicontroller_map_type& map = uicontroller_map();
	uicontroller_map_type::iterator it = map.find(className);
	if (it != map.end())
		return false;
	map.insert(uicontroller_map_type::value_type(className,controller));
	return true;
}



}

}

