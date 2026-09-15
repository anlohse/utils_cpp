/*
 * demo_window.cpp
 *
 * An interactive showcase of the window system: a Window laid out by
 * BasicLayout into a header, sidebar, canvas and status bar, with all four
 * regions custom-painted through the Canvas-style Graphics API.
 *
 * Run:  utils_ui_tests --window
 *
 * What it demonstrates:
 *   - Window creation, titling and close handling
 *   - BasicLayout reflowing every region as the window is resized
 *   - Paint listeners drawing with paths, gradients, transforms and text
 *   - Antialiasing toggled live, so the difference is visible side by side
 *   - Mouse and command events driving repaints
 */

#include <utils/ui/window.hpp>
#include <utils/ui/container.hpp>
#include <utils/ui/button.hpp>
#include <utils/ui/layout.hpp>
#include <utils/ui/event.hpp>
#include <utils/ui/graphics.hpp>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>

using namespace utils;
using namespace utils::ui;

namespace {

const float PI_F = 3.14159265358979323846f;

/** Everything the demo needs to share between its listeners. */
struct DemoState {
	bool  antialias;
	int   mouseX;
	int   mouseY;
	int   clicks;
	Component* canvas;
	Component* status;
	DemoState() : antialias(true), mouseX(-1), mouseY(-1), clicks(0),
	              canvas(NULL), status(NULL) { }
};

DemoState state;

// ---------------------------------------------------------------------------
// Header: a gradient bar with a title
// ---------------------------------------------------------------------------

struct HeaderPainter : EventListener {
	virtual void operator() (Event* evt) {
		PaintEvent* pe = dynamic_cast<PaintEvent*>(evt);
		if (pe == NULL) return;
		Graphics* g = pe->graphics();
		const Size sz = static_cast<Component*>(pe->source())->getSize();

		// A vertical gradient across the whole bar.
		Gradient* grad = g->createLinearGradient(0, 0, 0, (float) sz.height);
		grad->addColor(0.0f, 0.0f, Color(0x30, 0x3a, 0x4a));
		grad->addColor(1.0f, 0.0f, Color(0x1b, 0x22, 0x2e));
		g->setFillStyle(grad);
		g->fillRect(0, 0, (float) sz.width, (float) sz.height);
		g->setFillStyle(NULL);

		// A hairline along the bottom edge. Antialiasing is switched off for
		// it deliberately: a 1px rule wants to land on the pixel grid, which
		// is exactly why setAntialias is opt-in rather than always on.
		const bool wasAA = g->getAntialias();
		g->setAntialias(false);
		g->setFillColor(Color(0x4a, 0x90, 0xd9));
		g->fillRect(0, (float) (sz.height - 1), (float) sz.width, 1);
		g->setAntialias(wasAA);

		g->setTextColor(Color::WHITE);
		g->fillText("utils_cpp.ui  --  Canvas-style drawing on Win32", 14, 12, 0);
	}
};

/**
 * Fills the sidebar.
 *
 * Components do not reliably paint their own background: the default one is
 * a GdiColorBrush wrapping (HBRUSH)(COLOR_BTNFACE+1), which is a system
 * colour *constant* only meaningful in WNDCLASS.hbrBackground, not a real
 * brush handle. Painting the panel explicitly is what an application would
 * do anyway.
 */
struct SidebarPainter : EventListener {
	virtual void operator() (Event* evt) {
		PaintEvent* pe = dynamic_cast<PaintEvent*>(evt);
		if (pe == NULL) return;
		Graphics* g = pe->graphics();
		const Size sz = pe->source()->getSize();
		g->setFillColor(Color(0x2a, 0x31, 0x3e));
		g->fillRect(0, 0, (float) sz.width, (float) sz.height);

		const bool wasAA = g->getAntialias();
		g->setAntialias(false);
		g->setFillColor(Color(0x1b, 0x22, 0x2e));
		g->fillRect((float) (sz.width - 1), 0, 1, (float) sz.height);
		g->setAntialias(wasAA);

		g->setTextColor(Color(0x8b, 0x98, 0xa8));
		g->fillText("controls", 14, 8, 0);
	}
};

// ---------------------------------------------------------------------------
// Canvas: the scene that shows off the drawing API
// ---------------------------------------------------------------------------

struct CanvasPainter : EventListener {
	virtual void operator() (Event* evt) {
		PaintEvent* pe = dynamic_cast<PaintEvent*>(evt);
		if (pe == NULL) return;
		Graphics* g = pe->graphics();
		const Size sz = static_cast<Component*>(pe->source())->getSize();
		if (sz.width <= 0 || sz.height <= 0) return;

		g->setAntialias(state.antialias);

		g->setFillColor(Color(0xf7, 0xf7, 0xfa));
		g->fillRect(0, 0, (float) sz.width, (float) sz.height);

		const float w = (float) sz.width;
		const float h = (float) sz.height;

		// A fan of lines from one corner: the classic aliasing test.
		Stroke* thin = g->createStroke(LineStyle::SOLID, LineJoin::MITER, LineCap::FLAT, 1);
		g->setStroke(thin);
		g->setLineColor(Color(0xc0, 0xc8, 0xd4));
		for (int i = 0; i <= 12; ++i) {
			const float t = (float) i / 12.0f;
			g->beginPath();
			g->moveTo(10.0f, 10.0f);
			g->lineTo(10.0f + (w - 20.0f) * t, h - 10.0f);
			g->stroke();
		}

		// A filled bezier blob.
		Stroke* fat = g->createStroke(LineStyle::SOLID, LineJoin::ROUND, LineCap::ROUND, 3);
		g->setStroke(fat);
		g->setFillColor(Color(0x4a, 0x90, 0xd9, 0xb0));
		g->setLineColor(Color(0x1b, 0x4a, 0x7a));
		g->beginPath();
		g->moveTo(w * 0.15f, h * 0.65f);
		g->bezierCurveTo(w * 0.28f, h * 0.15f, w * 0.52f, h * 0.95f, w * 0.66f, h * 0.42f);
		g->bezierCurveTo(w * 0.72f, h * 0.20f, w * 0.40f, h * 0.18f, w * 0.15f, h * 0.65f);
		g->closePath();
		g->strokeAndFill();

		// A ring of circles, to show arc() and the transform stack.
		const float cx = w * 0.78f;
		const float cy = h * 0.40f;
		const float ringR = (w < h ? w : h) * 0.16f;
		for (int i = 0; i < 8; ++i) {
			const float a = (float) i / 8.0f * 2.0f * PI_F;
			g->save();
			g->translate(cx + std::cos(a) * ringR, cy + std::sin(a) * ringR);
			g->setFillColor(Color(0xe8, 0x6a, 0x3a, (t_byte) (80 + i * 20)));
			g->beginPath();
			g->arc(0, 0, ringR * 0.34f, 0, 2.0f * PI_F, false);
			g->fill();
			g->restore();
		}

		// A rotated square, exercising translate + rotate together.
		g->save();
		g->translate(w * 0.30f, h * 0.80f);
		g->rotate(PI_F / 7.0f);
		g->setFillColor(Color(0x5c, 0xb8, 0x5c, 0xcc));
		g->fillRect(-26, -26, 52, 52);
		g->restore();

		// Crosshair following the mouse, drawn unantialiased so it stays sharp.
		if (state.mouseX >= 0) {
			g->setAntialias(false);
			g->setFillColor(Color(0xd0, 0x30, 0x30));
			g->fillRect(0, (float) state.mouseY, w, 1);
			g->fillRect((float) state.mouseX, 0, 1, h);
			g->setAntialias(state.antialias);
		}

		g->setTextColor(Color(0x33, 0x33, 0x33));
		g->fillText(state.antialias ? "antialias: ON  (click Toggle AA)"
		                            : "antialias: OFF (click Toggle AA)",
		            12, h - 26, 0);
	}
};

// ---------------------------------------------------------------------------
// Status bar
// ---------------------------------------------------------------------------

struct StatusPainter : EventListener {
	virtual void operator() (Event* evt) {
		PaintEvent* pe = dynamic_cast<PaintEvent*>(evt);
		if (pe == NULL) return;
		Graphics* g = pe->graphics();
		Component* self = static_cast<Component*>(pe->source());
		const Size sz = self->getSize();

		g->setFillColor(Color(0x22, 0x28, 0x33));
		g->fillRect(0, 0, (float) sz.width, (float) sz.height);

		Size canvasSize(0, 0);
		if (state.canvas != NULL)
			canvasSize = state.canvas->getSize();

		char line[192];
		std::snprintf(line, sizeof line,
				"canvas %dx%d    mouse %d,%d    clicks %d    AA %s",
				canvasSize.width, canvasSize.height,
				state.mouseX, state.mouseY, state.clicks,
				state.antialias ? "on" : "off");

		g->setTextColor(Color(0xc8, 0xd0, 0xdc));
		g->fillText(line, 10, 5, 0);
	}
};

// ---------------------------------------------------------------------------
// Interaction
// ---------------------------------------------------------------------------

struct CanvasMouse : EventListener {
	virtual void operator() (Event* evt) {
		MouseEvent* me = dynamic_cast<MouseEvent*>(evt);
		if (me == NULL) return;
		if (evt->id() == MouseEvent::MOUSE_MOVE) {
			state.mouseX = me->x();
			state.mouseY = me->y();
		} else if (evt->id() == MouseEvent::CLICK) {
			++state.clicks;
		} else {
			return;
		}
		if (state.canvas) state.canvas->repaint();
		if (state.status) state.status->repaint();
	}
};

struct ToggleAAButton : EventListener {
	virtual void operator() (Event* evt) {
		if (evt->id() != CommandEvent::COMMAND) return;
		state.antialias = !state.antialias;
		std::cout << "  antialias -> " << (state.antialias ? "on" : "off") << std::endl;
		if (state.canvas) state.canvas->repaint();
		if (state.status) state.status->repaint();
	}
};

struct QuitButton : EventListener {
	virtual void operator() (Event* evt) {
		if (evt->id() != CommandEvent::COMMAND) return;
		std::cout << "  quit requested" << std::endl;
		Window::quit(0);
	}
};

/** Repaints the whole window whenever it is resized, so bands redraw cleanly. */
struct ResizeRepainter : EventListener {
	virtual void operator() (Event* evt) {
		if (evt->id() != ComponentEvent::RESIZE) return;
		if (state.canvas) state.canvas->repaint();
		if (state.status) state.status->repaint();
	}
};

// Listeners must outlive the window, so they are file-scope rather than local.
HeaderPainter   headerPainter;
SidebarPainter  sidebarPainter;
CanvasPainter   canvasPainter;
StatusPainter   statusPainter;
CanvasMouse     canvasMouse;
ToggleAAButton  toggleAA;
QuitButton      quitButton;
ResizeRepainter resizeRepainter;

} // namespace

