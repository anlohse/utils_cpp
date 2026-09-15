/*
 * graph_defs.cpp
 *
 *  Created on: 02/08/2014
 *      Author: Alan
 */

#include <utils/ui/ui_defs.hpp>
#include <utils/ui/graphics.hpp>
#include <iostream>
#include <errno.h>
#include <cctype>

namespace utils {
namespace ui {

INIT_CONSTANT(Size,AUTO,Size((int)0x80000000, (int)0x80000000));
INIT_CONSTANT(Size,ZERO,Size(0,0));
INIT_CONSTANT(Point,AUTO,Point((int)0x80000000, (int)0x80000000));
INIT_CONSTANT(Point,ZERO,Point(0, 0));

const char* runspaces(const char* str) {
	while (isspace(*str)) str++;
	return str;
}

int readnum(const char* str, const char** end) {
	str = runspaces(str);
	if (*str == 0)
		throw math::math_exception("Expecting number.");
	if (!isdigit(*str))
		throw math::math_exception("Expecting number.");
	const char* st = str;
	while (isdigit(*str)) str++;
	char buf[16];
	strncpy(buf,st,str-st);
	if (end) *end = str;
	return strtol(buf,NULL,10);
}

t_dword Color::parse_color(const char* c) {
	if (*c == '#') {
		t_dword l = strtoul(c+1,NULL,16);
		if (errno == EINVAL)
			throw math::math_exception("Invalid hex number.");
		if ((l & 0xff000000) != 0)
			return l;
		return 0xff000000 | l;
	} else if (strncmp(c,"rgba(",5) == 0) {
		const char* end = c;
		t_byte r = readnum(c+5,&end);
		c = end;
		c = runspaces(c);
		if (*c != ',')
			throw math::math_exception("Invalid color format.");
		c++;
		t_byte g = readnum(c,&end);
		c = end;
		c = runspaces(c);
		if (*c != ',')
			throw math::math_exception("Invalid color format.");
		c++;
		t_byte b = readnum(c,&end);
		c = end;
		c = runspaces(c);
		if (*c != ',')
			throw math::math_exception("Invalid color format.");
		c++;
		t_byte a = readnum(c,&end);
		c = end;
		c = runspaces(c);
		if (*c != ')')
			throw math::math_exception("Invalid color format.");
		c++;
		c = runspaces(c);
		if (*c != 0)
			throw math::math_exception("Invalid color format.");
		return to_rgb(r,g,b,a);
	} else if (strncmp(c,"rgb(",4) == 0) {
		const char* end = c;
		t_byte r = readnum(c+4,&end);
		c = end;
		c = runspaces(c);
		if (*c != ',')
			throw math::math_exception("Invalid color format.");
		c++;
		t_byte g = readnum(c,&end);
		c = end;
		c = runspaces(c);
		if (*c != ',')
			throw math::math_exception("Invalid color format.");
		c++;
		t_byte b = readnum(c,&end);
		c = end;
		c = runspaces(c);
		if (*c != ')')
			throw math::math_exception("Invalid color format.");
		c++;
		c = runspaces(c);
		if (*c != 0)
			throw math::math_exception("Invalid color format.");
		return to_rgb(r,g,b);
	}
	throw math::math_exception("Invalid color format.");
}

const Color Color::WHITE = Color(0xffffffff);
const Color Color::BLACK = Color(0xff000000);
const Color Color::SILVER = Color(0xffc0c0c0);
const Color Color::GRAY = Color(0xff808080);
const Color Color::LIME = Color(0xff00ff00);
const Color Color::GREEN = Color(0xff008000);
const Color Color::CYAN = Color(0xffffff00);
const Color Color::DARK_CYAN = Color(0xff808000);
const Color Color::BLUE = Color(0xffff0000);
const Color Color::DARK_BLUE = Color(0xff800000);
const Color Color::MAGENTA = Color(0xffff00ff);
const Color Color::DARK_MAGENTA = Color(0xff800080);
const Color Color::RED = Color(0xff0000ff);
const Color Color::MAROON = Color(0xff000080);
const Color Color::YELLOW = Color(0xff00ffff);
const Color Color::OLIVE = Color(0xff008080);

INIT_CONSTANT(CompositeOperation, SOURCE_COPY,CompositeOperation(1));
INIT_CONSTANT(CompositeOperation, SOURCE_AND,CompositeOperation(2));
INIT_CONSTANT(CompositeOperation, SOURCE_OR,CompositeOperation(3));
INIT_CONSTANT(CompositeOperation, SOURCE_XOR,CompositeOperation(4));
INIT_CONSTANT(CompositeOperation, DEST_INVERT,CompositeOperation(5));
INIT_CONSTANT(CompositeOperation, WHITE,CompositeOperation(6));
INIT_CONSTANT(CompositeOperation, BLACK,CompositeOperation(7));

INIT_CONSTANT(LineStyle,SOLID,LineStyle(1));
INIT_CONSTANT(LineStyle,DASH,LineStyle(2));
INIT_CONSTANT(LineStyle,DOT,LineStyle(3));
INIT_CONSTANT(LineStyle,DASHDOT,LineStyle(4));
INIT_CONSTANT(LineStyle,DASHDOTDOT,LineStyle(5));
INIT_CONSTANT(LineStyle,NULL_LINE,LineStyle(5));

INIT_CONSTANT(LineJoin,BEVEL,LineJoin(1));
INIT_CONSTANT(LineJoin,MITER,LineJoin(2));
INIT_CONSTANT(LineJoin,ROUND,LineJoin(3));

INIT_CONSTANT(LineCap,ROUND,LineCap(1));
INIT_CONSTANT(LineCap,SQUARE,LineCap(2));
INIT_CONSTANT(LineCap,FLAT,LineCap(3));

}// ui
}// utils

using namespace utils;
using namespace ui;

// A scratch test_graphics() lived here: global scope, no callers, printing to
// stdout from inside a UI library. Its colour-conversion checks now live in
// utils_cpp.ui/test_src/test_graphics.cpp as an actual test.

