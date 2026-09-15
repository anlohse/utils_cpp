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

/**
 * The size a child would like to occupy along its band.
 *
 * There is no getPreferredSize() in the Component interface, so the default
 * size stands in for it and the minimum size fills in whichever axis was left
 * unset. The result is clamped to the maximum size when one is given.
 */
static Size preferredSize(const Component* child) {
	Size pref = child->getDefaultSize();
	const Size min = child->getMinimumSize();
	if (pref.width <= 0)  pref.width  = min.width;
	if (pref.height <= 0) pref.height = min.height;

	const Size max = child->getMaximumSize();
	if (max.width > 0 && pref.width > max.width)    pref.width  = max.width;
	if (max.height > 0 && pref.height > max.height) pref.height = max.height;

	if (pref.width < 0)  pref.width = 0;
	if (pref.height < 0) pref.height = 0;
	return pref;
}

void BasicLayout::apply(Container* container) {
	if (container == NULL)
		return;

	// One child per region, which is what Layout::checkConstraints enforces.
	Component* north  = NULL;
	Component* south  = NULL;
	Component* west   = NULL;
	Component* east   = NULL;
	Component* centre = NULL;

	Container::component_iterator end = container->children_end();
	for (Container::component_iterator it = container->children(); it != end; ++it) {
		Component* child = it->child;
		if (child == NULL)
			continue;
		switch (it->constraint) {
			case BasicLayout::TOP:    north  = child; break;
			case BasicLayout::BOTTOM: south  = child; break;
			case BasicLayout::LEFT:   west   = child; break;
			case BasicLayout::RIGHT:  east   = child; break;
			case BasicLayout::CENTER: centre = child; break;
			default: break;
		}
	}

	// Work in the container's client area, inset by its border rectangle.
	const Size client = container->getSize();
	const Rect border = container->getBorderRect();
	int left   = border.left;
	int top    = border.top;
	int right  = client.width  - border.right;
	int bottom = client.height - border.bottom;
	if (right < left)   right = left;
	if (bottom < top)   bottom = top;

	// Border-layout order: the top and bottom bands span the full width, the
	// left and right bands take the height that remains, and the centre gets
	// whatever is left over.
	if (north != NULL) {
		int h = preferredSize(north).height;
		if (h > bottom - top) h = bottom - top;
		north->setRect(Rect(left, top, right, top + h));
		top += h + _vpadding;
		if (top > bottom) top = bottom;
	}
	if (south != NULL) {
		int h = preferredSize(south).height;
		if (h > bottom - top) h = bottom - top;
		south->setRect(Rect(left, bottom - h, right, bottom));
		bottom -= h + _vpadding;
		if (bottom < top) bottom = top;
	}
	if (west != NULL) {
		int w = preferredSize(west).width;
		if (w > right - left) w = right - left;
		west->setRect(Rect(left, top, left + w, bottom));
		left += w + _hpadding;
		if (left > right) left = right;
	}
	if (east != NULL) {
		int w = preferredSize(east).width;
		if (w > right - left) w = right - left;
		east->setRect(Rect(right - w, top, right, bottom));
		right -= w + _hpadding;
		if (right < left) right = left;
	}
	if (centre != NULL) {
		centre->setRect(Rect(left, top, right, bottom));
	}

	// Lay out nested containers explicitly. Resizing a child normally raises
	// a resize event that re-runs its own layout, but only when the size
	// actually changes -- a child that already had the right size would
	// otherwise never arrange its own children.
	for (Container::component_iterator it = container->children(); it != end; ++it) {
		Container* nested = dynamic_cast<Container*>(it->child);
		if (nested != NULL)
			nested->layout();
	}
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


