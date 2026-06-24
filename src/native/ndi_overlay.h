#pragma once

#include <stdint.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PhantomMirrorNdiConfig {
	const char *source_name;
	int prefer_exact_name;
	int monitor_x;
	int monitor_y;
	int monitor_width;
	int monitor_height;
	int include_in_screen_capture;
	int viewport_enabled;
	int viewport_width;
	int viewport_height;
	int viewport_anchor;
};

struct PhantomMirrorNdiStatus {
	int running;
	int connected;
	int source_found;
	int width;
	int height;
	double fps;
	long frame;
	char source_name[256];
	char last_error[256];
};

int phantom_mirror_ndi_start(HWND owner, const PhantomMirrorNdiConfig *config);
int phantom_mirror_ndi_update(const PhantomMirrorNdiConfig *config);
void phantom_mirror_ndi_set_visible(int visible);
void phantom_mirror_ndi_stop(void);
void phantom_mirror_ndi_status(PhantomMirrorNdiStatus *status);

#ifdef __cplusplus
}
#endif
