/*
 * button.cpp
 *
 *  Created on: 23/01/2016
 *      Author: Alan
 */


#include <utils/ui/button.hpp>

namespace utils {

namespace ui {

Button::Button(Container* parent, int left, int top, int width, int height, int idCommand) :
			Component(), _image(), _disabled_image(), _pressed_image() {
	createButton(parent,left,top,width,height);
	setId(idCommand);
}

Button::~Button() {
	// The three Ref members release themselves. Two of them used to be left
	// uninitialised entirely, so defaultPaint read indeterminate pointers.
}

Image* Button::getImage() const {
	return _image;
}
Image* Button::getDisabledImage() const {
	return _disabled_image;
}
Image* Button::getPressedImage() const {
	return _pressed_image;
}

// The setters return the previous image. They hand back a raw pointer for
// source compatibility, and keep a reference on it alive for the caller by
// detaching rather than releasing -- so the returned object is still valid
// and the caller now owns that reference.
Image* Button::setImage(Image* img) {
	Image* old = _image.detach();
	_image = img;
	return old;
}
Image* Button::setDisabledImage(Image* img) {
	Image* old = _disabled_image.detach();
	_disabled_image = img;
	return old;
}
Image* Button::setPressedImage(Image* img) {
	Image* old = _pressed_image.detach();
	_pressed_image = img;
	return old;
}

ButtonUIController buttonUIController;

UIController* Button::getDefaultUIController() const {
	return &buttonUIController;
}

ButtonUIController::ButtonUIController() {
}
ButtonUIController::~ButtonUIController() {
}

bool ButtonUIController::handleEvent(Component* component, Event* event) {
	bool ret = UIController::handleEvent(component,event);
	return event->defaultCanceled() || !ret;
}

bool ButtonUIController::eraseBackground(Component* component, Graphics* graphics) {
	return false;
}

bool ButtonUIController::defaultPaint(Component* component) {
	Button* btn = dynamic_cast<Button*>(component);
	if (btn == NULL)
		return true;
	// No images means the native control can paint itself.
	return !btn->_image && !btn->_disabled_image && !btn->_pressed_image;
//	return true;
}


} // ui

} // utils

