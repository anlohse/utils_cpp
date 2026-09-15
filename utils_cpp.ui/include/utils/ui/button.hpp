/*
 * button.hpp
 *
 *  Created on: 03/05/2015
 *      Author: ALAN
 */

#ifndef BUTTON_HPP_
#define BUTTON_HPP_

#include <utils/ui/container.hpp>
#include <utils/ui/event.hpp>
#include <utils/ui/image.hpp>
#include <utils/ui/ref.hpp>

namespace utils {

namespace ui {


class Button : public Component {
private:
	friend class ButtonUIController;
	Ref<Image> _image;
	Ref<Image> _disabled_image;
	Ref<Image> _pressed_image;
protected:
	virtual const char* getClassName() const;
	virtual UIController* getDefaultUIController() const;

	void createButton(Container* parent, int left, int top, int width, int height);

public:
	Button();
	Button(Container* parent);
	Button(Container* parent, int left, int top, int width, int height, int id);
	virtual ~Button();

	virtual void click();

	virtual Image* getImage() const;
	virtual Image* getDisabledImage() const;
	virtual Image* getPressedImage() const;

	virtual Image* setImage(Image* img);
	virtual Image* setDisabledImage(Image* img);
	virtual Image* setPressedImage(Image* img);
};


class ButtonUIController : public UIController {
public:
	ButtonUIController();
	virtual ~ButtonUIController();

	virtual bool handleEvent(Component* component, Event* event);

	virtual void paint(Component* component, Graphics* graphics);

	virtual bool eraseBackground(Component* component, Graphics* graphics);

	virtual bool defaultPaint(Component* component);

};

}  // namespace ui

}  // namespace utils



#endif /* BUTTON_HPP_ */
