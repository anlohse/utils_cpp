/*
 * cursor.cpp
 *
 *  Created on: 04/08/2014
 *      Author: Alan
 */

#include <utils/ui/cursor.hpp>
#include <utils/ui/ui.hpp>
#include <utils/ui/image.hpp>
#include <windows.h>
#include "int_datat.h"
#include "image_w32.h"

namespace utils {

namespace ui {

void Cursor::free_cursor_resource() {
	if (!system)
		DeleteObject((HCURSOR)internal_data);
}
void Cursor::load_cursor_properties() {
	 ICONINFO info = {0};
	 if (::GetIconInfo((HCURSOR)internal_data, &info)!=0 ) {
		 xHotspot = info.xHotspot;
		 yHotspot = info.yHotspot;
	 }
}
void Cursor::load_cursor_images() {
	 ICONINFO info = {0};
	 if (::GetIconInfo((HCURSOR)internal_data, &info)!=0 ) {
		 imgColor = Image_W32::createImage(info.hbmColor,false);
		 imgMask = Image_W32::createImage(info.hbmMask,false);
	 }
}

#ifdef UNICODE
typedef const wchar_t* t_resource;
#else
typedef const char* t_resource;
#endif

static t_resource SYSTEM_CURSORS[14] = {
	IDC_ARROW,
	IDC_APPSTARTING,
	IDC_CROSS,
	IDC_HAND,
	IDC_HELP,
	IDC_IBEAM,
	IDC_NO,
	IDC_SIZEALL,
	IDC_SIZENESW,
	IDC_SIZENS,
	IDC_SIZENWSE,
	IDC_SIZEWE,
	IDC_UPARROW,
	IDC_WAIT
};

Cursor* Cursor::getSystemCursor(const SystemCursor& cursorId) {
	HCURSOR hCursor = LoadCursor(NULL,SYSTEM_CURSORS[cursorId.value]);
	return new Cursor((void*)hCursor,true);
}

Cursor* Cursor::loadCursor(const char* filename) {
	HCURSOR hCursor = LoadCursorFromFileA(filename);
	return new Cursor((void*)hCursor,false);
}

Cursor* Cursor::createCursor(int _xHotspot, int _yHotspot, Image* _imgMask, Image* _imgColor) {
	ICONINFO iconinfo       = {0};
	iconinfo.fIcon          = FALSE;
	iconinfo.xHotspot       = _xHotspot;
	iconinfo.yHotspot       = _yHotspot;
	iconinfo.hbmMask        = dynamic_cast<Image_W32*>(_imgMask)->hBitmap;
	iconinfo.hbmColor       = dynamic_cast<Image_W32*>(_imgColor)->hBitmap;
	HCURSOR hCursor = ::CreateIconIndirect(&iconinfo);
	// The Cursor's Ref members take their own references.
	return new Cursor(hCursor,false,_xHotspot,_yHotspot,_imgMask,_imgColor);
}

} // ui

} // utils

