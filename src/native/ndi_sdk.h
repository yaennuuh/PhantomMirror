#pragma once

#include <stdint.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>
#include <vector>

// Minimal NDI ABI declarations based on the redistributable SDK headers.
namespace ndi {

constexpr uint32_t make_fourcc(char ch0, char ch1, char ch2, char ch3)
{
	return static_cast<uint32_t>(static_cast<uint8_t>(ch0)) |
	       (static_cast<uint32_t>(static_cast<uint8_t>(ch1)) << 8) |
	       (static_cast<uint32_t>(static_cast<uint8_t>(ch2)) << 16) |
	       (static_cast<uint32_t>(static_cast<uint8_t>(ch3)) << 24);
}

enum frame_type_e {
	frame_type_none = 0,
	frame_type_video = 1,
	frame_type_audio = 2,
	frame_type_metadata = 3,
	frame_type_error = 4,
	frame_type_status_change = 100,
};

enum FourCC_type_e : uint32_t {
	FourCC_type_UYVY = make_fourcc('U', 'Y', 'V', 'Y'),
	FourCC_type_BGRA = make_fourcc('B', 'G', 'R', 'A'),
	FourCC_type_BGRX = make_fourcc('B', 'G', 'R', 'X'),
	FourCC_type_RGBA = make_fourcc('R', 'G', 'B', 'A'),
	FourCC_type_RGBX = make_fourcc('R', 'G', 'B', 'X'),
	FourCC_type_UYVA = make_fourcc('U', 'Y', 'V', 'A'),
};

enum frame_format_type_e {
	frame_format_type_interleaved = 0,
	frame_format_type_progressive = 1,
	frame_format_type_field_0 = 2,
	frame_format_type_field_1 = 3,
};

enum recv_bandwidth_e {
	recv_bandwidth_metadata_only = -10,
	recv_bandwidth_audio_only = 10,
	recv_bandwidth_lowest = 0,
	recv_bandwidth_highest = 100,
};

enum recv_color_format_e {
	recv_color_format_BGRX_BGRA = 0,
	recv_color_format_UYVY_BGRA = 1,
	recv_color_format_RGBX_RGBA = 2,
	recv_color_format_UYVY_RGBA = 3,
	recv_color_format_BGRX_BGRA_flipped = 200,
	recv_color_format_fastest = 100,
};

struct source_t {
	const char *p_ndi_name;
	const char *p_ip_address;
};

struct video_frame_v2_t {
	int xres;
	int yres;
	FourCC_type_e FourCC;
	int frame_rate_N;
	int frame_rate_D;
	float picture_aspect_ratio;
	frame_format_type_e frame_format_type;
	int64_t timecode;
	uint8_t *p_data;
	int line_stride_in_bytes;
	const char *p_metadata;
	int64_t timestamp;
};

struct audio_frame_v2_t {
	int sample_rate;
	int no_channels;
	int no_samples;
	int64_t timecode;
	float *p_data;
	int channel_stride_in_bytes;
	const char *p_metadata;
	int64_t timestamp;
};

struct metadata_frame_t {
	int length;
	int64_t timecode;
	char *p_data;
};

struct find_create_t {
	bool show_local_sources;
	const char *p_groups;
	const char *p_extra_ips;
};

struct recv_create_v3_t {
	source_t source_to_connect_to;
	recv_color_format_e color_format;
	recv_bandwidth_e bandwidth;
	bool allow_video_fields;
	const char *p_ndi_name;
};

using find_instance_t = void *;
using recv_instance_t = void *;

struct Api {
	HMODULE module = nullptr;
	bool (*initialize)(void) = nullptr;
	void (*destroy)(void) = nullptr;
	const char *(*version)(void) = nullptr;
	bool (*is_supported_cpu)(void) = nullptr;
	find_instance_t (*find_create_v2)(const find_create_t *settings) = nullptr;
	void (*find_destroy)(find_instance_t instance) = nullptr;
	const source_t *(*find_get_current_sources)(find_instance_t instance, uint32_t *count) = nullptr;
	bool (*find_wait_for_sources)(find_instance_t instance, uint32_t timeout_ms) = nullptr;
	recv_instance_t (*recv_create_v3)(const recv_create_v3_t *settings) = nullptr;
	void (*recv_destroy)(recv_instance_t instance) = nullptr;
	frame_type_e (*recv_capture_v2)(recv_instance_t instance, video_frame_v2_t *video, audio_frame_v2_t *audio,
					metadata_frame_t *metadata, uint32_t timeout_ms) = nullptr;
	void (*recv_free_video_v2)(recv_instance_t instance, const video_frame_v2_t *video) = nullptr;
	int (*recv_get_no_connections)(recv_instance_t instance) = nullptr;
};

struct RuntimeAvailability {
	bool available = false;
	std::string error;
};

const Api *load_api(std::string *error = nullptr);
RuntimeAvailability runtime_availability();
std::vector<std::string> available_runtime_paths();

} // namespace ndi
