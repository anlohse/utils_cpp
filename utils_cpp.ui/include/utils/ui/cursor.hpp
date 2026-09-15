/*
 * cursor.hpp
 *
 *  Created on: 03/08/2014
 *      Author: Alan
 */

#ifndef CURSOR_HPP_
#define CURSOR_HPP_

#include <utils/ui/graphics.hpp>
#include <utils/ui/ui_allocator.hpp>
#include <utils/ui/image.hpp>
#include <utils/ui/ref.hpp>

namespace utils {

namespace ui {

class SystemCursor {
private:
	friend class Cursor;
	MAKE_ENUMERATION(SystemCursor)
	DECLARE_CONSTANT(SystemCursor,APPSTARTING);
	DECLARE_CONSTANT(SystemCursor,ARROW);
	DECLARE_CONSTANT(SystemCursor,CROSS);
	DECLARE_CONSTANT(SystemCursor,HAND);
	DECLARE_CONSTANT(SystemCursor,HELP);
	DECLARE_CONSTANT(SystemCursor,IBEAM);
	DECLARE_CONSTANT(SystemCursor,NO);
	DECLARE_CONSTANT(SystemCursor,SIZEALL);
	DECLARE_CONSTANT(SystemCursor,SIZENESW);
	DECLARE_CONSTANT(SystemCursor,SIZENS);
	DECLARE_CONSTANT(SystemCursor,SIZENWSE);
	DECLARE_CONSTANT(SystemCursor,SIZEWE);
	DECLARE_CONSTANT(SystemCursor,UPARROW);
	DECLARE_CONSTANT(SystemCursor,WAIT);
};

class Cursor : public UIObject {
private:
	friend class Component;
	void* internal_data;
	bool system;
	int xHotspot;
	int yHotspot;
	Ref<Image> imgMask;
	Ref<Image> imgColor;
protected:
	Cursor();
	Cursor(void* _data);
	Cursor(void* _data, bool _system);
	Cursor(void* _data, bool _system, int _xHotspot, int _yHotspot, Image* _imgMask, Image* _imgColor);
	Cursor(int _xHotspot, int _yHotspot, Image* _imgMask, Image* _imgColor);
	void free_cursor_resource();
	void load_cursor_properties();
	void load_cursor_images();
public:
	virtual ~Cursor();

	virtual Size getSize() const;

	virtual Point getHotSpot() const;

	virtual Image* getColorImage();

	virtual Image* getMaskImage();

	virtual bool isSystemCursor() const;

public:
	static Cursor* getSystemCursor(const SystemCursor& cursorId);
	static Cursor* loadCursor(const char* filename);
	static Cursor* createCursor(int _xHotspot, int _yHotspot, Image* _imgMask, Image* _imgColor);
	static void deleteCursor(Cursor* _cursor);
};

} // ui

} // utils

#endif /* CURSOR_HPP_ */
