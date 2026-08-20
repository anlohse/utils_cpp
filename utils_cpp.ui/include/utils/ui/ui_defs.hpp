/*
 * graph_defs.hpp
 *
 *  Created on: 02/08/2014
 *      Author: Alan
 */

#ifndef GRAPH_DEFS_HPP_
#define GRAPH_DEFS_HPP_

#include <utils/utils_defs.hpp>
#include <utils/funcs.hpp>
#include <utils/math/math.hpp>
#include <atomic>
#include <ostream>
#include <string>

namespace utils {
namespace ui {

struct Size {
	int width, height;
	Size() : width(0), height(0) { }
	Size(int _width, int _height) : width(_width), height(_height) { }
	Size(const Size& s) : width(s.width), height(s.height) { }
	Size& operator = (const Size& s) { width = s.width; height = s.height; return *this; }
	bool operator == (const Size& s) const { return width == s.width && height == s.height; }
	bool operator != (const Size& s) const { return width != s.width || height != s.height; }

	DECLARE_CONSTANT(Size,AUTO);
	DECLARE_CONSTANT(Size,ZERO);

};


struct Point {
	int left, top;
	Point() : left(0), top(0) { }
	Point(int _left, int _top) : left(_left), top(_top) { }
	Point(const Point& s) : left(s.left), top(s.top) { }
	Point& operator = (const Point& s) { left = s.left; top = s.top; return *this; }
	bool operator == (const Point& s) const { return left == s.left && top == s.top; }
	bool operator != (const Point& s) const { return left != s.left || top != s.top; }

	DECLARE_CONSTANT(Point,AUTO);
	DECLARE_CONSTANT(Point,ZERO);
};

struct Rect {
	int left, top, right, bottom;
	Rect() : left(0), top(0), right(0), bottom(0) { }
	Rect(int _left, int _top, int _right, int _bottom) : left(_left), top(_top), right(_right), bottom(_bottom) { }
	Rect(const Rect& s) : left(s.left), top(s.top), right(s.right), bottom(s.bottom) { }
	Rect& operator = (const Rect& s) { left = s.left; top = s.top; right = s.right; bottom = s.bottom; return *this; }
	bool operator == (const Rect& s) const { return left == s.left && top == s.top && right == s.right && bottom == s.bottom; }
	bool operator != (const Rect& s) const { return left != s.left || top != s.top || right != s.right || bottom != s.bottom; }
};

class Color {
private:
	t_dword value;
public:
	static t_dword to_rgb(t_byte r, t_byte g, t_byte b) {
		return 0xff000000 | (b << 16) | (g << 8) | (r);
	}

	static t_dword to_rgb(t_byte r, t_byte g, t_byte b, t_byte a) {
		return (a << 24) | (b << 16) | (g << 8) | (r);
	}

	static t_dword to_rgb(float r, float g, float b) {
		return 0xff000000 | (((t_byte)(b*255)) << 16) | (((t_byte)(g*255)) << 8) | ((t_byte)(r*255));
	}

	static t_dword to_rgb(float r, float g, float b, float a) {
		return (((t_byte)(a*255)) << 24) | (((t_byte)(b*255)) << 16) | (((t_byte)(g*255)) << 8) | ((t_byte)(r*255));
	}

	static t_byte to_red(t_dword rgb) {
		return rgb & 0xff;
	}

	static t_byte to_green(t_dword rgb) {
		return (rgb >> 8) & 0xff;
	}

	static t_byte to_blue(t_dword rgb) {
		return (rgb >> 16) & 0xff;
	}

	static t_byte to_alpha(t_dword rgb) {
		return (rgb >> 24) & 0xff;
	}

	static float* rgb_to_hsv(t_byte r, t_byte g, t_byte b, float* hsv) {
		t_byte max = math::Math::max(r,g,b);
		t_byte min = math::Math::min(r,g,b);
		float h = 0, s = 0, v = 0;
		float* nhsv = hsv;
		if (max > min) {
			if (max == r) {
				if (g >= b)
					h = 60.0f * (g-b)/(max-min);
				else
					h = 60.0f * (g-b)/(max-min) + 360;
			} else if (max == g)
				h = 60.0f * (b-r)/(max-min) + 120;
			else
				h = 60.0f * (r-g)/(max-min) + 240;

			s = 1.0f * (max - min)/max;
		}
		v = max / 255.0f;
		*(nhsv++) = h;
		*(nhsv++) = s;
		*nhsv = v;
		return hsv;
	}

