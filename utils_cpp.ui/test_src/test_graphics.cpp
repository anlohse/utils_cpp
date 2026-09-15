/*
 * test_graphics.cpp
 *
 * Renders one reference scene through each backend and writes the results out
 * as bitmaps, so the antialiasing can be compared side by side without a
 * window or a human watching.
 *
 * Run the utils_ui_tests executable; it drops gdi.bmp and gdiplus.bmp in the
 * working directory and reports the measured edge quality of each.
 */

#include <utils/ui/graphics.hpp>
#include <utils/ui/image.hpp>
#include <utils/ui/ui_defs.hpp>
#include <windows.h>
#include "int_datat.h"   // createBackendGraphics: internal to the Win32 backend
#include <cstdio>
#include <iostream>
#include <vector>

using namespace utils;
using namespace utils::ui;

namespace {

const int SCENE_W = 320;
const int SCENE_H = 240;

/**
 * The reference scene: shapes whose edges are all off the pixel grid, so an
 * antialiasing backend and a hard-edged one produce visibly different output.
 */
void drawScene(Graphics* g) {
	g->setFillColor(Color::WHITE);
	g->fillRect(0, 0, (float) SCENE_W, (float) SCENE_H);

	// A diagonal stroke -- the classic staircase test.
	g->setLineColor(Color::RED);
	g->setStroke(g->createStroke(LineStyle::SOLID, LineJoin::MITER, LineCap::ROUND, 3));
	g->beginPath();
	g->moveTo(20.0f, 20.0f);
	g->lineTo(140.0f, 95.0f);
	g->stroke();

	// A bezier, where curvature makes aliasing obvious.
	g->setLineColor(Color::BLUE);
	g->beginPath();
	g->moveTo(20.0f, 120.0f);
	g->bezierCurveTo(60.0f, 40.0f, 120.0f, 200.0f, 160.0f, 120.0f);
	g->stroke();

	// A filled circle: the edge is curved everywhere. LIME, not GREEN --
	// every colour in this scene must have channels that are only 0 or 255,
	// or countIntermediateTones below cannot tell a solid pixel from a
	// blended one (Color::GREEN is 0xff008000, a mid-tone by definition).
	g->setFillColor(Color::LIME);
	g->beginPath();
	g->arc(230.0f, 70.0f, 45.0f, 0.0f, 6.2831853f, false);
	g->fill();

	// A rotated square exercises the transform stack together with fills.
	g->save();
	g->translate(230.0f, 170.0f);
	g->rotate(0.3926991f);            // 22.5 degrees, deliberately off-axis
	g->setFillColor(Color::MAGENTA);
	g->fillRect(-35.0f, -35.0f, 70.0f, 70.0f);
	g->restore();

	// A thin near-horizontal wedge: the hardest case for edge coverage.
	g->setFillColor(Color::BLACK);
	g->beginPath();
	g->moveTo(20.0f, 210.0f);
	g->lineTo(170.0f, 200.0f);
	g->lineTo(170.0f, 205.0f);
	g->closePath();
	g->fill();
}

/** One rendered scene, kept as 32-bit BGRA top-down rows. */
struct Rendered {
	std::vector<unsigned char> pixels;
	int width;
	int height;
	Rendered() : width(0), height(0) { }
};

/** Renders the scene into an offscreen DIB using the selected backend. */
bool render(int backend, Rendered& out) {
	Graphics::setGraphicsImplementation(backend);

	BITMAPINFO bi;
	::memset(&bi, 0, sizeof(bi));
	bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth       = SCENE_W;
	bi.bmiHeader.biHeight      = -SCENE_H;      // negative: top-down
	bi.bmiHeader.biPlanes      = 1;
	bi.bmiHeader.biBitCount    = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	void* bits = NULL;
	HDC screen = ::GetDC(NULL);
	HBITMAP dib = ::CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	::ReleaseDC(NULL, screen);
	if (dib == NULL || bits == NULL)
		return false;

	HDC dc = ::CreateCompatibleDC(NULL);
	HGDIOBJ prev = ::SelectObject(dc, dib);

	Graphics* g = createBackendGraphics(dc, false);
	if (g == NULL) {
		::SelectObject(dc, prev);
		::DeleteDC(dc);
		::DeleteObject(dib);
		return false;
	}
	drawScene(g);
	destroyBackendGraphics(g);

	::GdiFlush();
	out.width = SCENE_W;
	out.height = SCENE_H;
	out.pixels.assign((unsigned char*) bits,
	                  (unsigned char*) bits + (size_t) SCENE_W * SCENE_H * 4);

	::SelectObject(dc, prev);
	::DeleteDC(dc);
	::DeleteObject(dib);
	return true;
}

/** Writes a 32-bit BGRA buffer out as an uncompressed .bmp. */
bool writeBmp(const char* path, const Rendered& r) {
	FILE* f = ::fopen(path, "wb");
	if (f == NULL) return false;

	const unsigned int pixelBytes = (unsigned int) r.width * (unsigned int) r.height * 4u;
	BITMAPFILEHEADER fh;
	::memset(&fh, 0, sizeof(fh));
	fh.bfType    = 0x4D42;   // "BM"
	fh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	fh.bfSize    = fh.bfOffBits + pixelBytes;

	BITMAPINFOHEADER ih;
	::memset(&ih, 0, sizeof(ih));
	ih.biSize        = sizeof(BITMAPINFOHEADER);
	ih.biWidth       = r.width;
	ih.biHeight      = -r.height;   // top-down, matching the buffer
	ih.biPlanes      = 1;
	ih.biBitCount    = 32;
	ih.biCompression = BI_RGB;
	ih.biSizeImage   = pixelBytes;

	::fwrite(&fh, sizeof(fh), 1, f);
	::fwrite(&ih, sizeof(ih), 1, f);
	::fwrite(&r.pixels[0], 1, pixelBytes, f);
	::fclose(f);
	return true;
}

/**
 * Counts pixels that are neither pure background nor a pure shape colour.
 *
 * Antialiasing works by emitting partial-coverage pixels along every edge, so
 * a smoothed render has many intermediate tones and a hard-edged one has
 * almost none. That count is the measurable signature of antialiasing.
 */
int countIntermediateTones(const Rendered& r) {
	int count = 0;
	for (size_t i = 0; i + 3 < r.pixels.size(); i += 4) {
		const int b = r.pixels[i];
		const int g = r.pixels[i + 1];
		const int rd = r.pixels[i + 2];
		// Pure white, or a channel set that is fully saturated/empty, is a
		// solid pixel. Anything partway between is a blended edge.
		const bool solid =
			(b == 255 && g == 255 && rd == 255) ||
			((b == 0 || b == 255) && (g == 0 || g == 255) && (rd == 0 || rd == 255));
		if (!solid)
			++count;
	}
	return count;
}

} // namespace

