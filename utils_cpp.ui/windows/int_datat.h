/*
 * int_datat.h
 *
 *  Created on: 05/08/2014
 *      Author: Alan
 *
 * Declarations shared between the Win32 backend translation units.
 */

#ifndef INT_DATAT_H_
#define INT_DATAT_H_

#include <utils/ui/ui.hpp>
#include <utils/ui/graphics.hpp>
#include <windows.h>

namespace utils {

namespace ui {

/**
 * Builds a Graphics for the backend currently selected by
 * Graphics::setGraphicsImplementation.
 *
 * Every site that needs a drawing surface goes through here, so adding a
 * backend does not mean hunting down open-coded `new GdiGraphics` calls --
 * which is how GDI+ ended up unreachable despite GRAPHICS_GDIPLUS existing.
 *
 * @param hdc     the device context to draw into
 * @param ownsDC  true when the returned Graphics should delete hdc
 * @return a new Graphics, or NULL if the selected backend is unavailable.
 *         The caller owns it and must pass it to destroyBackendGraphics.
 */
Graphics* createBackendGraphics(HDC hdc, bool ownsDC);

/** Destroys a Graphics obtained from createBackendGraphics. */
void destroyBackendGraphics(Graphics* graphics);

}

}

#endif /* INT_DATAT_H_ */