	static t_dword hsv_to_rgb(float h, float s, float v) {
		int hi = math::Math::abs((int)h/60) % 6;
		float f = h/60.0f - hi;
		float p = v*(1 - s);
		float q = v*(1 - f*s);
		float t = v*(1 - (1-f)*s);
		switch (hi) {
			case 0: return to_rgb(v,t,p);
			case 1: return to_rgb(q,v,p);
			case 2: return to_rgb(p,v,t);
			case 3: return to_rgb(p,q,v);
			case 4: return to_rgb(t,p,v);
			case 5: return to_rgb(v,p,q);
		}
		return to_rgb((t_byte)0,0,0);
	}

	static t_dword cmyk_to_rgb(float c, float m, float y, float k)  {
		t_byte r = 255 * (1-c) * (1-k);
		t_byte g = 255 * (1-m) * (1-k);
		t_byte b = 255 * (1-y) * (1-k);
		return to_rgb(r,g,b);
	}

	static float* rgb_to_cmyk(float r, float g, float b, float* cmyk) {
		float k = 1-math::Math::max(r, g, b);
		float c = (1-r-k) / (1-k);
		float m = (1-g-k) / (1-k);
		float y = (1-b-k) / (1-k);
		float* ncmyk = cmyk;
		*(ncmyk++) = c;
		*(ncmyk++) = m;
		*(ncmyk++) = y;
		*ncmyk = k;
		return cmyk;
	}

	static float* rgb_to_cmyk(t_byte r, t_byte g, t_byte b, float* cmyk) {
		return rgb_to_cmyk(r/255.0f,g/255.0f,b/255.0f,cmyk);
	}

	static t_dword parse_color(const char* c);

public:
	Color(): value(0) {}
	Color(t_dword argb): value(argb) {}
	Color(t_byte r, t_byte g, t_byte b): value(to_rgb(r,g,b)) {}
	Color(t_byte r, t_byte g, t_byte b, t_byte a): value(to_rgb(r,g,b,a)) {}
	Color(const Color& c): value(c.value) {}
	Color(const char* c): value(parse_color(c)) {}

	Color& operator = (const Color& c) { value = c.value; return *this; }
	bool operator == (const Color& c) const { return value == c.value; }
	bool operator != (const Color& c) const { return value != c.value; }

	Color& operator += (const Color& c) {
		int r = get_red();
		int g = get_green();
		int b = get_blue();
		int a = get_alpha();
		r = math::Math::min(r+c.get_red(),255);
		g = math::Math::min(g+c.get_green(),255);
		b = math::Math::min(b+c.get_blue(),255);
		a = math::Math::min(a+c.get_alpha(),255);
		value = to_rgb((t_byte)r,g,b,a);
		return *this;
	}

	Color& operator -= (const Color& c) {
		int r = get_red();
		int g = get_green();
		int b = get_blue();
		int a = get_alpha();
		r = math::Math::max(r-c.get_red(),0);
		g = math::Math::max(g-c.get_green(),0);
		b = math::Math::max(b-c.get_blue(),0);
		a = math::Math::max(a-c.get_alpha(),0);
		value = to_rgb((t_byte)r,g,b,a);
		return *this;
	}

	Color& operator |= (const Color& c) {
		value |= c.value;
		return *this;
	}

	Color& operator &= (const Color& c) {
		value &= c.value;
		return *this;
	}

	Color& operator ^= (const Color& c) {
		value ^= c.value;
		return *this;
	}

	t_byte get_red() const {
		return to_red(value);
	}

	t_byte get_green() const {
		return to_green(value);
	}

	t_byte get_blue() const {
		return to_blue(value);
	}

	t_byte get_alpha() const {
		return to_alpha(value);
	}

	t_dword get_rgb() const {
		return value;
	}

