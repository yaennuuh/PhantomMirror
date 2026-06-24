#include "ndi_sdk.h"

#include <mutex>
#include <vector>

namespace ndi {
namespace {

std::string dirname_of(const std::string &path)
{
	const size_t pos = path.find_last_of("/\\");
	return pos == std::string::npos ? std::string() : path.substr(0, pos);
}

std::string module_dir()
{
	char buffer[MAX_PATH]{};
	const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
	if (length == 0 || length >= MAX_PATH)
		return {};
	return dirname_of(std::string(buffer, length));
}

void append_candidate(std::vector<std::string> &paths, const std::string &candidate)
{
	if (candidate.empty())
		return;
	for (const std::string &path : paths)
		if (_stricmp(path.c_str(), candidate.c_str()) == 0)
			return;
	paths.push_back(candidate);
}

std::vector<std::string> build_candidates()
{
	std::vector<std::string> candidates;
	append_candidate(candidates, module_dir() + "\\Processing.NDI.Lib.x64.dll");

	char env_buffer[MAX_PATH]{};
	if (GetEnvironmentVariableA("NDI_RUNTIME_DIR_V6", env_buffer, MAX_PATH) > 0)
		append_candidate(candidates, std::string(env_buffer) + "\\Processing.NDI.Lib.x64.dll");
	if (GetEnvironmentVariableA("NDI_RUNTIME_DIR_V5", env_buffer, MAX_PATH) > 0)
		append_candidate(candidates, std::string(env_buffer) + "\\Processing.NDI.Lib.x64.dll");

	append_candidate(candidates, "C:\\Program Files\\NDI\\NDI 6 Tools\\Runtime\\Processing.NDI.Lib.x64.dll");
	append_candidate(candidates, "C:\\Program Files\\NDI\\NDI 5 Runtime\\v5\\Processing.NDI.Lib.x64.dll");
	append_candidate(candidates, "C:\\Program Files\\NDI\\NDI 6 Tools\\Router\\Processing.NDI.Lib.x64.dll");
	return candidates;
}

template <typename T>
bool load_symbol(HMODULE module, const char *name, T &target)
{
	target = reinterpret_cast<T>(GetProcAddress(module, name));
	return target != nullptr;
}

Api g_api;
std::mutex g_mutex;
std::string g_error;

bool try_load_api(Api &loaded_api, std::string &error)
{
	for (const std::string &candidate : build_candidates()) {
		HMODULE module = LoadLibraryA(candidate.c_str());
		if (!module)
			continue;

		Api api{};
		api.module = module;
		if (!load_symbol(module, "NDIlib_initialize", api.initialize) ||
		    !load_symbol(module, "NDIlib_destroy", api.destroy) ||
		    !load_symbol(module, "NDIlib_version", api.version) ||
		    !load_symbol(module, "NDIlib_is_supported_CPU", api.is_supported_cpu) ||
		    !load_symbol(module, "NDIlib_find_create_v2", api.find_create_v2) ||
		    !load_symbol(module, "NDIlib_find_destroy", api.find_destroy) ||
		    !load_symbol(module, "NDIlib_find_get_current_sources", api.find_get_current_sources) ||
		    !load_symbol(module, "NDIlib_find_wait_for_sources", api.find_wait_for_sources) ||
		    !load_symbol(module, "NDIlib_recv_create_v3", api.recv_create_v3) ||
		    !load_symbol(module, "NDIlib_recv_destroy", api.recv_destroy) ||
		    !load_symbol(module, "NDIlib_recv_capture_v2", api.recv_capture_v2) ||
		    !load_symbol(module, "NDIlib_recv_free_video_v2", api.recv_free_video_v2) ||
		    !load_symbol(module, "NDIlib_recv_get_no_connections", api.recv_get_no_connections)) {
			FreeLibrary(module);
			error = "NDI runtime is missing required entry points";
			continue;
		}

		if (!api.is_supported_cpu()) {
			FreeLibrary(module);
			error = "NDI runtime requires a newer CPU";
			continue;
		}
		if (!api.initialize()) {
			FreeLibrary(module);
			error = "NDI runtime failed to initialize";
			continue;
		}

		loaded_api = api;
		error.clear();
		return true;
	}

	if (error.empty())
		error = "NDI runtime not found";
	return false;
}

} // namespace

const Api *load_api(std::string *error)
{
	std::lock_guard<std::mutex> lock(g_mutex);
	if (!g_api.module) {
		Api loaded_api{};
		std::string load_error;
		if (try_load_api(loaded_api, load_error)) {
			g_api = loaded_api;
			g_error.clear();
		} else {
			g_error = load_error;
		}
	}
	if (!g_api.module) {
		if (error)
			*error = g_error;
		return nullptr;
	}
	if (error)
		error->clear();
	return &g_api;
}

RuntimeAvailability runtime_availability()
{
	Api loaded_api{};
	std::string error;
	RuntimeAvailability result;
	result.available = try_load_api(loaded_api, error);
	result.error = error;
	if (loaded_api.module) {
		loaded_api.destroy();
		FreeLibrary(loaded_api.module);
	}
	return result;
}

std::vector<std::string> available_runtime_paths()
{
	return build_candidates();
}

} // namespace ndi
