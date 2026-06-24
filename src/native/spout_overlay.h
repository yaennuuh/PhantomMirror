#pragma once

#include <stdint.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PhantomMirrorSpoutConfig {
	const char *sender_name;
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

struct PhantomMirrorSpoutStatus {
	int running;
	int connected;
	int sender_found;
	int width;
	int height;
	double fps;
	long frame;
	char sender_name[256];
	char last_error[256];
};

int phantom_mirror_spout_start(HWND owner, const PhantomMirrorSpoutConfig *config);
int phantom_mirror_spout_update(const PhantomMirrorSpoutConfig *config);
void phantom_mirror_spout_set_visible(int visible);
void phantom_mirror_spout_stop(void);
void phantom_mirror_spout_status(PhantomMirrorSpoutStatus *status);

#ifdef __cplusplus
}
#endif
