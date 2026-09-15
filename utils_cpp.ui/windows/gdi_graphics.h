/*
 * gdi_graphics.h
 *
 *  Created on: 07/08/2014
 *      Author: Alan
 */

#ifndef GDI_GRAPHICS_H_
#define GDI_GRAPHICS_H_

#include <utils/ui/graphics.hpp>
#include <utils/ui/ref.hpp>
#include <utils/math/geometry/matrix.hpp>
#include <windows.h>
#include <vector>

namespace utils {
namespace ui {

typedef math::matrix<float, 3,3> WorldMatrix;
typedef std::vector<WorldMatrix,ui_allocator<WorldMatrix> > MatrixStack;

class GdiFont;

class GdiGraphics : public Graphics {
public:
	HDC hdc;
	bool compatible;
	WorldMatrix world_matrix;
	MatrixStack matrix_stack;
	// Ref<Font>, not Ref<GdiFont>: GdiFont is merely forward-declared here,
	// and RefCounted cannot be checked against an incomplete type.
	Ref<Font> currentFont;
	Ref<FillStyle> currentPattern;
	Ref<Stroke> currentStroke;
	Color lineColor, fillColor, textColor;
	CompositeOperation globalCompositeOperation;
	float alpha;
	bool pathOpen;
	bool imageSmoothing;
	void commit_world_matrix();
	HPEN createCurrentPen();
	HGDIOBJ selectFillBrush(HBRUSH* owned);
public:
	GdiGraphics(HDC _hdc, bool _compatible);
	virtual ~GdiGraphics();
	// state
	virtual void save(); // push state on state stack
	virtual void restore(); // pop state stack and restore state

	// transformations (default transform is the identity matrix)
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

	static GdiGraphics* createGraphics(HDC _hdc, bool _compatible);
	static void deleteGraphics(GdiGraphics* _graphics);

};

class GdiObject {
public:
	virtual ~GdiObject() { }
	virtual HGDIOBJ getHandle() const = 0;
};

class GdiStroke : public Stroke, GdiObject {
private:
	friend class GdiGraphics;
	HPEN hpen;
	bool deletable;
	DWORD style;
	float width;
public:
	GdiStroke(HPEN _hpen);
	GdiStroke(const LineStyle& _style, const LineJoin& _join, const LineCap& _cap, float _width, const Color& color);
	virtual ~GdiStroke();

	virtual LineStyle getStyle() const;
	virtual LineJoin getJoin() const;
	virtual LineCap getCap() const;
	virtual float getWidth() const;

	virtual HGDIOBJ getHandle() const {
		return hpen;
	}
};


class GdiFont : public Font, GdiObject {
private:
	friend class GdiGraphics;
	HFONT hfont;
	char fontface[32];
	int size, style;
	bool deletable;
public:
	GdiFont(const char* facename, int _size, int _style);
	GdiFont(HFONT _hfont);
	virtual ~GdiFont();

	virtual const char* getFace() const;
	virtual int getSize() const;
	virtual int getStyle() const;

	static GdiFont* fromHDC(HDC hdc);
	static GdiFont* createFontImpl(const char* facename, int style, int size);
	static void destroyFontImpl(GdiFont* obj);

	virtual HGDIOBJ getHandle() const {
		return hfont;
	}
};

class GdiLinearGradient : public Gradient, GdiObject {
private:

public:

	virtual ~GdiLinearGradient();

	virtual void addColor(float x, float y, const Color& c);
	virtual int getSize() const;
	virtual int getDirection() const;

	virtual bool isGradient() const { return true; }


	virtual HGDIOBJ getHandle() const {
		return NULL;
	}
};

class GdiPattern : public Pattern, GdiObject {
private:
	friend class GdiGraphics;
	HBRUSH hbrush;
	Image* image;
	bool deletale;
	int repeat;
public:
	GdiPattern(HBRUSH _hbrush);
	GdiPattern(Image* _image, int _repeat);
	virtual ~GdiPattern();

	virtual Image* getImage();

	virtual int getRepeat() const;

	virtual bool isGradient() const { return false; }


	virtual HGDIOBJ getHandle() const {
		return hbrush;
	}
};

class GdiColorBrush : public ColorBrush, public GdiObject {
public:
	HBRUSH hbrush;
	bool deletable;
	GdiColorBrush(const Color& color);
	GdiColorBrush(HBRUSH _hbrush, bool _deletable);
	virtual ~GdiColorBrush();

	virtual bool isGradient() const {
		return false;
	}

	virtual Color getColor() const;


	virtual HGDIOBJ getHandle() const {
		return hbrush;
	}

};

}

}

#endif /* GDI_GRAPHICS_H_ */
