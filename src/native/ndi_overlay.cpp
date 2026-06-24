#include "ndi_overlay.h"

#include "ndi_sdk.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstddef>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr wchar_t kWindowClass[] = L"PhantomMirrorNdiOverlayWindow";
constexpr ULONGLONG kFrameTimeoutMs = 750;
#ifndef WDA_EXCLUDEFROMCAPTURE
constexpr DWORD WDA_EXCLUDEFROMCAPTURE = 0x00000011;
#endif

struct SharedState {
	std::mutex mutex;
	PhantomMirrorNdiConfig config{};
	std::string source_name;
	PhantomMirrorNdiStatus status{};
	HWND owner = nullptr;
	bool visible = true;
	std::atomic<bool> stop{false};
	std::thread worker;
};

SharedState g_state;

std::string safe_source_name(const char *source_name)
{
	if (!source_name)
		return {};
	std::string value(source_name);
	if (value.size() > 255)
		value.resize(255);
	return value;
}

void store_config_locked(const PhantomMirrorNdiConfig &config)
{
	g_state.config = config;
	g_state.config.source_name = nullptr;
	g_state.source_name = safe_source_name(config.source_name);
}

void set_error(const char *message)
{
	std::lock_guard<std::mutex> lock(g_state.mutex);
	strncpy_s(g_state.status.last_error, message ? message : "", _TRUNCATE);
}

bool apply_capture_exclusion(HWND hwnd)
{
	if (!hwnd)
		return false;
	if (SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE))
		return true;
	return SetWindowDisplayAffinity(hwnd, WDA_MONITOR) == TRUE;
}

bool apply_capture_policy(HWND hwnd, bool include_in_screen_capture)
{
	if (!hwnd)
		return false;
	if (include_in_screen_capture)
		return SetWindowDisplayAffinity(hwnd, WDA_NONE) == TRUE;
	return apply_capture_exclusion(hwnd);
}

RECT compute_viewport(const PhantomMirrorNdiConfig &config)
{
	RECT rect{config.monitor_x, config.monitor_y, config.monitor_x + config.monitor_width,
		  config.monitor_y + config.monitor_height};
	if (!config.viewport_enabled || config.viewport_width <= 0 || config.viewport_height <= 0)
		return rect;

	const int width = std::min(config.viewport_width, config.monitor_width);
	const int height = std::min(config.viewport_height, config.monitor_height);
	int x = config.monitor_x;
	int y = config.monitor_y;
	const int horizontal = config.viewport_anchor % 3;
	const int vertical = config.viewport_anchor / 3;
	if (horizontal == 1) x += (config.monitor_width - width) / 2;
	else if (horizontal == 2) x += config.monitor_width - width;
	if (vertical == 1) y += (config.monitor_height - height) / 2;
	else if (vertical == 2) y += config.monitor_height - height;
	return RECT{x, y, x + width, y + height};
}

RECT compute_dest_rect(const PhantomMirrorNdiConfig &config, unsigned int source_width, unsigned int source_height)
{
	const RECT viewport = compute_viewport(config);
	const int viewport_width = std::max(1L, viewport.right - viewport.left);
	const int viewport_height = std::max(1L, viewport.bottom - viewport.top);
	if (source_width == 0 || source_height == 0)
		return viewport;

	const double scale_x = static_cast<double>(viewport_width) / static_cast<double>(source_width);
	const double scale_y = static_cast<double>(viewport_height) / static_cast<double>(source_height);
	const double scale = std::min(scale_x, scale_y);
	const int width = std::max(1, static_cast<int>(static_cast<double>(source_width) * scale + 0.5));
	const int height = std::max(1, static_cast<int>(static_cast<double>(source_height) * scale + 0.5));
	const int x = viewport.left + (viewport_width - width) / 2;
	const int y = viewport.top + (viewport_height - height) / 2;
	return RECT{x, y, x + width, y + height};
}

LRESULT CALLBACK overlay_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_CLOSE:
	case WM_DESTROY:
		return 0;
	default:
		return DefWindowProcW(hwnd, msg, wparam, lparam);
	}
}

static uint8_t premult_lut[256][256];
static std::once_flag premult_lut_once;

static void init_premult_lut()
{
	for (int a = 0; a < 256; ++a)
		for (int c = 0; c < 256; ++c)
			premult_lut[a][c] = static_cast<uint8_t>((c * a) / 255);
}

