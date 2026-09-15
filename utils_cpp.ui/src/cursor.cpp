/*
 * cursor.cpp
 *
 *  Created on: 05/08/2014
 *      Author: Alan
 */

#include <utils/ui/cursor.hpp>
#include <utils/ui/image.hpp>
#include <utils/ui/ui.hpp>

namespace utils {

namespace ui {

INIT_CONSTANT(SystemCursor,APPSTARTING,1);
INIT_CONSTANT(SystemCursor,ARROW,0);
INIT_CONSTANT(SystemCursor,CROSS,2);
INIT_CONSTANT(SystemCursor,HAND,3);
INIT_CONSTANT(SystemCursor,HELP,4);
INIT_CONSTANT(SystemCursor,IBEAM,5);
INIT_CONSTANT(SystemCursor,NO,6);
INIT_CONSTANT(SystemCursor,SIZEALL,7);
INIT_CONSTANT(SystemCursor,SIZENESW,8);
INIT_CONSTANT(SystemCursor,SIZENS,9);
INIT_CONSTANT(SystemCursor,SIZENWSE,10);
INIT_CONSTANT(SystemCursor,SIZEWE,11);
INIT_CONSTANT(SystemCursor,UPARROW,12);
INIT_CONSTANT(SystemCursor,WAIT,13);

void Cursor::deleteCursor(Cursor* _cursor) {
	delete _cursor;
}

Cursor::Cursor() :
		internal_data(NULL),
		system(false),
		xHotspot(0),
		yHotspot(0),
		imgMask(NULL),
		imgColor(NULL) {
}
Cursor::Cursor(void* _data, bool _system, int _xHotspot, int _yHotspot, Image* _imgMask, Image* _imgColor) :
		internal_data(_data),
		system(_system),
		xHotspot(_xHotspot),
		yHotspot(_yHotspot),
		imgMask(_imgMask),
		imgColor(_imgColor) {
}

Cursor::Cursor(void* _data, bool _system) :
		internal_data(_data),
		system(_system),
		xHotspot(0),
		yHotspot(0),
		imgMask(NULL),
		imgColor(NULL) {
	load_cursor_properties();
}

Cursor::Cursor(int _xHotspot, int _yHotspot, Image* _imgMask, Image* _imgColor) :
		internal_data(NULL),
		system(false),
		xHotspot(_xHotspot),
		yHotspot(_yHotspot),
		imgMask(_imgMask),
		imgColor(_imgColor) {
}

Cursor::~Cursor() {
	// imgMask and imgColor release themselves.
	free_cursor_resource();
}

Size Cursor::getSize() const {
	return Size();
}

Point Cursor::getHotSpot() const {
	return Point(xHotspot,yHotspot);
}

Image* Cursor::getColorImage() {
	if (imgColor == NULL)
		load_cursor_images();
	return imgColor;
}

Image* Cursor::getMaskImage() {
	if (imgMask == NULL)
		load_cursor_images();
	return imgMask;
}

bool Cursor::isSystemCursor() const {
	return system;
}

}

}
