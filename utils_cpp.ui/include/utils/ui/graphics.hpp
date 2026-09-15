/*
 * graphics.hpp
 *
 *  Created on: 23/02/2014
 *      Author: Alan
 */

#ifndef GRAPHICS_HPP_
#define GRAPHICS_HPP_

#include <utils/ui/ui_defs.hpp>

namespace utils {
namespace ui {

class Font;
class Image;
class Gradient;
class Pattern;
class Stroke;
class FillStyle;

#ifdef UTILS_WINDOWS
#	define GRAPHICS_GDI 0
#	define GRAPHICS_GDIPLUS 1
#	define DEFAULT_GRAPHICS GRAPHICS_GDI
#endif

class Graphics : public UIObject {
public:
	Graphics(){}
	virtual ~Graphics(){}
	// state
	virtual void save() = 0; // push state on state stack
	virtual void restore() = 0; // pop state stack and restore state

	// transformations (default transform is the identity matrix)
	virtual void scale(float x, float y) = 0;
	virtual void rotate(float angle) = 0;
	virtual void translate(float x, float y) = 0;
	virtual void transform(float a, float b, float c, float d, float e, float f) = 0;
	virtual void setTransform(float a, float b, float c, float d, float e, float f) = 0;

	// colors and styles
	virtual Gradient* createLinearGradient(float x0, float y0, float x1, float y1) = 0;
	virtual Gradient* createRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1) = 0;
	virtual Pattern* createPattern(Image* image, long repetition) = 0;
	virtual Stroke* createStroke(const LineStyle& style, const LineJoin& join, const LineCap& cap, float width) = 0;

	virtual float getGlobalAlpha() = 0;
	virtual void setGlobalAlpha(float value) = 0;

	virtual CompositeOperation getGlobalCompositeOperation() = 0;
	virtual void setGlobalCompositeOperation(const CompositeOperation& go) = 0;

	// rendering quality
	//
	// Canvas 2D always antialiases geometry and offers no way to switch it
	// off. A native widget toolkit does need that switch -- a 1px border or
	// separator has to land on exact pixels rather than smear across two --
	// so antialiasing here is opt-in per Graphics, and is part of the save()
	// / restore() state like every other drawing attribute.
	//
	// Image smoothing is a separate knob, matching Canvas's
	// imageSmoothingEnabled: it controls the filter used when drawImage
	// scales, and is independent of whether path edges are antialiased.
	//
	// Not every backend can honour setAntialias(true). GDI has no
	// coverage-based rasteriser at all, so its implementation reports false
	// however it is set; check getAntialias() after setting it when the
	// distinction matters to the caller.
	virtual bool getAntialias() = 0;
	virtual void setAntialias(bool value) = 0;

	virtual bool getImageSmoothing() = 0;
	virtual void setImageSmoothing(bool value) = 0;

	virtual Color getLineColor() = 0;
	virtual void setLineColor(const Color& value) = 0;

	virtual Color getFillColor() = 0;
	virtual void setFillColor(const Color& value) = 0;

	virtual Color getTextColor() = 0;
	virtual void setTextColor(const Color& value) = 0;

	virtual FillStyle* getFillStyle() = 0;
	virtual void setFillStyle(FillStyle* value) = 0;

	virtual Stroke* getStroke() = 0;
	virtual void setStroke(Stroke* value) = 0;


	// rects
	virtual void clearRect(float x, float y, float w, float h) = 0;
	virtual void fillRect(float x, float y, float w, float h) = 0;
	virtual void strokeRect(float x, float y, float w, float h) = 0;

	// paths
	virtual void beginPath() = 0;
	virtual void closePath() = 0;
	virtual void fill() = 0;
	virtual void stroke() = 0;
	virtual void strokeAndFill() = 0;
	virtual void clip() = 0;

	virtual void moveTo(float x, float y) = 0;
	virtual void lineTo(float x, float y) = 0;
	virtual void quadraticCurveTo(float cpx, float cpy, float x, float y) = 0;
	virtual void bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y) = 0;
	virtual void arcTo(float x1, float y1, float x2, float y2, float radius) = 0;
	virtual void rect(float x, float y, float w, float h) = 0;
	virtual void arc(float x, float y, float radius, float startAngle, float endAngle, bool anticlockwise = false) = 0;

	// text
	virtual Font* getFont() = 0;
	virtual void setFont(Font* font) = 0;
	virtual void fillText(const char* text, float x, float y, float maxWidth) = 0;
	virtual void strokeText(const char* text, float x, float y, float maxWidth) = 0;
	virtual void fillText(const wchar_t* text, float x, float y, float maxWidth) = 0;
	virtual void strokeText(const wchar_t* text, float x, float y, float maxWidth) = 0;
	virtual float measureText(const char* text) = 0;
	virtual float measureText(const wchar_t* text) = 0;

	// drawing images
	virtual void drawImage(Image* image, float dx, float dy) = 0;
	virtual void drawImage(Image* image, float dx, float dy, float dw, float dh) = 0;
	virtual void drawImage(Image* image, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) = 0;

	// pixel manipulation
	virtual Image* createImage(float sw, float sh) = 0;
	virtual Image* createImage(Image* imagedata) = 0;
	virtual Image* getImage(float sx, float sy, float sw, float sh) = 0;

public:
	static void setGraphicsImplementation(int graphicsImpl);
	static int getGraphicsImplementation();
};


class Stroke : public UIObject {
public:
	Stroke() { }
	virtual ~Stroke() { }

	virtual LineStyle getStyle() const = 0;
	virtual LineJoin getJoin() const = 0;
	virtual LineCap getCap() const = 0;
	virtual float getWidth() const = 0;

};

class Font : public UIObject {
public:
	Font() { }
	virtual ~Font() { }

	virtual const char* getFace() const = 0;
	virtual int getSize() const = 0;
	virtual int getStyle() const = 0;

	MAKE_CONSTANT(int,PLAIN,0);
	MAKE_CONSTANT(int,BOLD,1);
	MAKE_CONSTANT(int,ITALIC,2);
	MAKE_CONSTANT(int,UNDERLINE,4);
	MAKE_CONSTANT(int,STRIKE_OUT,8);

	static Font* createFont(const char* facename, int style, int size);
	static void destroyFont(Font* obj);
};


class FillStyle : public UIObject {
public:
	virtual ~FillStyle() { }

	virtual bool isGradient() const = 0;
};

class ColorBrush : public FillStyle {
public:
	ColorBrush() { }
	virtual ~ColorBrush() { }

	virtual Color getColor() const = 0;

};

class Gradient : public FillStyle {
public:
	Gradient() { }
	virtual ~Gradient() { }

	virtual void addColor(float x, float y, const Color& c) = 0;
	virtual int getSize() const = 0;
	virtual int getDirection() const = 0;

	MAKE_CONSTANT(int,HORIZONTAL,0);
	MAKE_CONSTANT(int,VERTICAL,1);
	MAKE_CONSTANT(int,TRIANGLE,2);
};

class Pattern : public FillStyle {
public:
	Pattern() { }
	virtual ~Pattern() { }

	virtual Image* getImage() = 0;

	virtual int getRepeat() const = 0;

	MAKE_CONSTANT(int,NO_REPEAT,0);
	MAKE_CONSTANT(int,REPEAT_X,1);
	MAKE_CONSTANT(int,REPEAT_Y,2);
	MAKE_CONSTANT(int,REPEAT,3);
};

}
}

#endif /* GRAPHICS_HPP_ */
