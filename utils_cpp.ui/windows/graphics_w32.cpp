/*
 * graphics_w32.cpp
 *
 *  Created on: 07/08/2014
 *      Author: Alan
 */

#include <utils/ui/ui.hpp>
#include <utils/ui/image.hpp>
#include <windows.h>
#include "gdi_graphics.h"
#include "gdiplus_graphics.h"
#include "int_datat.h"

namespace utils {

namespace ui {

extern int graphics_impl;

Graphics* createBackendGraphics(HDC hdc, bool ownsDC) {
	if (hdc == NULL)
		return NULL;
	if (graphics_impl == GRAPHICS_GDIPLUS)
		return GdiPlusGraphics::createGraphics(hdc, ownsDC);
	return GdiGraphics::createGraphics(hdc, ownsDC);
}

void destroyBackendGraphics(Graphics* graphics) {
	// Both backends have virtual destructors, so a plain delete dispatches
	// correctly whichever one produced it.
	delete graphics;
}

Font* Font::createFont(const char* facename, int style, int size) {
	if (graphics_impl == GRAPHICS_GDIPLUS)
		return new GdiPlusFont(facename, style, size);

	return GdiFont::createFontImpl(facename, style, size);
}

void Font::destroyFont(Font* obj) {
	if (obj == NULL)
		return;
	// A GDI+ font owns no HGDIOBJ, so it is simply deleted; the GDI one has
	// a handle to release first.
	GdiFont* gdi = dynamic_cast<GdiFont*>(obj);
	if (gdi != NULL) {
		GdiFont::destroyFontImpl(gdi);
		return;
	}
	delete obj;
}

}

}