void copy_to_bgra_premultiplied(std::vector<unsigned char> &target, const uint8_t *source, int width, int height,
				int stride_bytes, bool source_is_bgra, bool source_has_alpha)
{
	std::call_once(premult_lut_once, init_premult_lut);
	target.resize(static_cast<size_t>(width) * height * 4);
	for (int y = 0; y < height; ++y) {
		const uint8_t *src_row = source + static_cast<ptrdiff_t>(y) * stride_bytes;
		uint8_t *dst_row = target.data() + static_cast<size_t>(y) * width * 4;
		for (int x = 0; x < width; ++x) {
			const size_t si = static_cast<size_t>(x) * 4;
			const uint8_t c0 = src_row[si + 0];
			const uint8_t g = src_row[si + 1];
			const uint8_t c2 = src_row[si + 2];
			const uint8_t a = source_has_alpha ? src_row[si + 3] : 255;
			const uint8_t b = source_is_bgra ? c0 : c2;
			const uint8_t r = source_is_bgra ? c2 : c0;
			dst_row[si + 0] = premult_lut[a][b];
			dst_row[si + 1] = premult_lut[a][g];
			dst_row[si + 2] = premult_lut[a][r];
			dst_row[si + 3] = a;
		}
	}
}

bool fourcc_is_bgra_family(ndi::FourCC_type_e fourcc)
{
	return fourcc == ndi::FourCC_type_BGRA || fourcc == ndi::FourCC_type_BGRX;
}

bool fourcc_is_rgba_family(ndi::FourCC_type_e fourcc)
{
	return fourcc == ndi::FourCC_type_RGBA || fourcc == ndi::FourCC_type_RGBX;
}

bool fourcc_has_alpha(ndi::FourCC_type_e fourcc)
{
	return fourcc == ndi::FourCC_type_BGRA || fourcc == ndi::FourCC_type_RGBA;
}

bool fourcc_supported_for_overlay(ndi::FourCC_type_e fourcc)
{
	return fourcc_is_bgra_family(fourcc) || fourcc_is_rgba_family(fourcc);
}

struct GdiCache {
	HDC memory_dc = nullptr;
	HBITMAP bitmap = nullptr;
	HGDIOBJ old_bitmap = nullptr;
	void *bits = nullptr;
	int cached_width = 0;
	int cached_height = 0;

	~GdiCache() { release(); }

	void release()
	{
		if (memory_dc && old_bitmap) { SelectObject(memory_dc, old_bitmap); old_bitmap = nullptr; }
		if (bitmap) { DeleteObject(bitmap); bitmap = nullptr; bits = nullptr; }
		if (memory_dc) { DeleteDC(memory_dc); memory_dc = nullptr; }
		cached_width = cached_height = 0;
	}

	bool ensure(int w, int h)
	{
		if (w == cached_width && h == cached_height && memory_dc && bitmap)
			return true;
		release();
		memory_dc = CreateCompatibleDC(nullptr);
		if (!memory_dc) return false;
		BITMAPINFO info{};
		info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info.bmiHeader.biWidth = w;
		info.bmiHeader.biHeight = -h;
		info.bmiHeader.biPlanes = 1;
		info.bmiHeader.biBitCount = 32;
		info.bmiHeader.biCompression = BI_RGB;
		bitmap = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
		if (!bitmap || !bits) { DeleteDC(memory_dc); memory_dc = nullptr; return false; }
		old_bitmap = SelectObject(memory_dc, bitmap);
		cached_width = w;
		cached_height = h;
		return true;
	}
};

bool update_layered(HWND hwnd, const RECT &dest, const std::vector<unsigned char> &pixels,
		    int w, int h, GdiCache &gdi)
{
	if (pixels.empty() || w <= 0 || h <= 0) return false;
	if (!gdi.ensure(w, h)) return false;
	memcpy(gdi.bits, pixels.data(), pixels.size());
	POINT pos{dest.left, dest.top};
	SIZE sz{w, h};
	POINT src{0, 0};
	BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
	return UpdateLayeredWindow(hwnd, nullptr, &pos, &sz, gdi.memory_dc, &src, 0, &blend, ULW_ALPHA) == TRUE;
}

bool clear_layered(HWND hwnd, GdiCache &gdi)
{
	static const std::vector<unsigned char> transparent_pixel(4, 0);
	RECT dest{0, 0, 1, 1};
	return update_layered(hwnd, dest, transparent_pixel, 1, 1, gdi);
}

bool case_insensitive_equals(const std::string &left, const std::string &right)
{
	return _stricmp(left.c_str(), right.c_str()) == 0;
}

