/*
 * gdiplus_graphics.h
 *
 * Antialiasing backend for the Canvas-style Graphics interface.
 *
 * GDI has no coverage-based rasteriser, so it cannot antialias geometry at
 * all. GDI+ can, and maps closely onto the rest of the interface: it has
 * paths with beziers and arcs, an affine world transform, gradient and
 * texture brushes, pens with joins/caps/dashes, and per-primitive smoothing.
 *
 * Select it at runtime with
 *     Graphics::setGraphicsImplementation(GRAPHICS_GDIPLUS);
 * before the first Graphics is created for a component.
 */

#ifndef GDIPLUS_GRAPHICS_H_
#define GDIPLUS_GRAPHICS_H_

#include <utils/ui/graphics.hpp>
#include <utils/ui/ui_allocator.hpp>
#include <utils/ui/ref.hpp>
#include <windows.h>
// gdiplus.h uses IStream and PROPID, which WIN32_LEAN_AND_MEAN keeps out of
// windows.h. They have to be declared before gdiplus.h is parsed.
#include <objidl.h>

// gdiplus.h refers to unqualified min/max. NOMINMAX (set by the build) removes
// the windows.h macros, so inject the std ones into its namespace instead.
#include <algorithm>
namespace Gdiplus {
	using std::min;
	using std::max;
}
#include <gdiplus.h>

#include <vector>

namespace utils {
namespace ui {

/**
 * Starts GDI+ on first use.
 *
 * Deliberately never calls GdiplusShutdown. Shutdown during static
 * destruction is a well-known crash source, because any GDI+ object still
 * held by another static (a Font, a cached Image) would be destroyed after
 * the library had already torn itself down. Process exit reclaims it anyway.
 */
void gdiplusEnsureStarted();

/**
 * Attributes that save() must preserve and restore() put back.
 *
 * Gdiplus::Graphics::Save covers the transform, the clip and the quality
 * modes, but knows nothing about our colours, stroke or font, so those are
 * stacked alongside its state token.
 */
struct GdiPlusState {
	Gdiplus::GraphicsState token = 0;
	Color lineColor;
	Color fillColor;
	Color textColor;
	// Ref, not a raw pointer: the stacked style has to keep the object alive,
	// or an intervening setFillStyle() frees what restore() would hand back.
	Ref<FillStyle> fillStyle;
	Ref<Stroke> stroke;
	// The font is borrowed: fonts come from Font::createFont and are released
	// with Font::destroyFont by whoever created them.
	Font* font = nullptr;
	float alpha = 1.0f;
	bool antialias = true;
	bool imageSmoothing = true;
	// MAKE_ENUMERATION gives the enumeration classes a private default
	// constructor, so this member needs an initialiser for GdiPlusState to be
	// default-constructible at all.
	CompositeOperation composite = CompositeOperation::SOURCE_COPY;
};

typedef std::vector<GdiPlusState, ui_allocator<GdiPlusState> > GdiPlusStateStack;

class GdiPlusGraphics : public Graphics {
private:
	HDC hdc;
	bool ownsDC;
	Gdiplus::Graphics* g;
	Gdiplus::GraphicsPath* path;

	// Canvas tracks a current point across path calls; GraphicsPath only
	// exposes the last point of the figure being built, so keep our own.
	Gdiplus::PointF currentPoint;
	bool hasCurrentPoint;
	bool figureOpen;

	GdiPlusStateStack stateStack;

	Color lineColor, fillColor, textColor;
	Ref<FillStyle> currentFillStyle;
	Ref<Stroke> currentStroke;
	Font* currentFont;
	CompositeOperation globalCompositeOperation;
	float alpha;
	bool antialias;
	bool imageSmoothing;

	/** Applies globalAlpha to a colour and converts it to the GDI+ form. */
	Gdiplus::Color toGdiPlus(const Color& c) const;
	/** A brush for the current fill style, or for the flat fill colour. */
	Gdiplus::Brush* makeFillBrush() const;
	/** A pen matching the current stroke and line colour. */
	Gdiplus::Pen* makeStrokePen() const;
	void applyQuality();

public:
	GdiPlusGraphics(HDC _hdc, bool _ownsDC);
	virtual ~GdiPlusGraphics();

	// state
	virtual void save();
	virtual void restore();

	// transformations
	virtual void scale(float x, float y);
	virtual void rotate(float angle);
	virtual void translate(float x, float y);
	virtual void transform(float a, float b, float c, float d, float e, float f);
	virtual void setTransform(float a, float b, float c, float d, float e, float f);

	// colors and styles
	virtual Gradient* createLinearGradient(float x0, float y0, float x1, float y1);
	virtual Gradient* createRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1);
	virtual Pattern* createPattern(Image* image, long repetition);
	virtual Stroke* createStroke(const LineStyle& style, const LineJoin& join, const LineCap& cap, float width);

	virtual float getGlobalAlpha();
	virtual void setGlobalAlpha(float value);