	std::string to_string() const {
		std::string s = "rgba(";
		char buf[8];
		s.append(itoa(get_red(),buf,10));
		s.append(",");
		s.append(itoa(get_green(),buf,10));
		s.append(",");
		s.append(itoa(get_blue(),buf,10));
		s.append(",");
		s.append(itoa(get_alpha(),buf,10));
		s.append(")");
		return s;
	}

public:
	static const Color WHITE;
	static const Color BLACK;
	static const Color SILVER;
	static const Color GRAY;
	static const Color LIME;
	static const Color GREEN;
	static const Color CYAN;
	static const Color DARK_CYAN;
	static const Color BLUE;
	static const Color DARK_BLUE;
	static const Color MAGENTA;
	static const Color DARK_MAGENTA;
	static const Color RED;
	static const Color MAROON;
	static const Color YELLOW;
	static const Color OLIVE;
};

inline Color operator + (const Color& c1, const Color& c2) {
	Color r = c1;
	r += c2;
	return r;
}

inline Color operator - (const Color& c1, const Color& c2) {
	Color r = c1;
	r -= c2;
	return r;
}


inline Color operator | (const Color& c1, const Color& c2) {
	Color r = c1;
	r |= c2;
	return r;
}

inline Color operator & (const Color& c1, const Color& c2) {
	Color r = c1;
	r &= c2;
	return r;
}

inline Color operator ^ (const Color& c1, const Color& c2) {
	Color r = c1;
	r ^= c2;
	return r;
}

inline std::ostream& operator<<(std::ostream& os, const Color& c) {
	os << c.to_string();
	return os;
}


class CompositeOperation {
	MAKE_ENUMERATION(CompositeOperation)
	DECLARE_CONSTANT(CompositeOperation, SOURCE_COPY);
	DECLARE_CONSTANT(CompositeOperation, SOURCE_AND);
	DECLARE_CONSTANT(CompositeOperation, SOURCE_OR);
	DECLARE_CONSTANT(CompositeOperation, SOURCE_XOR);
	DECLARE_CONSTANT(CompositeOperation, DEST_INVERT);
	DECLARE_CONSTANT(CompositeOperation, WHITE);
	DECLARE_CONSTANT(CompositeOperation, BLACK);
};

class LineStyle {
	MAKE_ENUMERATION(LineStyle)
	DECLARE_CONSTANT(LineStyle,SOLID);
	DECLARE_CONSTANT(LineStyle,DASH);
	DECLARE_CONSTANT(LineStyle,DOT);
	DECLARE_CONSTANT(LineStyle,DASHDOT);
	DECLARE_CONSTANT(LineStyle,DASHDOTDOT);
	DECLARE_CONSTANT(LineStyle,NULL_LINE);
};

class LineJoin {
	MAKE_ENUMERATION(LineJoin)
	DECLARE_CONSTANT(LineJoin,BEVEL);
	DECLARE_CONSTANT(LineJoin,MITER);
	DECLARE_CONSTANT(LineJoin,ROUND);
};

class LineCap {
	MAKE_ENUMERATION(LineCap)
	DECLARE_CONSTANT(LineCap,ROUND);
	DECLARE_CONSTANT(LineCap,SQUARE);
	DECLARE_CONSTANT(LineCap,FLAT);
};

struct TextMetrics {
	int	ascent;
	int	descent;
	int	height;
	int	leading;
};

/**
 * User Interface base object
 */
class UIObject {
private:
	// std::atomic, not volatile int: volatile provides no atomicity, so the
	// ++/-- below were a data race whenever a UI object was referenced from
	// more than one thread. volatile has never meant "thread-safe" in C++.
	std::atomic<int> _M_references;
public:
	UIObject() : _M_references(0) { }
	virtual ~UIObject() { }

	// std::atomic is not copyable, but UIObject still needs to be. A copy is a
	// distinct object, so it starts with its own zero reference count rather
	// than inheriting the source's -- copying the count would over-retain the
	// new object and leak it.
	UIObject(const UIObject&) : _M_references(0) { }
	UIObject& operator = (const UIObject&) { return *this; }

	UIObject* add_reference() {
		_M_references.fetch_add(1, std::memory_order_relaxed);
		return this;
	}

	UIObject* rem_reference() {
		// acq_rel so that the decrement that reaches zero happens-after every
		// other thread's use of the object.
		_M_references.fetch_sub(1, std::memory_order_acq_rel);
		return this;
	}

	int get_references() const {
		return _M_references.load(std::memory_order_acquire);
	}

	static void* operator new(size_t nbytes);
	static void operator delete(void* p, size_t nbytes);

};

} // ui
} // utils

#endif /* GRAPH_DEFS_HPP_ */
