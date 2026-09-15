/*
 * test_layout.cpp
 *
 * Geometry tests for BasicLayout, the border layout.
 *
 * These create real windows but never show them or pump a message loop:
 * SetWindowPos updates geometry synchronously, so the arrangement can be read
 * straight back with getRect().
 */

#include <utils/ui/container.hpp>
#include <utils/ui/layout.hpp>
#include <utils/ui/component.hpp>
#include <cstdio>
#include <iostream>

using namespace utils;
using namespace utils::ui;

namespace {

int failures = 0;

void check(const char* what, bool ok, const char* detail) {
	std::printf("  %-46s %s  %s\n", what, ok ? "[ok]    " : "[FAIL]  ", detail);
	if (!ok) ++failures;
}

void checkRect(const char* what, const Rect& got, int left, int top, int right, int bottom) {
	char buf[160];
	const bool ok = got.left == left && got.top == top
	             && got.right == right && got.bottom == bottom;
	if (ok)
		std::snprintf(buf, sizeof buf, "(%d,%d)-(%d,%d)", got.left, got.top, got.right, got.bottom);
	else
		std::snprintf(buf, sizeof buf, "expected (%d,%d)-(%d,%d) but got (%d,%d)-(%d,%d)",
				left, top, right, bottom, got.left, got.top, got.right, got.bottom);
	check(what, ok, buf);
}

/** A child with a fixed preferred size along the axis its band cares about. */
Component* makeChild(int prefW, int prefH) {
	Component* c = new Component(NULL, 0, 0, 10, 10);
	c->setDefaultSize(Size(prefW, prefH));
	return c;
}

// ---------------------------------------------------------------------------

void testFullBorderLayout() {
	std::printf("\n BasicLayout: all five regions, no padding\n");

	Container* root = new Container(0, 0, 400, 300);
	root->setLayout(new BasicLayout(0, 0));

	Component* north  = makeChild(0, 40);
	Component* south  = makeChild(0, 30);
	Component* west   = makeChild(50, 0);
	Component* east   = makeChild(60, 0);
	Component* centre = makeChild(0, 0);

	check("addChild TOP",    root->addChild(north,  BasicLayout::TOP),    "accepted");
	check("addChild BOTTOM", root->addChild(south,  BasicLayout::BOTTOM), "accepted");
	check("addChild LEFT",   root->addChild(west,   BasicLayout::LEFT),   "accepted");
	check("addChild RIGHT",  root->addChild(east,   BasicLayout::RIGHT),  "accepted");
	check("addChild CENTER", root->addChild(centre, BasicLayout::CENTER), "accepted");

	root->layout();

	const Size c = root->getSize();
	char note[64];
	std::snprintf(note, sizeof note, "client area is %dx%d", c.width, c.height);
	check("container reports a client area", c.width > 0 && c.height > 0, note);

	// North and south span the full width and keep their preferred height.
	checkRect("TOP spans full width",    north->getRect(), 0, 0, c.width, 40);
	checkRect("BOTTOM sits at the base", south->getRect(), 0, c.height - 30, c.width, c.height);
	// West and east take the height left between them.
	checkRect("LEFT fills middle band",  west->getRect(),  0, 40, 50, c.height - 30);
	checkRect("RIGHT is flush right",    east->getRect(),  c.width - 60, 40, c.width, c.height - 30);
	// The centre gets whatever is left.
	checkRect("CENTER takes the remainder", centre->getRect(),
			50, 40, c.width - 60, c.height - 30);

	delete root;   // Container::~Container deletes its children
}

void testPadding() {
	std::printf("\n BasicLayout: padding inserts gaps between bands\n");

	Container* root = new Container(0, 0, 400, 300);
	root->setLayout(new BasicLayout(8, 6));   // vpadding 8, hpadding 6

	Component* north = makeChild(0, 40);
	Component* west  = makeChild(50, 0);
	Component* centre = makeChild(0, 0);
	root->addChild(north,  BasicLayout::TOP);
	root->addChild(west,   BasicLayout::LEFT);
	root->addChild(centre, BasicLayout::CENTER);

	root->layout();
	const Size c = root->getSize();

	checkRect("TOP unaffected by its own padding", north->getRect(), 0, 0, c.width, 40);
	// vpadding pushes everything below the top band down by 8.
	checkRect("LEFT starts below TOP + vpadding", west->getRect(), 0, 48, 50, c.height);
	// hpadding then pushes the centre right of the left band by 6.
	checkRect("CENTER offset by hpadding", centre->getRect(),
			56, 48, c.width, c.height);

	delete root;
}

void testClampsWhenTooSmall() {
	std::printf("\n BasicLayout: bands clamp when the container is too small\n");

	Container* root = new Container(0, 0, 60, 50);
	root->setLayout(new BasicLayout(0, 0));

	// Both want more height than the container has between them.
	Component* north = makeChild(0, 200);
	Component* south = makeChild(0, 200);
	root->addChild(north, BasicLayout::TOP);
	root->addChild(south, BasicLayout::BOTTOM);

	root->layout();
	const Size c = root->getSize();

	// The top band is clipped to the client height, leaving nothing below it.
	checkRect("TOP clipped to client height", north->getRect(), 0, 0, c.width, c.height);
	checkRect("BOTTOM collapses to zero height", south->getRect(),
			0, c.height, c.width, c.height);

	delete root;
}

void testOneRegionPerConstraint() {
	std::printf("\n BasicLayout: checkConstraints allows one child per region\n");

	Container* root = new Container(0, 0, 200, 200);
	root->setLayout(new BasicLayout(0, 0));

	Component* first  = makeChild(0, 20);
	Component* second = makeChild(0, 20);

	check("first TOP accepted", root->addChild(first, BasicLayout::TOP) == true, "added");
	check("second TOP rejected", root->addChild(second, BasicLayout::TOP) == false,
			"a region holds at most one child");

	delete second;   // never adopted, so the container will not free it
	delete root;
}

void testNestedContainer() {
	std::printf("\n BasicLayout: nested containers are laid out too\n");

	Container* root = new Container(0, 0, 400, 300);
	root->setLayout(new BasicLayout(0, 0));

	Container* inner = new Container(0, 0, 10, 10);
	inner->setLayout(new BasicLayout(0, 0));
	Component* innerTop = makeChild(0, 25);
	inner->addChild(innerTop, BasicLayout::TOP);

	root->addChild(inner, BasicLayout::CENTER);
	root->layout();

	const Size rc = root->getSize();
	checkRect("inner container fills the centre", inner->getRect(), 0, 0, rc.width, rc.height);

	// The nested layout must have run, arranging the inner container's own
	// child across its full width.
	const Size ic = inner->getSize();
	checkRect("inner TOP laid out by nested pass", innerTop->getRect(), 0, 0, ic.width, 25);

	delete root;
}

void testMinimumSize() {
	std::printf("\n BasicLayout: getMinimumSize accumulates the bands\n");

	Container* root = new Container(0, 0, 400, 300);
	BasicLayout* layout = new BasicLayout(0, 0);
	root->setLayout(layout);

	Component* north = makeChild(0, 0);  north->setMinimumSize(Size(120, 40));
	Component* west  = makeChild(0, 0);  west->setMinimumSize(Size(50, 30));
	Component* centre = makeChild(0, 0); centre->setMinimumSize(Size(70, 60));
	root->addChild(north,  BasicLayout::TOP);
	root->addChild(west,   BasicLayout::LEFT);
	root->addChild(centre, BasicLayout::CENTER);

	const Size min = layout->getMinimumSize(root);
	char buf[96];
	// width  = west(50) + centre(70) = 120, floored by north's 120
	// height = north(40) + centre(60) = 100, floored by west's 30
	std::snprintf(buf, sizeof buf, "got %dx%d", min.width, min.height);
	check("minimum size sums bands", min.width == 120 && min.height == 100, buf);

	delete root;
}

} // namespace

int test_layout() {
	std::printf("--- BasicLayout (border layout) ---\n");
	failures = 0;

	testFullBorderLayout();
	testPadding();
	testClampsWhenTooSmall();
	testOneRegionPerConstraint();
	testNestedContainer();
	testMinimumSize();

	std::printf("\n  %d layout failure(s).\n", failures);
	return failures;
}