	virtual CompositeOperation getGlobalCompositeOperation();
	virtual void setGlobalCompositeOperation(const CompositeOperation& go);

	virtual bool getAntialias();
	virtual void setAntialias(bool value);

	virtual bool getImageSmoothing();
	virtual void setImageSmoothing(bool value);

	virtual Color getLineColor();
	virtual void setLineColor(const Color& value);

	virtual Color getFillColor();
	virtual void setFillColor(const Color& value);

	virtual Color getTextColor();
	virtual void setTextColor(const Color& value);

	virtual FillStyle* getFillStyle();
	virtual void setFillStyle(FillStyle* value);

	virtual Stroke* getStroke();
	virtual void setStroke(Stroke* value);

	// rects
	virtual void clearRect(float x, float y, float w, float h);
	virtual void fillRect(float x, float y, float w, float h);
	virtual void strokeRect(float x, float y, float w, float h);

	// paths
	virtual void beginPath();
	virtual void closePath();
	virtual void fill();
	virtual void stroke();
	virtual void strokeAndFill();
	virtual void clip();

	virtual void moveTo(float x, float y);
	virtual void lineTo(float x, float y);
	virtual void quadraticCurveTo(float cpx, float cpy, float x, float y);
	virtual void bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y);
	virtual void arcTo(float x1, float y1, float x2, float y2, float radius);
	virtual void rect(float x, float y, float w, float h);
	virtual void arc(float x, float y, float radius, float startAngle, float endAngle, bool anticlockwise);

	// text
	virtual Font* getFont();
	virtual void setFont(Font* font);
	virtual void fillText(const char* text, float x, float y, float maxWidth);
	virtual void strokeText(const char* text, float x, float y, float maxWidth);
	virtual void fillText(const wchar_t* text, float x, float y, float maxWidth);
	virtual void strokeText(const wchar_t* text, float x, float y, float maxWidth);
	virtual float measureText(const char* text);
	virtual float measureText(const wchar_t* text);

	// drawing images
	virtual void drawImage(Image* image, float dx, float dy);
	virtual void drawImage(Image* image, float dx, float dy, float dw, float dh);
	virtual void drawImage(Image* image, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);

	// pixel manipulation
	virtual Image* createImage(float sw, float sh);
	virtual Image* createImage(Image* imagedata);
	virtual Image* getImage(float sx, float sy, float sw, float sh);

	static GdiPlusGraphics* createGraphics(HDC _hdc, bool _ownsDC);
	static void deleteGraphics(GdiPlusGraphics* _graphics);
};

/** Stroke attributes; the Pen itself is built per draw from these plus colour. */
class GdiPlusStroke : public Stroke {
public:
	LineStyle style;
	LineJoin join;
	LineCap cap;
	float width;

	GdiPlusStroke(const LineStyle& _style, const LineJoin& _join, const LineCap& _cap, float _width);
	virtual ~GdiPlusStroke();

	virtual LineStyle getStyle() const;
	virtual LineJoin getJoin() const;
	virtual LineCap getCap() const;
	virtual float getWidth() const;
};

class GdiPlusFont : public Font {
public:
	char fontface[64];
	int size;
	int style;

	GdiPlusFont(const char* facename, int _style, int _size);
	virtual ~GdiPlusFont();

	virtual const char* getFace() const;
	virtual int getSize() const;
	virtual int getStyle() const;

	/** Builds a GDI+ font; the caller owns the result. */
	Gdiplus::Font* makeGdiPlusFont() const;
};

/** Colour stop, shared by the linear and radial gradients. */
struct GradientStop {
	float offset;
	Color color;
};

typedef std::vector<GradientStop, ui_allocator<GradientStop> > GradientStops;

class GdiPlusGradient : public Gradient {
public:
	bool radial;
	float x0, y0, r0, x1, y1, r1;
	GradientStops stops;

	GdiPlusGradient(bool _radial, float _x0, float _y0, float _r0, float _x1, float _y1, float _r1);
	virtual ~GdiPlusGradient();

	virtual void addColor(float x, float y, const Color& c);
	virtual int getSize() const;
	virtual int getDirection() const;
	virtual bool isGradient() const { return true; }

	/** Builds a brush honouring globalAlpha; the caller owns the result. */
	Gdiplus::Brush* makeBrush(float alpha) const;
};

class GdiPlusPattern : public Pattern {
public:
	Image* image;
	int repeat;

	GdiPlusPattern(Image* _image, int _repeat);
	virtual ~GdiPlusPattern();

	virtual Image* getImage();
	virtual int getRepeat() const;
	virtual bool isGradient() const { return false; }

	Gdiplus::Brush* makeBrush() const;
};

class GdiPlusColorBrush : public ColorBrush {
public:
	Color color;
	GdiPlusColorBrush(const Color& _color);
	virtual ~GdiPlusColorBrush();
	virtual bool isGradient() const { return false; }
	virtual Color getColor() const;
};

}
}

#endif /* GDIPLUS_GRAPHICS_H_ */
