#include <d2d1_3.h>
#include "D2DApp.h"
#include <vector>
#include <windowsx.h>

namespace {
	const FLOAT scale = 250;

	// Deklaracje u¿ycia pomocniczych funkcji
	using D2D1::RenderTargetProperties;
	using D2D1::HwndRenderTargetProperties;
	using D2D1::SizeU;
	using D2D1::Point2F;
	using D2D1::BezierSegment;
	using D2D1::QuadraticBezierSegment;
	using D2D1::RadialGradientBrushProperties;
	using D2D1::Matrix3x2F;
	using D2D1::Ellipse;

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
	ID2D1SolidColorBrush* brush_2 = nullptr;
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
	D2D1_COLOR_F const color_black =
	{ .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };

	ID2D1RadialGradientBrush* rad_brush_1 = nullptr;
	ID2D1GradientStopCollection* rad_stops_1 = nullptr;
	UINT const NUM_RAD_STOPS_1 = 3;
	D2D1_GRADIENT_STOP rad_stops_data_1[NUM_RAD_STOPS_1];

	ID2D1RadialGradientBrush* rad_brush_2 = nullptr;
	ID2D1GradientStopCollection* rad_stops_2 = nullptr;
	UINT const NUM_RAD_STOPS_2 = 3;
	D2D1_GRADIENT_STOP rad_stops_data_2[NUM_RAD_STOPS_2];

	ID2D1PathGeometry* path_1 = nullptr;
	ID2D1GeometrySink* path_sink_1 = nullptr;

	Path path_data;
	Matrix3x2F transformation;
	Matrix3x2F transformation_eye_1;
	Matrix3x2F transformation_eye_2;

	FLOAT eye1centerX;;
	FLOAT eye1centerY;
	FLOAT eye2centerX;
	FLOAT eye2centerY;
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
	rad_stops_data_2[1] = { .position = 0.75f, .color = color_white };
	rad_stops_data_2[2] = { .position = 1.0f, .color = color_gray };

	path_data.start = Point2F(0.0f, -0.8f);
	path_data.segments.push_back({
		Point2F(-0.8f, -0.8f),
		Point2F(-1.2f, -0.8f),
		Point2F(-0.8f, 0.4f)
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
		Point2F(0.8f, 0.4f)
		});
	path_data.segments.push_back({
		Point2F(1.2f, -0.8f),
		Point2F(0.8f, -0.8f),
		Point2F(0.0f, -0.8f)
		});

	d2d_factory->CreatePathGeometry(&path_1);
	path_1->Open(&path_sink_1);
	path_sink_1->BeginFigure(path_data.start, D2D1_FIGURE_BEGIN_FILLED);
	for (auto segment : path_data.segments) {
		path_sink_1->AddBezier(BezierSegment(
			segment.control1, segment.control2, segment.target));
	}
	path_sink_1->EndFigure(D2D1_FIGURE_END_OPEN);
	path_sink_1->Close();
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

	FLOAT height = rc.bottom - rc.top;
	FLOAT width = rc.right - rc.left;

	if (d2d_render_target == NULL) {
		exit(3);
	}

	d2d_render_target->CreateSolidColorBrush(color_black, &brush);
	d2d_render_target->CreateSolidColorBrush(color_gray, &brush_2);

	d2d_render_target->CreateGradientStopCollection(
		rad_stops_data_1, NUM_RAD_STOPS_1, &rad_stops_1);
	if (rad_stops_1) {
		d2d_render_target->CreateRadialGradientBrush(
			RadialGradientBrushProperties(Point2F(0, 0),
				Point2F(0, 0), 1.0f, 1.0f),
			rad_stops_1, &rad_brush_1);
	}
	
	d2d_render_target->CreateGradientStopCollection(
		rad_stops_data_2, NUM_RAD_STOPS_2, &rad_stops_2);
	if (rad_stops_2) {
		d2d_render_target->CreateRadialGradientBrush(
			RadialGradientBrushProperties(Point2F(0, 0),
				Point2F(0, 0), 0.275f, 0.275f),
			rad_stops_2, &rad_brush_2);
	}

