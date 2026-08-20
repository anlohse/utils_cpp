/*
 * container.cpp
 *
 *  Created on: 01/05/2015
 *      Author: ALAN
 */

#include <utils/ui/container.hpp>
#include <utils/ui/event.hpp>

namespace utils {

namespace ui {

//	component_list _children;
Container::Container() : Component(), _children(), _layout() {
}
Container::Container(int left, int top, int width, int height) :
		Component(NULL,left, top, width, height), _children(), _layout() {
}

Container::~Container() {
	// Standard iterators have no conversion to bool; compare against end().
	for (component_list::iterator it = _children.begin(); it != _children.end(); ++it) {
		delete it->child;
	}
	SAFE_DELETE(_layout);
}

static ContainerUIController containerUIController;

UIController* Container::getDefaultUIController() const {
	return &containerUIController;
}

void Container::addChild(Component* child) {
	addChild(child,0);
}
bool Container::addChild(Component* child, int constraint) {
	if (child->getParent()) return false;
	if (_layout)
		if (!_layout->checkConstraints(this,child, constraint))
			return false;
	child->add_reference();
	_children.push_back(container_child(child,constraint));
	child->setParent(this);
	return true;
}
void Container::removeChild(Component* child) {
	if (child->getParent() == this) {
		child->setParent(NULL);
		_children.remove(container_child(child,0));
	}
}
Container::component_iterator Container::children() const {
	return _children.cbegin();
}
Container::component_iterator Container::children_end() const {
	return _children.cend();
}
void Container::setLayout(Layout* layout) {
	_layout = layout;
}
Layout* Container::getLayout() const {
	return _layout;
}
void Container::layout() {
	if (_layout)
		_layout->apply(this);
}
Size Container::getMinimumSize() const {
	Size sz = Component::getMinimumSize();
	if (_layout) {
		Size sz2 = _layout->getMinimumSize(this);
		if (sz2.width > sz.width)
			sz.width = sz2.width;
		if (sz2.height > sz.height)
			sz.height = sz2.height;
	}
	return sz;
}

//--------------------------------------------------------------------------------------------------------------------------

ContainerUIController::ContainerUIController() {
}
ContainerUIController::~ContainerUIController() {
}

bool ContainerUIController::handleEvent(Component* component, Event* event) {
	if (event->id() == ComponentEvent::RESIZE) {
		Container* cont = dynamic_cast<Container*>(component);
		if (cont && cont->_layout)
			cont->_layout->apply(cont);
	}
	return UIController::handleEvent(component, event);
}

void ContainerUIController::paint(Component* component, Graphics* graphics) {
	UIController::paint(component, graphics);
}

//--------------------------------------------------------------------------------------------------------------------------

bool Layout::checkConstraints(const Container* container, const Component* child, int constraint) {
	Container::component_iterator end = container->children_end();
	for (Container::component_iterator it = container->children(); it != end; ++it) {
		if (it->constraint == constraint)
			return false;
	}
	return true;
}

void BasicLayout::apply(Container* container) {
	// TODO
}

Size BasicLayout::getMinimumSize(const Container* container) const {
	int width = 0, height = 0, tb = 0, lr = 0;
	Container::component_iterator end = container->children_end();
	for (Container::component_iterator it = container->children(); it != end; ++it) {
		Size msz = it->child->getMinimumSize();
		switch (it->constraint) {
		case BasicLayout::TOP:
		case BasicLayout::BOTTOM:
			if (tb < msz.width)
				tb = msz.width;
			height += msz.height + _vpadding;
			break;
		case BasicLayout::LEFT:
		case BasicLayout::RIGHT:
			if (lr < msz.height)
				lr = msz.height;
			width += msz.width + _hpadding;
			break;
		case BasicLayout::CENTER:
			width += msz.width;
			height += msz.height;
			break;
		}
	}
	if (tb > width) width = tb;
	if (lr > height) height = lr;
	return Size(width,height);
}

//--------------------------------------------------------------------------------------------------------------------------

} // ui

} // utils