bool case_insensitive_contains(const std::string &haystack, const std::string &needle)
{
	if (needle.empty())
		return false;
	std::string left = haystack;
	std::string right = needle;
	std::transform(left.begin(), left.end(), left.begin(), [](unsigned char ch) { return static_cast<char>(tolower(ch)); });
	std::transform(right.begin(), right.end(), right.begin(), [](unsigned char ch) { return static_cast<char>(tolower(ch)); });
	return left.find(right) != std::string::npos;
}

bool select_source(const ndi::Api &api, ndi::find_instance_t finder, const std::string &configured_name,
		   bool prefer_exact_name, ndi::source_t &selected, std::string &selected_name)
{
	uint32_t count = 0;
	const ndi::source_t *sources = api.find_get_current_sources(finder, &count);
	if (!sources || count == 0)
		return false;

	if (configured_name.empty()) {
		selected = sources[0];
		selected_name = selected.p_ndi_name ? selected.p_ndi_name : "";
		return true;
	}

	if (prefer_exact_name) {
		for (uint32_t i = 0; i < count; ++i) {
			const std::string name = sources[i].p_ndi_name ? sources[i].p_ndi_name : "";
			if (case_insensitive_equals(name, configured_name)) {
				selected = sources[i];
				selected_name = name;
				return true;
			}
		}
		return false;
	}

	for (uint32_t i = 0; i < count; ++i) {
		const std::string name = sources[i].p_ndi_name ? sources[i].p_ndi_name : "";
		if (case_insensitive_equals(name, configured_name)) {
			selected = sources[i];
			selected_name = name;
			return true;
		}
	}
	for (uint32_t i = 0; i < count; ++i) {
		const std::string name = sources[i].p_ndi_name ? sources[i].p_ndi_name : "";
		if (case_insensitive_contains(name, configured_name)) {
			selected = sources[i];
			selected_name = name;
			return true;
		}
	}

	return false;
}