	transformation = Matrix3x2F::Scale(300.0f, -300.0f, Point2F(0.0f, 0.0f));
	transformation.SetProduct(transformation, Matrix3x2F::Translation(width / 2.0f, height / 2.0f));

	transformation_eye_1 = Matrix3x2F::Translation(-0.4f, 0.4f);
	transformation_eye_1.SetProduct(transformation_eye_1, Matrix3x2F::Scale(300.0f, -300.0f, Point2F(0.0f, 0.0f)));
	transformation_eye_1.SetProduct(transformation_eye_1, Matrix3x2F::Translation(width / 2.0f, height / 2.0f));

	transformation_eye_2 = Matrix3x2F::Translation(0.4f, 0.4f);
	transformation_eye_2.SetProduct(transformation_eye_2, Matrix3x2F::Scale(300.0f, -300.0f, Point2F(0.0f, 0.0f)));
	transformation_eye_2.SetProduct(transformation_eye_2, Matrix3x2F::Translation(width / 2.0f, height / 2.0f));

	eye1centerX = width / 2.0f - 120;
	eye1centerY = height / 2.0f - 120;
	eye2centerX = width / 2.0f + 120;
	eye2centerY = height / 2.0f - 120;
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

void OnPaint(HWND hwnd, FLOAT arg, LPARAM lparam) {
	if (!d2d_render_target) RecreateRenderTarget(hwnd);

	d2d_render_target->BeginDraw();
	d2d_render_target->Clear(clear_color);
	
	d2d_render_target->SetTransform(transformation);
	d2d_render_target->FillGeometry(path_1, rad_brush_1);
	d2d_render_target->DrawGeometry(path_1, brush, 0.01f);

	d2d_render_target->SetTransform(transformation_eye_1);
	d2d_render_target->FillEllipse(Ellipse(Point2F(0, 0), 0.25f, 0.25f), rad_brush_2);
	d2d_render_target->DrawEllipse(Ellipse(Point2F(0, 0), 0.25f, 0.25f), brush, 0.01f);

	d2d_render_target->SetTransform(transformation_eye_2);
	d2d_render_target->FillEllipse(Ellipse(Point2F(0, 0), 0.25f, 0.25f), rad_brush_2);
	d2d_render_target->DrawEllipse(Ellipse(Point2F(0, 0), 0.25f, 0.25f), brush, 0.01f);

	d2d_render_target->SetTransform(Matrix3x2F::Identity());
	FLOAT x = GET_X_LPARAM(lparam);
	FLOAT y = GET_Y_LPARAM(lparam);

	FLOAT eyeX = eye1centerX, eyeY = eye1centerY;

	FLOAT dist = sqrt((x - eye1centerX) * (x - eye1centerX) + (y - eye1centerY) * (y - eye1centerY));
	if (dist > 50) {
		eyeX = x * 50 / dist + eye1centerX * (1 - 50 / dist);
		eyeY = y * 50 / dist + eye1centerY * (1 - 50 / dist);
	}
	
	d2d_render_target->FillEllipse(Ellipse(Point2F(eyeX, eyeY), 25.0f, 25.0f), brush);

	eyeX = eye2centerX;
	eyeY = eye2centerY;
	dist = sqrt((x - eye2centerX) * (x - eye2centerX) + (y - eye2centerY) * (y - eye2centerY));
	if (dist > 50) {
		eyeX = x * 50 / dist + eye2centerX * (1 - 50 / dist);
		eyeY = y * 50 / dist + eye2centerY * (1 - 50 / dist);
	}

	d2d_render_target->FillEllipse(Ellipse(Point2F(eyeX, eyeY), 25.0f, 25.0f), brush);



	if (d2d_render_target->EndDraw() == D2DERR_RECREATE_TARGET) {
		DestroyRenderTarget();
		OnPaint(hwnd, arg, lparam);
	}
}