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
			Component(), _image(NULL), _disabled_image(NULL), _pressed_image(NULL) {
	createButton(parent,left,top,width,height);
	setId(idCommand);
}

Button::~Button() {
	// All three images are reference-counted, not just the first.
	if (_image && _image->rem_reference()->get_references() < 1) delete _image;
	if (_disabled_image && _disabled_image->rem_reference()->get_references() < 1) delete _disabled_image;
	if (_pressed_image && _pressed_image->rem_reference()->get_references() < 1) delete _pressed_image;
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

Image* Button::setImage(Image* img) {
	Image* old = _image;
	if (img) img->add_reference();
	_image = img;
	if (old) old->rem_reference();
	return old;
}
Image* Button::setDisabledImage(Image* img) {
	Image* old = _disabled_image;
	if (img) img->add_reference();
	_disabled_image = img;
	if (old) old->rem_reference();
	return old;
}
Image* Button::setPressedImage(Image* img) {
	Image* old = _pressed_image;
	if (img) img->add_reference();
	_pressed_image = img;
	if (old) old->rem_reference();
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
	return btn->_image == NULL &&
			btn->_disabled_image == NULL &&
			btn->_pressed_image == NULL;
//	return true;
}


} // ui

} // utils

