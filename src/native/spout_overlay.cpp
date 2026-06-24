#include "spout_overlay.h"

#include "SpoutDX.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr wchar_t kWindowClass[] = L"PhantomMirrorSpoutOverlayWindow";
constexpr ULONGLONG kFrameTimeoutMs = 750;
#ifndef WDA_EXCLUDEFROMCAPTURE
constexpr DWORD WDA_EXCLUDEFROMCAPTURE = 0x00000011;
#endif

struct SharedState {
	std::mutex mutex;
	PhantomMirrorSpoutConfig config{};
	std::string sender_name = "OBS Spout2 Output";
	PhantomMirrorSpoutStatus status{};
	HWND owner = nullptr;
	bool visible = true;
	std::atomic<bool> stop{false};
	std::thread worker;
};

SharedState g_state;

std::string safe_sender_name(const char *sender_name)
{
	if (!sender_name || !sender_name[0])
		return "OBS Spout2 Output";
	std::string value(sender_name);
	if (value.size() > 255)
		value.resize(255);
	return value;
}

void store_config_locked(const PhantomMirrorSpoutConfig &config)
{
	g_state.config = config;
	g_state.config.sender_name = nullptr;
	g_state.sender_name = safe_sender_name(config.sender_name);
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

RECT compute_viewport(const PhantomMirrorSpoutConfig &config)
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

RECT compute_dest_rect(const PhantomMirrorSpoutConfig &config, unsigned int source_width, unsigned int source_height)
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

// Lookup table: premult_lut[alpha][channel] = (channel * alpha) / 255.
// Replaces per-pixel integer division in the hot path.
static uint8_t premult_lut[256][256];
static std::once_flag premult_lut_once;

static void init_premult_lut()
{
	for (int a = 0; a < 256; ++a)
		for (int c = 0; c < 256; ++c)
			premult_lut[a][c] = static_cast<uint8_t>((c * a) / 255);
}

bool is_rgba_sender_format(DXGI_FORMAT format)
{
	return format == DXGI_FORMAT_R8G8B8A8_UNORM ||
	       format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ||
	       format == DXGI_FORMAT_R8G8B8A8_SNORM;
}

void copy_to_bgra_premultiplied(std::vector<unsigned char> &target, const std::vector<unsigned char> &source,
				bool source_is_bgra)
{
	std::call_once(premult_lut_once, init_premult_lut);
	target.resize(source.size());
	const size_t count = source.size();
	for (size_t i = 0; i + 3 < count; i += 4) {
		const uint8_t c0 = source[i + 0], g = source[i + 1], c2 = source[i + 2], a = source[i + 3];
		const uint8_t b = source_is_bgra ? c0 : c2;
		const uint8_t r = source_is_bgra ? c2 : c0;
		target[i + 0] = premult_lut[a][b];
		target[i + 1] = premult_lut[a][g];
		target[i + 2] = premult_lut[a][r];
		target[i + 3] = a;
	}
}

// Caches the memory DC and DIB bitmap across frames — avoids per-frame
// CreateCompatibleDC / CreateDIBSection / DeleteObject overhead.
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

void render_thread()
{
	HINSTANCE instance = GetModuleHandleW(nullptr);
	WNDCLASSW wc{};
	wc.lpfnWndProc = overlay_proc;
	wc.hInstance = instance;
	wc.lpszClassName = kWindowClass;
	RegisterClassW(&wc);

	HWND hwnd = CreateWindowExW(
		WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
		kWindowClass, L"Phantom Mirror Spout Overlay", WS_POPUP,
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

	spoutDX receiver;
	if (!receiver.OpenDirectX11()) {
		set_error("Cannot initialize Spout DirectX receiver");
		DestroyWindow(hwnd);
		return;
	}

	std::vector<unsigned char> receive_pixels;
	std::vector<unsigned char> bgra_pixels;
	std::vector<unsigned char> scaled_pixels;
	std::vector<int> src_x_lut;
	std::string active_sender;
	unsigned int width = 0, height = 0;
	int last_scale_src_w = 0, last_scale_dst_w = 0;
	bool overlay_visible = false;
	bool requested_visible = true;
	bool include_in_screen_capture = capture_visible_in_screen;
	ULONGLONG last_frame_time = 0;
	GdiCache gdi;

	{
		std::lock_guard<std::mutex> lock(g_state.mutex);
		g_state.status.running = 1;
	}

	while (!g_state.stop.load()) {
		PhantomMirrorSpoutConfig config{};
		std::string configured_sender;
		{
			std::lock_guard<std::mutex> lock(g_state.mutex);
			config = g_state.config;
			configured_sender = g_state.sender_name;
			requested_visible = g_state.visible;
		}
		if (include_in_screen_capture != (config.include_in_screen_capture != 0)) {
			include_in_screen_capture = config.include_in_screen_capture != 0;
			if (!apply_capture_policy(hwnd, include_in_screen_capture))
				set_error("Capture exclusion unavailable");
		}

		const std::string sender = configured_sender.empty() ? "OBS Spout2 Output" : configured_sender;
		if (sender != active_sender) {
			receiver.ReleaseReceiver();
			receiver.SetReceiverName(sender.c_str());
			active_sender = sender;
			width = height = 0;
			receive_pixels.clear();
			bgra_pixels.clear();
			if (overlay_visible)
				overlay_visible = !clear_layered(hwnd, gdi);
		}

		bool connected = receiver.ReceiveImage(
			receive_pixels.empty() ? nullptr : receive_pixels.data(), width, height, false, false);
		if (receiver.IsUpdated()) {
			width = receiver.GetSenderWidth();
			height = receiver.GetSenderHeight();
			if (width > 0 && height > 0) {
				receive_pixels.assign(static_cast<size_t>(width) * height * 4, 0);
				bgra_pixels.resize(receive_pixels.size());
			}
			connected = receiver.IsConnected();
		}

		const bool frame_new = receiver.IsFrameNew();
		if (!requested_visible) {
			if (overlay_visible)
				overlay_visible = !clear_layered(hwnd, gdi);
		} else if (connected && frame_new && !receive_pixels.empty() && width > 0 && height > 0) {
			last_frame_time = GetTickCount64();
			copy_to_bgra_premultiplied(
				bgra_pixels,
				receive_pixels,
				!is_rgba_sender_format(receiver.GetSenderFormat()));
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
		} else if (requested_visible && !bgra_pixels.empty() && width > 0 && height > 0 && !overlay_visible) {
			const RECT dest = compute_dest_rect(config, width, height);
			const int dest_w = std::max(1L, dest.right - dest.left);
			const int dest_h = std::max(1L, dest.bottom - dest.top);
			if (dest_w == static_cast<int>(width) && dest_h == static_cast<int>(height))
				overlay_visible = update_layered(hwnd, dest, bgra_pixels, dest_w, dest_h, gdi);
		} else if ((!connected || !receiver.IsConnected()) && overlay_visible) {
			overlay_visible = !clear_layered(hwnd, gdi);
		} else if (connected && overlay_visible && last_frame_time > 0 &&
			   GetTickCount64() - last_frame_time > kFrameTimeoutMs) {
			overlay_visible = !clear_layered(hwnd, gdi);
		}

		{
			std::lock_guard<std::mutex> lock(g_state.mutex);
			g_state.status.connected = connected ? 1 : 0;
			g_state.status.sender_found = receiver.IsConnected() ? 1 : 0;
			g_state.status.width = static_cast<int>(receiver.GetSenderWidth());
			g_state.status.height = static_cast<int>(receiver.GetSenderHeight());
			g_state.status.fps = receiver.GetSenderFps();
			g_state.status.frame = receiver.GetSenderFrame();
			const char *sname = receiver.GetSenderName();
			strncpy_s(g_state.status.sender_name, sname ? sname : sender.c_str(), _TRUNCATE);
			if (connected) {
				if (overlay_visible || frame_new)
					g_state.status.last_error[0] = '\0';
				else
					strncpy_s(g_state.status.last_error, "Waiting for new Spout frame", _TRUNCATE);
			} else {
				strncpy_s(g_state.status.last_error, "Waiting for Spout sender", _TRUNCATE);
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

	receiver.ReleaseReceiver();
	receiver.CloseDirectX11();
	DestroyWindow(hwnd);
	UnregisterClassW(kWindowClass, instance);
	{
		std::lock_guard<std::mutex> lock(g_state.mutex);
		g_state.status.running = 0;
		g_state.status.connected = 0;
	}
}

} // namespace

int phantom_mirror_spout_start(HWND owner, const PhantomMirrorSpoutConfig *config)
{
	if (!config) return 0;
	phantom_mirror_spout_stop();
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

int phantom_mirror_spout_update(const PhantomMirrorSpoutConfig *config)
{
	if (!config) return 0;
	std::lock_guard<std::mutex> lock(g_state.mutex);
	if (!g_state.worker.joinable()) return 0;
	store_config_locked(*config);
	return 1;
}

void phantom_mirror_spout_set_visible(int visible)
{
	std::lock_guard<std::mutex> lock(g_state.mutex);
	g_state.visible = visible != 0;
}

void phantom_mirror_spout_stop(void)
{
	g_state.stop.store(true);
	if (g_state.worker.joinable())
		g_state.worker.join();
}

void phantom_mirror_spout_status(PhantomMirrorSpoutStatus *status)
{
	if (!status) return;
	std::lock_guard<std::mutex> lock(g_state.mutex);
	*status = g_state.status;
}
