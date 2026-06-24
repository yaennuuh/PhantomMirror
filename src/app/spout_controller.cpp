#include "spout_controller.h"

#include "../native/spout_overlay.h"
#include "SpoutDX.h"

#include <QByteArray>

namespace {

QString fromNativeText(const char *value)
{
	return QString::fromLocal8Bit(value ? value : "").trimmed();
}

PhantomMirrorSpoutConfig makeNativeConfig(const AppConfig &config, const QRect &monitorRect, const QByteArray &senderName)
{
	return PhantomMirrorSpoutConfig{
		senderName.constData(),
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

SpoutController::SpoutController(QObject *parent)
	: QObject(parent)
{
	pollTimer_.setInterval(500);
	connect(&pollTimer_, &QTimer::timeout, this, &SpoutController::pollStatus);
}

SpoutController::~SpoutController()
{
	stop();
}

bool SpoutController::start(HWND owner, const AppConfig &config, const QRect &monitorRect)
{
	return apply(owner, config, monitorRect, true);
}

bool SpoutController::update(const AppConfig &config, const QRect &monitorRect)
{
	return apply(nullptr, config, monitorRect, false);
}

void SpoutController::stop()
{
	phantom_mirror_spout_stop();
	enabled_ = false;
	pollTimer_.stop();
	emit statusChanged(status());
}

SpoutStatus SpoutController::status() const
{
	PhantomMirrorSpoutStatus native{};
	phantom_mirror_spout_status(&native);
	return SpoutStatus{
		native.running != 0,
		native.connected != 0,
		native.sender_found != 0,
		native.width,
		native.height,
		native.fps,
		native.frame,
		fromNativeText(native.sender_name),
		fromNativeText(native.last_error),
	};
}

QStringList SpoutController::availableSenders() const
{
	spoutDX senderQuery;
	const std::vector<std::string> senders = senderQuery.GetSenderList();
	QStringList result;
	result.reserve(static_cast<qsizetype>(senders.size()));
	for (const std::string &sender : senders) {
		const QString name = QString::fromLocal8Bit(sender.c_str()).trimmed();
		if (!name.isEmpty() && !result.contains(name))
			result.append(name);
	}
	return result;
}

void SpoutController::pollStatus()
{
	emit statusChanged(status());
}

bool SpoutController::apply(HWND owner, const AppConfig &config, const QRect &monitorRect, bool allowStart)
{
	if (!config.overlay.spoutReceiver.enabled) {
		stop();
		return true;
	}

	const QByteArray senderName = config.overlay.spoutReceiver.senderName.toLocal8Bit();
	const PhantomMirrorSpoutConfig native = makeNativeConfig(config, monitorRect, senderName);
	if (phantom_mirror_spout_update(&native) == 0) {
		if (!allowStart || phantom_mirror_spout_start(owner, &native) == 0)
			return false;
	}

	enabled_ = true;
	if (!pollTimer_.isActive())
		pollTimer_.start();
	pollStatus();
	return true;
}