void demo_window() {
	std::cout << "--- window system demo ---" << std::endl;

	// GDI+ so the canvas can actually antialias; GDI would ignore the toggle.
	Graphics::setGraphicsImplementation(GRAPHICS_GDIPLUS);

	Window* wnd = new Window(NULL, "utils_cpp.ui demo", 120, 120, 900, 620);
	wnd->setCloseAction(CloseAction::QUIT);
	wnd->setResizable(true);
	wnd->setLayout(new BasicLayout(0, 0));

	// --- header -----------------------------------------------------------
	Component* header = new Component(NULL, 0, 0, 10, 10);
	header->setDefaultSize(Size(0, 44));
	header->addPaintListener(&headerPainter);
	wnd->addChild(header, BasicLayout::TOP);

	// --- sidebar ----------------------------------------------------------
	Container* sidebar = new Container(0, 0, 10, 10);
	sidebar->setDefaultSize(Size(150, 0));
	sidebar->setLayout(new BasicLayout(8, 8));
	sidebar->addPaintListener(&sidebarPainter);
	sidebar->setBorderRect(Rect(10, 32, 10, 10));   // inset the buttons

	Button* aaBtn = new Button(NULL, 0, 0, 130, 30, 1);
	aaBtn->setText("Toggle AA");
	aaBtn->setDefaultSize(Size(0, 30));
	aaBtn->addCommandListener(&toggleAA);
	sidebar->addChild(aaBtn, BasicLayout::TOP);

	Button* quitBtn = new Button(NULL, 0, 0, 130, 30, 2);
	quitBtn->setText("Quit");
	quitBtn->setDefaultSize(Size(0, 30));
	quitBtn->addCommandListener(&quitButton);
	sidebar->addChild(quitBtn, BasicLayout::BOTTOM);

	wnd->addChild(sidebar, BasicLayout::LEFT);

	// --- canvas -----------------------------------------------------------
	Component* canvas = new Component(NULL, 0, 0, 10, 10);
	canvas->addPaintListener(&canvasPainter);
	canvas->addMouseListener(&canvasMouse);
	wnd->addChild(canvas, BasicLayout::CENTER);
	state.canvas = canvas;

	// --- status bar -------------------------------------------------------
	Component* status = new Component(NULL, 0, 0, 10, 10);
	status->setDefaultSize(Size(0, 26));
	status->addPaintListener(&statusPainter);
	wnd->addChild(status, BasicLayout::BOTTOM);
	state.status = status;

	wnd->addComponentListener(&resizeRepainter);

	header->setVisible(true);
	sidebar->setVisible(true);
	aaBtn->setVisible(true);
	quitBtn->setVisible(true);
	canvas->setVisible(true);
	status->setVisible(true);

	wnd->layout();

	std::cout << "  window open -- resize it, move the mouse over the canvas," << std::endl;
	std::cout << "  press Toggle AA, then Quit (or close the window)." << std::endl;

	// Blocking: Window::setVisible runs the message loop until quit.
	wnd->setVisible(true);

	std::cout << "  window closed" << std::endl;
	delete wnd;   // Container::~Container deletes the children it adopted
}