int test_graphics_backends() {
	int failures = 0;
	std::cout << "--- Canvas graphics backends ---" << std::endl;

	Rendered gdi, gdiplus;
	const bool gdiOk = render(GRAPHICS_GDI, gdi);
	const bool gdiplusOk = render(GRAPHICS_GDIPLUS, gdiplus);

	if (!gdiOk || !gdiplusOk) {
		std::cout << "  render failed (gdi=" << gdiOk << " gdiplus=" << gdiplusOk << ")" << std::endl;
		return 1;
	}

	const int gdiEdges = countIntermediateTones(gdi);
	const int gdiplusEdges = countIntermediateTones(gdiplus);

	writeBmp("gdi.bmp", gdi);
	writeBmp("gdiplus.bmp", gdiplus);

	std::cout << "  GDI     blended edge pixels: " << gdiEdges << "   -> gdi.bmp" << std::endl;
	std::cout << "  GDI+    blended edge pixels: " << gdiplusEdges << "   -> gdiplus.bmp" << std::endl;

	// GDI cannot antialias, so it should produce essentially no partial
	// coverage; GDI+ with smoothing on should produce a great deal.
	if (gdiplusEdges > gdiEdges * 10 && gdiplusEdges > 500) {
		std::cout << "  PASS: GDI+ antialiases, GDI does not." << std::endl;
	} else {
		std::cout << "  FAIL: expected far more blended pixels from GDI+." << std::endl;
		++failures;
	}

	// And the opt-out has to actually take effect.
	Graphics::setGraphicsImplementation(GRAPHICS_GDIPLUS);
	Rendered off;
	{
		BITMAPINFO bi;
		::memset(&bi, 0, sizeof(bi));
		bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth       = SCENE_W;
		bi.bmiHeader.biHeight      = -SCENE_H;
		bi.bmiHeader.biPlanes      = 1;
		bi.bmiHeader.biBitCount    = 32;
		bi.bmiHeader.biCompression = BI_RGB;
		void* bits = NULL;
		HDC screen = ::GetDC(NULL);
		HBITMAP dib = ::CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
		::ReleaseDC(NULL, screen);
		HDC dc = ::CreateCompatibleDC(NULL);
		HGDIOBJ prev = ::SelectObject(dc, dib);
		Graphics* g = createBackendGraphics(dc, false);
		g->setAntialias(false);              // the opt-out under test
		drawScene(g);
		destroyBackendGraphics(g);
		::GdiFlush();
		off.width = SCENE_W;
		off.height = SCENE_H;
		off.pixels.assign((unsigned char*) bits,
		                  (unsigned char*) bits + (size_t) SCENE_W * SCENE_H * 4);
		::SelectObject(dc, prev);
		::DeleteDC(dc);
		::DeleteObject(dib);
	}
	const int offEdges = countIntermediateTones(off);
	writeBmp("gdiplus_noaa.bmp", off);
	std::cout << "  GDI+ AA off blended edge pixels: " << offEdges << "   -> gdiplus_noaa.bmp" << std::endl;

	if (offEdges < gdiplusEdges / 4) {
		std::cout << "  PASS: setAntialias(false) is honoured." << std::endl;
	} else {
		std::cout << "  FAIL: disabling antialiasing changed little." << std::endl;
		++failures;
	}
	return failures;
}
