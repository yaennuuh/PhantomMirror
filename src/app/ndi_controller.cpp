#include "ndi_controller.h"

#include "../native/ndi_overlay.h"
#include "../native/ndi_sdk.h"

#include <QByteArray>

namespace {

QString fromNativeText(const char *value)
{
	return QString::fromLocal8Bit(value ? value : "").trimmed();
}

PhantomMirrorNdiConfig makeNativeConfig(const AppConfig &config, const QRect &monitorRect, const QByteArray &sourceName)
{
	return PhantomMirrorNdiConfig{
		sourceName.constData(),
		config.overlay.ndiReceiver.preferExactName ? 1 : 0,
		monitorRect.x(),
		monitorRect.y(),
		monitorRect.width(),
		monitorRect.height(),
		config.overlay.includeInScreenCapture ? 1 : 0,
		config.overlay.virtualViewport.enabled ? 1 : 0,
		config.overlay.virtualViewport.width,
		config.overlay.virtualViewport.height,
		viewportAnchorIndex(config.overlay.virtualViewport.anchor),
	};
}

} // namespace

NdiController::NdiController(QObject *parent)
	: QObject(parent)
{
	pollTimer_.setInterval(500);
	connect(&pollTimer_, &QTimer::timeout, this, &NdiController::pollStatus);
}

NdiController::~NdiController()
{
	stop();
}

bool NdiController::start(HWND owner, const AppConfig &config, const QRect &monitorRect)
{
	return apply(owner, config, monitorRect, true);
}

bool NdiController::update(const AppConfig &config, const QRect &monitorRect)
{
	return apply(nullptr, config, monitorRect, false);
}

void NdiController::stop()
{
	phantom_mirror_ndi_stop();
	enabled_ = false;
	lastError_.clear();
	pollTimer_.stop();
	emit statusChanged(status());
}

NdiStatus NdiController::status() const
{
	if (!lastError_.isEmpty())
		return unavailableStatus(lastError_);

	PhantomMirrorNdiStatus native{};
	phantom_mirror_ndi_status(&native);
	return NdiStatus{
		native.running != 0,
		native.connected != 0,
		native.source_found != 0,
		native.width,
		native.height,
		native.fps,
		native.frame,
		fromNativeText(native.source_name),
		fromNativeText(native.last_error),
	};
}

NdiRuntimeAvailability NdiController::runtimeAvailability() const
{
	const ndi::RuntimeAvailability native = ndi::runtime_availability();
	return NdiRuntimeAvailability{native.available, QString::fromLocal8Bit(native.error.c_str()).trimmed()};
}

QStringList NdiController::availableSources() const
{
	std::string error;
	const ndi::Api *api = ndi::load_api(&error);
	if (!api)
		return {};

	const ndi::find_create_t finderSettings{true, nullptr, nullptr};
	ndi::find_instance_t finder = api->find_create_v2(&finderSettings);
	if (!finder)
		return {};

	api->find_wait_for_sources(finder, 150);
	uint32_t count = 0;
	const ndi::source_t *sources = api->find_get_current_sources(finder, &count);
	QStringList result;
	result.reserve(static_cast<qsizetype>(count));
	for (uint32_t i = 0; i < count; ++i) {
		const QString name = QString::fromUtf8(sources[i].p_ndi_name ? sources[i].p_ndi_name : "").trimmed();
		if (!name.isEmpty() && !result.contains(name))
			result.append(name);
	}
	api->find_destroy(finder);
	return result;
}

void NdiController::pollStatus()
{
	emit statusChanged(status());
}

bool NdiController::apply(HWND owner, const AppConfig &config, const QRect &monitorRect, bool allowStart)
{
	if (!config.overlay.ndiReceiver.enabled) {
		stop();
		return true;
	}

	const NdiRuntimeAvailability availability = runtimeAvailability();
	if (!availability.available) {
		phantom_mirror_ndi_stop();
		enabled_ = false;
		lastError_ = availability.error;
		pollTimer_.stop();
		emit statusChanged(status());
		return false;
	}

	const QByteArray sourceName = config.overlay.ndiReceiver.sourceName.toUtf8();
	const PhantomMirrorNdiConfig native = makeNativeConfig(config, monitorRect, sourceName);
	if (phantom_mirror_ndi_update(&native) == 0) {
		if (!allowStart || phantom_mirror_ndi_start(owner, &native) == 0)
			return false;
	}

	enabled_ = true;
	lastError_.clear();
	if (!pollTimer_.isActive())
		pollTimer_.start();
	pollStatus();
	return true;
}

NdiStatus NdiController::unavailableStatus(const QString &error) const
{
	NdiStatus status;
	status.lastError = error;
	return status;
}