void render_thread()
{
	std::string ndi_error;
	const ndi::Api *api = ndi::load_api(&ndi_error);
	if (!api) {
		set_error(ndi_error.c_str());
		return;
	}

	HINSTANCE instance = GetModuleHandleW(nullptr);
	WNDCLASSW wc{};
	wc.lpfnWndProc = overlay_proc;
	wc.hInstance = instance;
	wc.lpszClassName = kWindowClass;
	RegisterClassW(&wc);

	HWND hwnd = CreateWindowExW(
		WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
		kWindowClass, L"Phantom Mirror NDI Overlay", WS_POPUP,
		0, 0, 1, 1, g_state.owner, nullptr, instance, nullptr);
	if (!hwnd) {
		set_error("Cannot create native overlay window");
		return;
	}
	bool capture_visible_in_screen = false;
	{
		std::lock_guard<std::mutex> lock(g_state.mutex);
		capture_visible_in_screen = g_state.config.include_in_screen_capture != 0;
	}
	if (!apply_capture_policy(hwnd, capture_visible_in_screen))
		set_error("Capture exclusion unavailable");
	ShowWindow(hwnd, SW_SHOWNOACTIVATE);

	const ndi::find_create_t finder_settings{true, nullptr, nullptr};
	ndi::find_instance_t finder = api->find_create_v2(&finder_settings);
	if (!finder) {
		set_error("Cannot initialize NDI source discovery");
		DestroyWindow(hwnd);
		return;
	}

	ndi::recv_instance_t receiver = nullptr;
	std::vector<unsigned char> bgra_pixels;
	std::vector<unsigned char> scaled_pixels;
	std::vector<int> src_x_lut;
	std::string active_source;
	unsigned int width = 0, height = 0;
	int last_scale_src_w = 0, last_scale_dst_w = 0;
	bool overlay_visible = false;
	bool requested_visible = true;
	bool include_in_screen_capture = capture_visible_in_screen;
	ULONGLONG last_frame_time = 0;
	long frame_count = 0;
	bool active_source_has_alpha = false;
	bool last_frame_format_supported = true;
	GdiCache gdi;

	{
		std::lock_guard<std::mutex> lock(g_state.mutex);
		g_state.status.running = 1;
	}

	while (!g_state.stop.load()) {
		PhantomMirrorNdiConfig config{};
		std::string configured_name;
		bool prefer_exact_name = true;
		{
			std::lock_guard<std::mutex> lock(g_state.mutex);
			config = g_state.config;
			configured_name = g_state.source_name;
			prefer_exact_name = config.prefer_exact_name != 0;
			requested_visible = g_state.visible;
		}
		if (include_in_screen_capture != (config.include_in_screen_capture != 0)) {
			include_in_screen_capture = config.include_in_screen_capture != 0;
			if (!apply_capture_policy(hwnd, include_in_screen_capture))
				set_error("Capture exclusion unavailable");
		}

		ndi::source_t selected_source{};
		std::string selected_name;
		const bool source_found = select_source(*api, finder, configured_name, prefer_exact_name, selected_source, selected_name);

		if (!source_found) {
			if (receiver) {
				api->recv_destroy(receiver);
				receiver = nullptr;
			}
			active_source.clear();
			width = height = 0;
			if (overlay_visible)
				overlay_visible = !clear_layered(hwnd, gdi);
			{
				std::lock_guard<std::mutex> lock(g_state.mutex);
				g_state.status.connected = 0;
				g_state.status.source_found = 0;
				g_state.status.width = 0;
				g_state.status.height = 0;
				g_state.status.fps = 0.0;
				g_state.status.frame = frame_count;
				g_state.status.source_name[0] = '\0';
				strncpy_s(g_state.status.last_error, "Waiting for NDI source", _TRUNCATE);
			}
			api->find_wait_for_sources(finder, 100);
			continue;
		}

		if (!receiver || !case_insensitive_equals(active_source, selected_name)) {
			if (receiver)
				api->recv_destroy(receiver);
			const ndi::recv_create_v3_t recv_settings{
				selected_source,
				ndi::recv_color_format_BGRX_BGRA,
				ndi::recv_bandwidth_highest,
				false,
				"Phantom Mirror",
			};
			receiver = api->recv_create_v3(&recv_settings);
			active_source = selected_name;
			active_source_has_alpha = false;
			last_frame_format_supported = true;
			width = height = 0;
			bgra_pixels.clear();
			if (overlay_visible)
				overlay_visible = !clear_layered(hwnd, gdi);
		}

		if (!receiver) {
			set_error("Cannot create NDI receiver");
			Sleep(50);
			continue;
		}

		ndi::video_frame_v2_t video_frame{};
		const ndi::frame_type_e frame_type = api->recv_capture_v2(receiver, &video_frame, nullptr, nullptr, 16);
		const bool connected = api->recv_get_no_connections(receiver) > 0;
		bool frame_new = false;

		if (!requested_visible) {
			if (overlay_visible)
				overlay_visible = !clear_layered(hwnd, gdi);
		} else if (frame_type == ndi::frame_type_video && video_frame.p_data && video_frame.xres > 0 && video_frame.yres > 0) {
			width = static_cast<unsigned int>(video_frame.xres);
			height = static_cast<unsigned int>(video_frame.yres);
			if (fourcc_supported_for_overlay(video_frame.FourCC)) {
				last_frame_format_supported = true;
				const bool source_is_bgra = fourcc_is_bgra_family(video_frame.FourCC);
				active_source_has_alpha = fourcc_has_alpha(video_frame.FourCC);
				copy_to_bgra_premultiplied(
					bgra_pixels,
					video_frame.p_data,
					video_frame.xres,
					video_frame.yres,
					video_frame.line_stride_in_bytes,
					source_is_bgra,
					active_source_has_alpha);
				last_frame_time = GetTickCount64();
				++frame_count;
				frame_new = true;

				const RECT dest = compute_dest_rect(config, width, height);
				const int dest_w = std::max(1L, dest.right - dest.left);
				const int dest_h = std::max(1L, dest.bottom - dest.top);
				if (dest_w == static_cast<int>(width) && dest_h == static_cast<int>(height)) {
					overlay_visible = update_layered(hwnd, dest, bgra_pixels, dest_w, dest_h, gdi);
				} else {
					scaled_pixels.resize(static_cast<size_t>(dest_w) * dest_h * 4);
					if (last_scale_src_w != static_cast<int>(width) || last_scale_dst_w != dest_w) {
						src_x_lut.resize(dest_w);
						for (int x = 0; x < dest_w; ++x)
							src_x_lut[x] = std::clamp((x * static_cast<int>(width)) / dest_w,
										  0, static_cast<int>(width) - 1);
						last_scale_src_w = static_cast<int>(width);
						last_scale_dst_w = dest_w;
					}
					for (int y = 0; y < dest_h; ++y) {
						const int sy = std::clamp((y * static_cast<int>(height)) / dest_h,
									  0, static_cast<int>(height) - 1);
						const auto *src_row = reinterpret_cast<const uint32_t *>(bgra_pixels.data())
								      + static_cast<size_t>(sy) * width;
						auto *dst_row = reinterpret_cast<uint32_t *>(scaled_pixels.data())
								+ static_cast<size_t>(y) * dest_w;
						for (int x = 0; x < dest_w; ++x)
							dst_row[x] = src_row[src_x_lut[x]];
					}
					overlay_visible = update_layered(hwnd, dest, scaled_pixels, dest_w, dest_h, gdi);
				}
			} else {
				last_frame_format_supported = false;
			}
			api->recv_free_video_v2(receiver, &video_frame);
		} else if (requested_visible && !bgra_pixels.empty() && width > 0 && height > 0 && !overlay_visible) {
			const RECT dest = compute_dest_rect(config, width, height);
			const int dest_w = std::max(1L, dest.right - dest.left);
			const int dest_h = std::max(1L, dest.bottom - dest.top);
			if (dest_w == static_cast<int>(width) && dest_h == static_cast<int>(height))
				overlay_visible = update_layered(hwnd, dest, bgra_pixels, dest_w, dest_h, gdi);
		} else if (frame_type == ndi::frame_type_error && overlay_visible) {
			overlay_visible = !clear_layered(hwnd, gdi);
		} else if (!connected && overlay_visible) {
			overlay_visible = !clear_layered(hwnd, gdi);
		} else if (connected && overlay_visible && last_frame_time > 0 &&
			   GetTickCount64() - last_frame_time > kFrameTimeoutMs) {
			overlay_visible = !clear_layered(hwnd, gdi);
		}

		{
			std::lock_guard<std::mutex> lock(g_state.mutex);
			g_state.status.connected = connected ? 1 : 0;
			g_state.status.source_found = 1;
			g_state.status.width = static_cast<int>(width);
			g_state.status.height = static_cast<int>(height);
			g_state.status.fps = frame_new ? (video_frame.frame_rate_D > 0 ? static_cast<double>(video_frame.frame_rate_N) /
						static_cast<double>(video_frame.frame_rate_D) : 0.0) : g_state.status.fps;
			g_state.status.frame = frame_count;
			strncpy_s(g_state.status.source_name, active_source.c_str(), _TRUNCATE);
			if (connected) {
				if ((overlay_visible || frame_new) && active_source_has_alpha)
					g_state.status.last_error[0] = '\0';
				else if (overlay_visible || frame_new)
					strncpy_s(g_state.status.last_error, "NDI source has no alpha channel; rendering opaque", _TRUNCATE);
				else if (!last_frame_format_supported)
					strncpy_s(g_state.status.last_error, "NDI video format unsupported for alpha overlay", _TRUNCATE);
				else
					strncpy_s(g_state.status.last_error, "Waiting for NDI video frame", _TRUNCATE);
			} else {
				strncpy_s(g_state.status.last_error, "Connecting to NDI source", _TRUNCATE);
			}
		}

		MSG msg{};
		while (PeekMessageW(&msg, hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		if (!frame_new)
			Sleep(1);
	}

	if (receiver)
		api->recv_destroy(receiver);
	api->find_destroy(finder);
	DestroyWindow(hwnd);
	UnregisterClassW(kWindowClass, instance);
	{
		std::lock_guard<std::mutex> lock(g_state.mutex);
		g_state.status.running = 0;
		g_state.status.connected = 0;
	}
}

} // namespace

int phantom_mirror_ndi_start(HWND owner, const PhantomMirrorNdiConfig *config)
{
	if (!config) return 0;
	phantom_mirror_ndi_stop();
	{
		std::lock_guard<std::mutex> lock(g_state.mutex);
		g_state.owner = owner;
		store_config_locked(*config);
		g_state.visible = true;
		memset(&g_state.status, 0, sizeof(g_state.status));
	}
	g_state.stop.store(false);
	g_state.worker = std::thread(render_thread);
	return 1;
}

int phantom_mirror_ndi_update(const PhantomMirrorNdiConfig *config)
{
	if (!config) return 0;
	std::lock_guard<std::mutex> lock(g_state.mutex);
	if (!g_state.worker.joinable()) return 0;
	store_config_locked(*config);
	return 1;
}

void phantom_mirror_ndi_set_visible(int visible)
{
	std::lock_guard<std::mutex> lock(g_state.mutex);
	g_state.visible = visible != 0;
}

void phantom_mirror_ndi_stop(void)
{
	g_state.stop.store(true);
	if (g_state.worker.joinable())
		g_state.worker.join();
}

void phantom_mirror_ndi_status(PhantomMirrorNdiStatus *status)
{
	if (!status) return;
	std::lock_guard<std::mutex> lock(g_state.mutex);
	*status = g_state.status;
}
