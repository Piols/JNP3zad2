#include <d2d1_3.h>
#include "D2DApp.h"
#include <vector>

namespace {
	// Deklaracje u¿ycia pomocniczych funkcji
	using D2D1::RenderTargetProperties;
	using D2D1::HwndRenderTargetProperties;
	using D2D1::SizeU;
	using D2D1::Point2F;
	using D2D1::BezierSegment;
	using D2D1::QuadraticBezierSegment;
	using D2D1::Matrix3x2F;

	struct CubicBezier {
		D2D1_POINT_2F control1;
		D2D1_POINT_2F control2;
		D2D1_POINT_2F target;
	};

	struct Path {
		D2D1_POINT_2F start;
		std::vector<CubicBezier> segments;
	};

	// Interfejsy potrzebne do zainicjowania Direct2D
	ID2D1Factory7* d2d_factory = nullptr;
	ID2D1HwndRenderTarget* d2d_render_target = nullptr;

	ID2D1SolidColorBrush* brush = nullptr;
	D2D1_COLOR_F const clear_color =
	{ .r = 0.75f, .g = 0.75f, .b = 1.0f, .a = 1.0f };
	D2D1_COLOR_F const color_light_green =
	{ .r = 0.75f, .g = 1.0f, .b = 0.75f, .a = 1.0f };
	D2D1_COLOR_F const color_green =
	{ .r = 0.1f, .g = 0.8f, .b = 0.1f, .a = 1.0f };
	D2D1_COLOR_F const color_dark_green =
	{ .r = 0.0f, .g = 0.45f, .b = 0.0f, .a = 1.0f };
	D2D1_COLOR_F const color_gray =
	{ .r = 0.25f, .g = 0.25f, .b = 0.25f, .a = 1.0f };
	D2D1_COLOR_F const color_white =
	{ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f };

	ID2D1RadialGradientBrush* rad_brush_1 = nullptr;
	ID2D1GradientStopCollection* rad_stops_1 = nullptr;
	UINT const NUM_RAD_STOPS_1 = 2;
	D2D1_GRADIENT_STOP rad_stops_data_1[NUM_RAD_STOPS_1];

	ID2D1RadialGradientBrush* rad_brush_2 = nullptr;
	ID2D1GradientStopCollection* rad_stops_2 = nullptr;
	UINT const NUM_RAD_STOPS_2 = 2;
	D2D1_GRADIENT_STOP rad_stops_data_2[NUM_RAD_STOPS_2];

	ID2D1PathGeometry* path = nullptr;
	ID2D1GeometrySink* path_sink = nullptr;

	Path path_data;
}

void InitDirect2D(HWND hwnd) {
	// Utworzenie fabryki Direct2D
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
	if (d2d_factory == NULL) {
		exit(2);
	}
	RecreateRenderTarget(hwnd);

	rad_stops_data_1[0] = { .position = 0.0f, .color = color_light_green };
	rad_stops_data_1[1] = { .position = 0.5f, .color = color_green };
	rad_stops_data_1[2] = { .position = 1.0f, .color = color_dark_green };

	rad_stops_data_2[0] = { .position = 0.0f, .color = color_white };
	rad_stops_data_2[1] = { .position = 1.0f, .color = color_gray };

	path_data.start = Point2F(0.0f, -0.8f);
	path_data.segments.push_back({
		Point2F(-0.8f, -0.8f),
		Point2F(-1.2f, -0.8f),
		Point2F(-0.8f, -0.4f)
	});
	path_data.segments.push_back({
		Point2F(-1.2f, 1.0f),
		Point2F(-0.6f, 1.0f),
		Point2F(-0.4f, 0.8f)
	});
	path_data.segments.push_back({
		Point2F(-0.4f, 1.0f),
		Point2F(0.4f, 1.0f),
		Point2F(0.4f, 0.8f)
	});
	path_data.segments.push_back({
		Point2F(0.6f, 1.0f),
		Point2F(1.2f, 1.0f),
		Point2F(0.8f, -0.4f)
	});
	path_data.segments.push_back({
		Point2F(1.2f, -0.8f),
		Point2F(0.8f, -0.8f),
		Point2F(0.0f, -0.8f)
	});

	d2d_factory->CreatePathGeometry(&path);
		path->Open(&path_sink);
		path_sink->BeginFigure(Point2F(200, 300), D2D1_FIGURE_BEGIN_FILLED);
			path_sink->AddBezier(BezierSegment(
				Point2F(100, 150), Point2F(500, 150), Point2F(400, 300)));
		path_sink->EndFigure(D2D1_FIGURE_END_OPEN);
	path_sink->Close();

}

void RecreateRenderTarget(HWND hwnd) {
	RECT rc;
	GetClientRect(hwnd, &rc);
	// Utworzenie celu renderowania w oknie Windows
	d2d_factory->CreateHwndRenderTarget(
		RenderTargetProperties(),
		HwndRenderTargetProperties(hwnd,
			SizeU(static_cast<UINT32>(rc.right) -
				static_cast<UINT32>(rc.left),
				static_cast<UINT32>(rc.bottom) -
				static_cast<UINT32>(rc.top))),
		&d2d_render_target);

	if (d2d_render_target == NULL) {
		exit(3);
	}

	d2d_render_target->CreateSolidColorBrush(color_grey, &brush);


}

void DestroyRenderTarget() {
	if (d2d_render_target) {
		d2d_render_target->Release();
		d2d_render_target = nullptr;
	}
}

void DestroyDirect2D() {
	// Bezpieczne zwolnienie zasobów
	if (d2d_render_target) d2d_render_target->Release();
	if (d2d_factory) d2d_factory->Release();
}

void OnPaint(HWND hwnd) {
	if (!d2d_render_target) RecreateRenderTarget(hwnd);

	d2d_render_target->BeginDraw();
	d2d_render_target->Clear(clear_color);

	


	if (d2d_render_target->EndDraw() == D2DERR_RECREATE_TARGET) {
		DestroyRenderTarget();
		OnPaint(hwnd);
	}
}