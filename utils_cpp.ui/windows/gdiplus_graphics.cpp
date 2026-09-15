/*
 * gdiplus_graphics.cpp
 *
 * GDI+ implementation of the Canvas-style Graphics interface.
 */

#include "gdiplus_graphics.h"
#include "image_w32.h"
#include <cmath>
#include <cstring>
#include <string>

namespace utils {
namespace ui {

using namespace Gdiplus;

static const float PI_F = 3.14159265358979323846f;

// ---------------------------------------------------------------------------
// Library lifetime
// ---------------------------------------------------------------------------

void gdiplusEnsureStarted() {
	// Function-local static: initialised once, thread-safely, on first use.
	struct Starter {
		ULONG_PTR token;
		Starter() : token(0) {
			GdiplusStartupInput input;
			GdiplusStartup(&token, &input, NULL);
		}
		// No destructor calling GdiplusShutdown -- see the header.
	};
	static Starter starter;
	(void) starter;
}

// ---------------------------------------------------------------------------
// Small conversions
// ---------------------------------------------------------------------------

static Gdiplus::LineJoin toGdiPlusJoin(const utils::ui::LineJoin& j) {
	if (j == utils::ui::LineJoin::BEVEL) return LineJoinBevel;
	if (j == utils::ui::LineJoin::ROUND) return LineJoinRound;
	return LineJoinMiter;
}

static Gdiplus::LineCap toGdiPlusCap(const utils::ui::LineCap& c) {
	if (c == utils::ui::LineCap::ROUND)  return LineCapRound;
	if (c == utils::ui::LineCap::SQUARE) return LineCapSquare;
	return LineCapFlat;
}

static Gdiplus::DashStyle toGdiPlusDash(const utils::ui::LineStyle& s) {
	if (s == utils::ui::LineStyle::DASH)       return DashStyleDash;
	if (s == utils::ui::LineStyle::DOT)        return DashStyleDot;
	if (s == utils::ui::LineStyle::DASHDOT)    return DashStyleDashDot;
	if (s == utils::ui::LineStyle::DASHDOTDOT) return DashStyleDashDotDot;
	return DashStyleSolid;
}

/** Widens a narrow string for the GDI+ text and font APIs, which are UTF-16. */
static std::wstring widen(const char* s) {
	if (s == NULL) return std::wstring();
	const int len = (int) ::strlen(s);
	if (len == 0) return std::wstring();
	const int need = ::MultiByteToWideChar(CP_UTF8, 0, s, len, NULL, 0);
	if (need <= 0) return std::wstring();
	std::wstring out((size_t) need, L'\0');
	::MultiByteToWideChar(CP_UTF8, 0, s, len, &out[0], need);
	return out;
}

// ---------------------------------------------------------------------------
// GdiPlusGraphics
// ---------------------------------------------------------------------------

GdiPlusGraphics* GdiPlusGraphics::createGraphics(HDC _hdc, bool _ownsDC) {
	return new GdiPlusGraphics(_hdc, _ownsDC);
}

void GdiPlusGraphics::deleteGraphics(GdiPlusGraphics* _graphics) {
	delete _graphics;
}

GdiPlusGraphics::GdiPlusGraphics(HDC _hdc, bool _ownsDC) :
		hdc(_hdc),
		ownsDC(_ownsDC),
		g(NULL),
		path(NULL),
		currentPoint(0.0f, 0.0f),
		hasCurrentPoint(false),
		figureOpen(false),
		stateStack(),
		lineColor(Color::BLACK),
		fillColor(Color::BLACK),
		textColor(Color::BLACK),
		currentFillStyle(),
		currentStroke(),
		currentFont(NULL),
		globalCompositeOperation(CompositeOperation::SOURCE_COPY),
		alpha(1.0f),
		// Antialiasing defaults ON: it is what a Canvas-shaped API implies,
		// and callers that need crisp pixel edges opt out explicitly.
		antialias(true),
		imageSmoothing(true) {
	gdiplusEnsureStarted();
	g = new Gdiplus::Graphics(hdc);
	path = new GraphicsPath(FillModeWinding);   // Canvas default: non-zero
	applyQuality();
}

GdiPlusGraphics::~GdiPlusGraphics() {
	// currentFillStyle and currentStroke release themselves.
	delete path;
	delete g;
	if (ownsDC && hdc != NULL)
		::DeleteDC(hdc);
}

void GdiPlusGraphics::applyQuality() {
	g->SetSmoothingMode(antialias ? SmoothingModeAntiAlias : SmoothingModeNone);
	// Text is smoothed in step with geometry so a scene is internally
	// consistent; ClearType is avoided because its subpixel output is wrong
	// when the surface is later composited or scaled.
	g->SetTextRenderingHint(antialias ? TextRenderingHintAntiAliasGridFit
	                                  : TextRenderingHintSingleBitPerPixelGridFit);
	g->SetInterpolationMode(imageSmoothing ? InterpolationModeHighQualityBilinear
	                                       : InterpolationModeNearestNeighbor);
	g->SetPixelOffsetMode(antialias ? PixelOffsetModeHalf : PixelOffsetModeNone);
}

Gdiplus::Color GdiPlusGraphics::toGdiPlus(const utils::ui::Color& c) const {
	float a = (float) c.get_alpha() * (alpha < 0.0f ? 0.0f : (alpha > 1.0f ? 1.0f : alpha));
	if (a > 255.0f) a = 255.0f;
	return Gdiplus::Color((BYTE) (a + 0.5f), c.get_red(), c.get_green(), c.get_blue());
}

Gdiplus::Brush* GdiPlusGraphics::makeFillBrush() const {
	if (currentFillStyle) {
		if (currentFillStyle->isGradient()) {
			const GdiPlusGradient* grad = dynamic_cast<const GdiPlusGradient*>(currentFillStyle.get());
			if (grad != NULL)
				return grad->makeBrush(alpha);
		} else {
			const GdiPlusPattern* pat = dynamic_cast<const GdiPlusPattern*>(currentFillStyle.get());
			if (pat != NULL)
				return pat->makeBrush();
			const ColorBrush* cb = dynamic_cast<const ColorBrush*>(currentFillStyle.get());
			if (cb != NULL)
				return new SolidBrush(toGdiPlus(cb->getColor()));
		}
	}
	return new SolidBrush(toGdiPlus(fillColor));
}

Gdiplus::Pen* GdiPlusGraphics::makeStrokePen() const {
	float width = 1.0f;
	Pen* pen;
	if (currentStroke)
		width = currentStroke->getWidth();
	if (width <= 0.0f) width = 1.0f;
	pen = new Pen(toGdiPlus(lineColor), width);
	if (currentStroke) {
		pen->SetLineJoin(toGdiPlusJoin(currentStroke->getJoin()));
		pen->SetStartCap(toGdiPlusCap(currentStroke->getCap()));
		pen->SetEndCap(toGdiPlusCap(currentStroke->getCap()));
		const utils::ui::LineStyle ls = currentStroke->getStyle();
		if (ls == utils::ui::LineStyle::NULL_LINE)
			pen->SetColor(Gdiplus::Color(0, 0, 0, 0));
		else
			pen->SetDashStyle(toGdiPlusDash(ls));
	}
	return pen;
}

// state ---------------------------------------------------------------------

void GdiPlusGraphics::save() {
	GdiPlusState s;
	s.token          = g->Save();
	s.lineColor      = lineColor;
	s.fillColor      = fillColor;
	s.textColor      = textColor;
	// Copying the Ref takes the reference that keeps the stacked style alive.
	s.fillStyle      = currentFillStyle;
	s.stroke         = currentStroke;
	s.font           = currentFont;
	s.alpha          = alpha;
	s.antialias      = antialias;
	s.imageSmoothing = imageSmoothing;
	s.composite      = globalCompositeOperation;
	stateStack.push_back(s);
}

void GdiPlusGraphics::restore() {
	if (stateStack.empty())
		return;   // Canvas ignores an unmatched restore()
	const GdiPlusState s = stateStack.back();
	stateStack.pop_back();
	g->Restore(s.token);
	lineColor                = s.lineColor;
	fillColor                = s.fillColor;
	textColor                = s.textColor;
	currentFillStyle         = s.fillStyle;
	currentStroke            = s.stroke;
	currentFont              = s.font;
	alpha                    = s.alpha;
	antialias                = s.antialias;
	imageSmoothing           = s.imageSmoothing;
	globalCompositeOperation = s.composite;
	applyQuality();
}

// transformations -----------------------------------------------------------

void GdiPlusGraphics::scale(float x, float y)      { g->ScaleTransform(x, y); }
void GdiPlusGraphics::rotate(float angle)          { g->RotateTransform(angle * 180.0f / PI_F); }
void GdiPlusGraphics::translate(float x, float y)  { g->TranslateTransform(x, y); }

void GdiPlusGraphics::transform(float a, float b, float c, float d, float e, float f) {
	Matrix m(a, b, c, d, e, f);
	g->MultiplyTransform(&m, MatrixOrderPrepend);
}

void GdiPlusGraphics::setTransform(float a, float b, float c, float d, float e, float f) {
	Matrix m(a, b, c, d, e, f);
	g->SetTransform(&m);
}

// colors and styles ---------------------------------------------------------

Gradient* GdiPlusGraphics::createLinearGradient(float x0, float y0, float x1, float y1) {
	return new GdiPlusGradient(false, x0, y0, 0.0f, x1, y1, 0.0f);
}

Gradient* GdiPlusGraphics::createRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1) {
	return new GdiPlusGradient(true, x0, y0, r0, x1, y1, r1);
}

Pattern* GdiPlusGraphics::createPattern(Image* image, long repetition) {
	return new GdiPlusPattern(image, (int) repetition);
}

Stroke* GdiPlusGraphics::createStroke(const utils::ui::LineStyle& style, const utils::ui::LineJoin& join,
		const utils::ui::LineCap& cap, float width) {
	return new GdiPlusStroke(style, join, cap, width);
}

float GdiPlusGraphics::getGlobalAlpha() { return alpha; }
void  GdiPlusGraphics::setGlobalAlpha(float value) {
	alpha = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

CompositeOperation GdiPlusGraphics::getGlobalCompositeOperation() {
	return globalCompositeOperation;
}

void GdiPlusGraphics::setGlobalCompositeOperation(const CompositeOperation& go) {
	globalCompositeOperation = go;
	// GDI+ offers only two compositing modes. SOURCE_COPY overwrites the
	// destination including its alpha; everything else falls back to the
	// normal over-blend. The raster-op style operations (AND/OR/XOR/INVERT)
	// have no GDI+ equivalent and are ignored -- they need Direct2D.
	g->SetCompositingMode(go == CompositeOperation::SOURCE_COPY
			? CompositingModeSourceCopy : CompositingModeSourceOver);
}

bool GdiPlusGraphics::getAntialias() { return antialias; }

void GdiPlusGraphics::setAntialias(bool value) {
	antialias = value;
	applyQuality();
}

bool GdiPlusGraphics::getImageSmoothing() { return imageSmoothing; }

void GdiPlusGraphics::setImageSmoothing(bool value) {
	imageSmoothing = value;
	applyQuality();
}

utils::ui::Color GdiPlusGraphics::getLineColor()  { return lineColor; }
void GdiPlusGraphics::setLineColor(const utils::ui::Color& value) { lineColor = value; }

utils::ui::Color GdiPlusGraphics::getFillColor()  { return fillColor; }
void GdiPlusGraphics::setFillColor(const utils::ui::Color& value) { fillColor = value; }

utils::ui::Color GdiPlusGraphics::getTextColor()  { return textColor; }
void GdiPlusGraphics::setTextColor(const utils::ui::Color& value) { textColor = value; }

FillStyle* GdiPlusGraphics::getFillStyle() { return currentFillStyle; }

void GdiPlusGraphics::setFillStyle(FillStyle* value) {
	// Assigning the Ref retains the new style and releases the old one, in
	// that order, so passing back the style already held is harmless.
	currentFillStyle = value;
}

Stroke* GdiPlusGraphics::getStroke() { return currentStroke; }

void GdiPlusGraphics::setStroke(Stroke* value) {
	currentStroke = value;
}

// rects ---------------------------------------------------------------------

void GdiPlusGraphics::clearRect(float x, float y, float w, float h) {
	// Canvas clearRect erases to transparent black, which means replacing the
	// pixels rather than blending over them.
	const CompositingMode saved = g->GetCompositingMode();
	g->SetCompositingMode(CompositingModeSourceCopy);
	SolidBrush transparent(Gdiplus::Color(0, 0, 0, 0));
	g->FillRectangle(&transparent, x, y, w, h);
	g->SetCompositingMode(saved);
}

void GdiPlusGraphics::fillRect(float x, float y, float w, float h) {
	Brush* b = makeFillBrush();
	g->FillRectangle(b, x, y, w, h);
	delete b;
}

void GdiPlusGraphics::strokeRect(float x, float y, float w, float h) {
	Pen* p = makeStrokePen();
	g->DrawRectangle(p, x, y, w, h);
	delete p;
}

// paths ---------------------------------------------------------------------

void GdiPlusGraphics::beginPath() {
	path->Reset();
	path->SetFillMode(FillModeWinding);
	hasCurrentPoint = false;
	figureOpen = false;
}

void GdiPlusGraphics::closePath() {
	if (figureOpen) {
		path->CloseFigure();
		figureOpen = false;
	}
}

void GdiPlusGraphics::fill() {
	Brush* b = makeFillBrush();
	g->FillPath(b, path);
	delete b;
}

void GdiPlusGraphics::stroke() {
	Pen* p = makeStrokePen();
	g->DrawPath(p, path);
	delete p;
}

void GdiPlusGraphics::strokeAndFill() {
	fill();
	stroke();
}

void GdiPlusGraphics::clip() {
	g->SetClip(path, CombineModeIntersect);
}

void GdiPlusGraphics::moveTo(float x, float y) {
	// A new subpath. StartFigure leaves any previous one open, matching
	// Canvas, where moveTo does not close what came before.
	path->StartFigure();
	currentPoint = PointF(x, y);
	hasCurrentPoint = true;
	figureOpen = true;
}

void GdiPlusGraphics::lineTo(float x, float y) {
	if (!hasCurrentPoint) {
		// Canvas treats lineTo with no current point as a moveTo.
		moveTo(x, y);
		return;
	}
	path->AddLine(currentPoint.X, currentPoint.Y, x, y);
	currentPoint = PointF(x, y);
	figureOpen = true;
}

void GdiPlusGraphics::quadraticCurveTo(float cpx, float cpy, float x, float y) {
	if (!hasCurrentPoint)
		moveTo(cpx, cpy);
	// GDI+ has no quadratic segment. Raise the degree: a quadratic with
	// control C between P0 and P1 is the cubic with controls
	// P0 + 2/3 (C - P0) and P1 + 2/3 (C - P1).
	const float x0 = currentPoint.X, y0 = currentPoint.Y;
	const float c1x = x0 + 2.0f / 3.0f * (cpx - x0);
	const float c1y = y0 + 2.0f / 3.0f * (cpy - y0);
	const float c2x = x  + 2.0f / 3.0f * (cpx - x);
	const float c2y = y  + 2.0f / 3.0f * (cpy - y);
	path->AddBezier(x0, y0, c1x, c1y, c2x, c2y, x, y);
	currentPoint = PointF(x, y);
	figureOpen = true;
}

void GdiPlusGraphics::bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y) {
	if (!hasCurrentPoint)
		moveTo(cp1x, cp1y);
	path->AddBezier(currentPoint.X, currentPoint.Y, cp1x, cp1y, cp2x, cp2y, x, y);
	currentPoint = PointF(x, y);
	figureOpen = true;
}

void GdiPlusGraphics::arcTo(float x1, float y1, float x2, float y2, float radius) {
	// The Canvas tangent arc: a circle of the given radius tangent to both
	// the line from the current point to (x1,y1) and the line from (x1,y1)
	// to (x2,y2), joined to the current point by a straight segment.
	if (!hasCurrentPoint) {
		moveTo(x1, y1);
		return;
	}
	const float x0 = currentPoint.X, y0 = currentPoint.Y;
	if (radius <= 0.0f) {
		lineTo(x1, y1);
		return;
	}

	float v1x = x0 - x1, v1y = y0 - y1;
	float v2x = x2 - x1, v2y = y2 - y1;
	const float l1 = std::sqrt(v1x * v1x + v1y * v1y);
	const float l2 = std::sqrt(v2x * v2x + v2y * v2y);
	if (l1 < 1e-6f || l2 < 1e-6f) {
		lineTo(x1, y1);
		return;
	}
	v1x /= l1; v1y /= l1;
	v2x /= l2; v2y /= l2;

	// Collinear points leave no room for an arc.
	const float cross = v1x * v2y - v1y * v2x;
	if (std::fabs(cross) < 1e-6f) {
		lineTo(x1, y1);
		return;
	}

	float cosA = v1x * v2x + v1y * v2y;
	if (cosA < -1.0f) cosA = -1.0f;
	if (cosA >  1.0f) cosA =  1.0f;
	const float angle = std::acos(cosA);
	const float tanDist = radius / std::tan(angle / 2.0f);

	// Tangent points along each leg, and the arc centre on the bisector.
	const float t1x = x1 + v1x * tanDist, t1y = y1 + v1y * tanDist;
	const float t2x = x1 + v2x * tanDist, t2y = y1 + v2y * tanDist;

	float bx = v1x + v2x, by = v1y + v2y;
	const float bl = std::sqrt(bx * bx + by * by);
	if (bl < 1e-6f) {
		lineTo(x1, y1);
		return;
	}
	bx /= bl; by /= bl;
	const float centreDist = radius / std::sin(angle / 2.0f);
	const float cx = x1 + bx * centreDist, cy = y1 + by * centreDist;

	float start = std::atan2(t1y - cy, t1x - cx) * 180.0f / PI_F;
	float end   = std::atan2(t2y - cy, t2x - cx) * 180.0f / PI_F;
	float sweep = end - start;
	while (sweep <= -180.0f) sweep += 360.0f;
	while (sweep >   180.0f) sweep -= 360.0f;

	path->AddLine(x0, y0, t1x, t1y);
	path->AddArc(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, start, sweep);
	currentPoint = PointF(t2x, t2y);
	figureOpen = true;
}

void GdiPlusGraphics::rect(float x, float y, float w, float h) {
	// Canvas rect() adds a closed subpath and leaves the current point at
	// its origin.
	path->AddRectangle(RectF(x, y, w, h));
	path->StartFigure();
	currentPoint = PointF(x, y);
	hasCurrentPoint = true;
	figureOpen = false;
}

void GdiPlusGraphics::arc(float x, float y, float radius, float startAngle, float endAngle, bool anticlockwise) {
	float start = startAngle * 180.0f / PI_F;
	float end   = endAngle   * 180.0f / PI_F;
	float sweep = end - start;

	// Canvas normalises the sweep by direction, and a full turn stays a full
	// circle rather than collapsing to nothing.
	if (!anticlockwise) {
		if (sweep >= 360.0f) sweep = 360.0f;
		else while (sweep < 0.0f) sweep += 360.0f;
	} else {
		if (sweep <= -360.0f) sweep = -360.0f;
		else while (sweep > 0.0f) sweep -= 360.0f;
	}

	path->AddArc(x - radius, y - radius, radius * 2.0f, radius * 2.0f, start, sweep);
	const float endRad = (start + sweep) * PI_F / 180.0f;
	currentPoint = PointF(x + radius * std::cos(endRad), y + radius * std::sin(endRad));
	hasCurrentPoint = true;
	figureOpen = true;
}

// text ----------------------------------------------------------------------

Font* GdiPlusGraphics::getFont() { return currentFont; }
void GdiPlusGraphics::setFont(Font* font) { currentFont = font; }

void GdiPlusGraphics::fillText(const char* text, float x, float y, float maxWidth) {
	fillText(widen(text).c_str(), x, y, maxWidth);
}

void GdiPlusGraphics::strokeText(const char* text, float x, float y, float maxWidth) {
	strokeText(widen(text).c_str(), x, y, maxWidth);
}

void GdiPlusGraphics::fillText(const wchar_t* text, float x, float y, float maxWidth) {
	if (text == NULL) return;
	const GdiPlusFont* f = dynamic_cast<const GdiPlusFont*>(currentFont);
	GdiPlusFont fallback("Segoe UI", Font::PLAIN, 12);
	Gdiplus::Font* gf = (f != NULL ? f : &fallback)->makeGdiPlusFont();
	SolidBrush brush(toGdiPlus(textColor));
	if (maxWidth > 0.0f) {
		RectF layout(x, y, maxWidth, 1e6f);
		g->DrawString(text, -1, gf, layout, NULL, &brush);
	} else {
		g->DrawString(text, -1, gf, PointF(x, y), &brush);
	}
	delete gf;
}

void GdiPlusGraphics::strokeText(const wchar_t* text, float x, float y, float maxWidth) {
	if (text == NULL) return;
	// Outlined text: put the glyphs into a path and stroke it.
	const GdiPlusFont* f = dynamic_cast<const GdiPlusFont*>(currentFont);
	GdiPlusFont fallback("Segoe UI", Font::PLAIN, 12);
	const GdiPlusFont* use = (f != NULL ? f : &fallback);

	FontFamily family(widen(use->getFace()).c_str());
	int style = FontStyleRegular;
	if (use->getStyle() & Font::BOLD)   style |= FontStyleBold;
	if (use->getStyle() & Font::ITALIC) style |= FontStyleItalic;

	GraphicsPath textPath(FillModeWinding);
	StringFormat fmt;
	textPath.AddString(text, -1, &family, style, (REAL) use->getSize(),
			PointF(x, y), &fmt);
	Pen* p = makeStrokePen();
	g->DrawPath(p, &textPath);
	delete p;
	(void) maxWidth;
}

float GdiPlusGraphics::measureText(const char* text) {
	return measureText(widen(text).c_str());
}

float GdiPlusGraphics::measureText(const wchar_t* text) {
	if (text == NULL) return 0.0f;
	const GdiPlusFont* f = dynamic_cast<const GdiPlusFont*>(currentFont);
	GdiPlusFont fallback("Segoe UI", Font::PLAIN, 12);
	Gdiplus::Font* gf = (f != NULL ? f : &fallback)->makeGdiPlusFont();
	RectF bounds;
	// A zero-origin layout rect with GenericTypographic avoids the padding
	// DrawString would otherwise add around the run.
	g->MeasureString(text, -1, gf, PointF(0.0f, 0.0f),
			StringFormat::GenericTypographic(), &bounds);
	delete gf;
	return bounds.Width;
}

// images --------------------------------------------------------------------

/** Wraps an Image_W32 HBITMAP for GDI+; the caller owns the result. */
static Gdiplus::Bitmap* toGdiPlusBitmap(Image* image) {
	Image_W32* w32 = dynamic_cast<Image_W32*>(image);
	if (w32 == NULL || w32->hBitmap == NULL)
		return NULL;
	return Gdiplus::Bitmap::FromHBITMAP(w32->hBitmap, NULL);
}

void GdiPlusGraphics::drawImage(Image* image, float dx, float dy) {
	Gdiplus::Bitmap* bmp = toGdiPlusBitmap(image);
	if (bmp == NULL) return;
	g->DrawImage(bmp, dx, dy);
	delete bmp;
}

void GdiPlusGraphics::drawImage(Image* image, float dx, float dy, float dw, float dh) {
	Gdiplus::Bitmap* bmp = toGdiPlusBitmap(image);
	if (bmp == NULL) return;
	g->DrawImage(bmp, RectF(dx, dy, dw, dh));
	delete bmp;
}

void GdiPlusGraphics::drawImage(Image* image, float sx, float sy, float sw, float sh,
		float dx, float dy, float dw, float dh) {
	Gdiplus::Bitmap* bmp = toGdiPlusBitmap(image);
	if (bmp == NULL) return;
	g->DrawImage(bmp, RectF(dx, dy, dw, dh), sx, sy, sw, sh, UnitPixel);
	delete bmp;
}

Image* GdiPlusGraphics::createImage(float sw, float sh) {
	return Image::createCompatibleImage((int) sw, (int) sh);
}

Image* GdiPlusGraphics::createImage(Image* imagedata) {
	return Image::createImage(imagedata);
}

Image* GdiPlusGraphics::getImage(float sx, float sy, float sw, float sh) {
	// Copy the region out of the underlying DC into a fresh compatible image.
	const int w = (int) sw, h = (int) sh;
	if (w <= 0 || h <= 0) return NULL;
	Image* dest = Image::createCompatibleImage(w, h);
	Image_W32* destW32 = dynamic_cast<Image_W32*>(dest);
	if (destW32 == NULL) return dest;
	HDC memDC = ::CreateCompatibleDC(hdc);
	HGDIOBJ old = ::SelectObject(memDC, destW32->hBitmap);
	::BitBlt(memDC, 0, 0, w, h, hdc, (int) sx, (int) sy, SRCCOPY);
	::SelectObject(memDC, old);
	::DeleteDC(memDC);
	return dest;
}

// ---------------------------------------------------------------------------
// GdiPlusStroke
// ---------------------------------------------------------------------------

GdiPlusStroke::GdiPlusStroke(const utils::ui::LineStyle& _style, const utils::ui::LineJoin& _join,
		const utils::ui::LineCap& _cap, float _width) :
		style(_style), join(_join), cap(_cap), width(_width) {
}
GdiPlusStroke::~GdiPlusStroke() { }
utils::ui::LineStyle GdiPlusStroke::getStyle() const { return style; }
utils::ui::LineJoin  GdiPlusStroke::getJoin() const  { return join; }
utils::ui::LineCap   GdiPlusStroke::getCap() const   { return cap; }
float GdiPlusStroke::getWidth() const { return width; }

// ---------------------------------------------------------------------------
// GdiPlusFont
// ---------------------------------------------------------------------------

GdiPlusFont::GdiPlusFont(const char* facename, int _style, int _size) :
		size(_size), style(_style) {
	fontface[0] = 0;
	if (facename != NULL) {
		::strncpy(fontface, facename, sizeof(fontface) - 1);
		fontface[sizeof(fontface) - 1] = 0;
	}
}
GdiPlusFont::~GdiPlusFont() { }
const char* GdiPlusFont::getFace() const { return fontface; }
int GdiPlusFont::getSize() const  { return size; }
int GdiPlusFont::getStyle() const { return style; }

Gdiplus::Font* GdiPlusFont::makeGdiPlusFont() const {
	int gdiStyle = FontStyleRegular;
	if (style & Font::BOLD)        gdiStyle |= FontStyleBold;
	if (style & Font::ITALIC)      gdiStyle |= FontStyleItalic;
	if (style & Font::UNDERLINE)   gdiStyle |= FontStyleUnderline;
	if (style & Font::STRIKE_OUT)  gdiStyle |= FontStyleStrikeout;
	const std::wstring face = widen(fontface[0] ? fontface : "Segoe UI");
	return new Gdiplus::Font(face.c_str(), (REAL) size, gdiStyle, UnitPixel);
}

// ---------------------------------------------------------------------------
// GdiPlusGradient
// ---------------------------------------------------------------------------

GdiPlusGradient::GdiPlusGradient(bool _radial, float _x0, float _y0, float _r0,
		float _x1, float _y1, float _r1) :
		radial(_radial), x0(_x0), y0(_y0), r0(_r0), x1(_x1), y1(_y1), r1(_r1), stops() {
}
GdiPlusGradient::~GdiPlusGradient() { }

void GdiPlusGradient::addColor(float x, float y, const utils::ui::Color& c) {
	// x is the Canvas colour-stop offset in [0,1]; y is unused here and kept
	// only because the interface predates that reading.
	(void) y;
	GradientStop s;
	s.offset = x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
	s.color  = c;
	stops.push_back(s);
}

int GdiPlusGradient::getSize() const { return (int) stops.size(); }
int GdiPlusGradient::getDirection() const {
	if (radial) return Gradient::TRIANGLE;
	return (std::fabs(y1 - y0) > std::fabs(x1 - x0)) ? Gradient::VERTICAL : Gradient::HORIZONTAL;
}

Gdiplus::Brush* GdiPlusGradient::makeBrush(float alpha) const {
	const size_t n = stops.size();
	if (n == 0)
		return new SolidBrush(Gdiplus::Color(0, 0, 0, 0));

	// GDI+ wants the stops sorted and spanning 0..1.
	std::vector<Gdiplus::Color> colors;
	std::vector<REAL> offsets;
	colors.reserve(n + 2);
	offsets.reserve(n + 2);
	for (size_t i = 0; i < n; ++i) {
		const utils::ui::Color& c = stops[i].color;
		float a = (float) c.get_alpha() * alpha;
		if (a < 0.0f) a = 0.0f;
		if (a > 255.0f) a = 255.0f;
		colors.push_back(Gdiplus::Color((BYTE) (a + 0.5f), c.get_red(), c.get_green(), c.get_blue()));
		offsets.push_back((REAL) stops[i].offset);
	}
	if (offsets.front() > 0.0f) {
		colors.insert(colors.begin(), colors.front());
		offsets.insert(offsets.begin(), 0.0f);
	}
	if (offsets.back() < 1.0f) {
		colors.push_back(colors.back());
		offsets.push_back(1.0f);
	}

	if (!radial) {
		LinearGradientBrush* b = new LinearGradientBrush(
				PointF(x0, y0), PointF(x1, y1), colors.front(), colors.back());
		b->SetInterpolationColors(&colors[0], &offsets[0], (INT) colors.size());
		b->SetWrapMode(WrapModeTileFlipXY);
		return b;
	}

	// Canvas radial gradients are a two-circle cone, which GDI+ has no brush
	// for. PathGradientBrush over the outer circle is the closest available
	// shape: it interpolates from a centre point out to the boundary, so the
	// inner radius r0 and a centre offset from (x1,y1) are approximated
	// rather than reproduced. Fidelity here needs Direct2D or a custom
	// gradient evaluation.
	const float outer = r1 > r0 ? r1 : r0;
	GraphicsPath circle;
	circle.AddEllipse(x1 - outer, y1 - outer, outer * 2.0f, outer * 2.0f);
	PathGradientBrush* b = new PathGradientBrush(&circle);
	b->SetCenterPoint(PointF(x0, y0));
	b->SetCenterColor(colors.front());
	// The boundary takes the last stop; one colour per path point.
	Gdiplus::Color edge = colors.back();
	INT count = 1;
	b->SetSurroundColors(&edge, &count);
	// Reverse the ramp: PathGradientBrush runs centre -> edge, which is the
	// same direction as Canvas, but its interpolation array is edge-first.
	std::vector<Gdiplus::Color> rev(colors.rbegin(), colors.rend());
	std::vector<REAL> revOff(offsets.size());
	for (size_t i = 0; i < offsets.size(); ++i)
		revOff[i] = 1.0f - offsets[offsets.size() - 1 - i];
	b->SetInterpolationColors(&rev[0], &revOff[0], (INT) rev.size());
	return b;
}

// ---------------------------------------------------------------------------
// GdiPlusPattern
// ---------------------------------------------------------------------------

GdiPlusPattern::GdiPlusPattern(Image* _image, int _repeat) : image(_image), repeat(_repeat) { }
GdiPlusPattern::~GdiPlusPattern() { }
Image* GdiPlusPattern::getImage() { return image; }
int GdiPlusPattern::getRepeat() const { return repeat; }

Gdiplus::Brush* GdiPlusPattern::makeBrush() const {
	Gdiplus::Bitmap* bmp = toGdiPlusBitmap(image);
	if (bmp == NULL)
		return new SolidBrush(Gdiplus::Color(0, 0, 0, 0));
	WrapMode mode = WrapModeTile;
	if (repeat == Pattern::NO_REPEAT) mode = WrapModeClamp;
	else if (repeat == Pattern::REPEAT_X) mode = WrapModeTileFlipY;
	else if (repeat == Pattern::REPEAT_Y) mode = WrapModeTileFlipX;
	TextureBrush* b = new TextureBrush(bmp, mode);
	// TextureBrush copies the bitmap data it needs, so the wrapper can go.
	delete bmp;
	return b;
}

// ---------------------------------------------------------------------------
// GdiPlusColorBrush
// ---------------------------------------------------------------------------

GdiPlusColorBrush::GdiPlusColorBrush(const utils::ui::Color& _color) : color(_color) { }
GdiPlusColorBrush::~GdiPlusColorBrush() { }
utils::ui::Color GdiPlusColorBrush::getColor() const { return color; }

}
}
