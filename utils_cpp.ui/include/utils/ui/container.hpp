/*
 * container.hpp
 *
 *  Created on: 25/04/2015
 *      Author: Alan
 */

#ifndef CONTAINER_HPP_
#define CONTAINER_HPP_

#include <utils/ui/component.hpp>
#include <utils/ui/layout.hpp>

namespace utils {

namespace ui {

struct container_child {
	Component* child;
	int constraint;
	container_child(Component* _child, int _constr) : child(_child), constraint(_constr) { }
	container_child() : child(), constraint() { }
	operator Component*() const {
		return child;
	}
	bool operator == (const container_child& other) const {
		return child == other.child;
	}
};

class Container : public Component {
public:
	typedef std::list<container_child, ui_allocator<container_child> > component_list;
	typedef component_list::const_iterator component_iterator;
private:
	friend class ContainerUIController;
	component_list _children;
	Layout*  _layout;
protected:
	virtual UIController* getDefaultUIController() const;

public:
	Container();
	Container(int left, int top, int width, int height);

	virtual ~Container();

	virtual void addChild(Component* child);
	virtual bool addChild(Component* child, int constraint);
	virtual void removeChild(Component* child);
	// children() alone cannot be iterated safely -- there was no way to know
	// where the sequence ends. The old fwdlist iterator converted to bool,
	// which standard iterators do not.
	virtual component_iterator children() const;
	virtual component_iterator children_end() const;

	virtual void setLayout(Layout* layout);
	virtual Layout* getLayout() const;
	virtual void layout();
	virtual Size getMinimumSize() const;

};

class ContainerUIController : public UIController {
public:
	ContainerUIController();
	virtual ~ContainerUIController();

	virtual bool handleEvent(Component* component, Event* event);

	virtual void paint(Component* component, Graphics* graphics);

};


} // ui

} // utils

#endif /* CONTAINER_HPP_ */
