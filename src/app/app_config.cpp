#include "app_config.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QtGlobal>

namespace {

constexpr auto kConfigFileName = "app-config.json";

QJsonObject toJson(const AppConfig &config)
{
	QJsonObject viewport{
		{"enabled", config.overlay.virtualViewport.enabled},
		{"width", config.overlay.virtualViewport.width},
		{"height", config.overlay.virtualViewport.height},
		{"anchor", config.overlay.virtualViewport.anchor},
	};
	QJsonObject spout{
		{"enabled", config.overlay.spoutReceiver.enabled},
		{"senderName", config.overlay.spoutReceiver.senderName},
	};
	QJsonObject ndi{
		{"enabled", config.overlay.ndiReceiver.enabled},
		{"sourceName", config.overlay.ndiReceiver.sourceName},
		{"preferExactName", config.overlay.ndiReceiver.preferExactName},
	};
	QJsonObject hotkey{
		{"enabled", config.overlay.hotkey.enabled},
		{"modifiers", QJsonArray::fromStringList(config.overlay.hotkey.modifiers)},
		{"key", config.overlay.hotkey.key},
	};
	QJsonObject overlay{
		{"input", overlayInputModeToString(config.overlay.input)},
		{"monitor", config.overlay.monitor},
		{"includeInScreenCapture", config.overlay.includeInScreenCapture},
		{"alwaysOnTop", config.overlay.alwaysOnTop},
		{"clickThrough", config.overlay.clickThrough},
		{"virtualViewport", viewport},
		{"spoutReceiver", spout},
		{"ndiReceiver", ndi},
		{"hotkey", hotkey},
	};
	QJsonObject debug{{"enabled", config.debug.enabled}};
	QJsonObject ui{{"language", config.ui.language}};
	QJsonObject onboarding{{"completed", config.onboarding.completed}};
	return QJsonObject{{"overlay", overlay}, {"debug", debug}, {"ui", ui}, {"onboarding", onboarding}};
}

AppConfig fromJson(const QJsonObject &root)
{
	AppConfig config;
	const QJsonObject overlay = root.value("overlay").toObject();
	config.overlay.input = overlayInputModeFromString(overlay.value("input").toString());
	config.overlay.monitor = overlay.value("monitor").toString(config.overlay.monitor).trimmed();
	if (config.overlay.monitor.isEmpty())
		config.overlay.monitor = "primary";
	config.overlay.includeInScreenCapture = overlay.value("includeInScreenCapture").toBool(config.overlay.includeInScreenCapture);
	config.overlay.alwaysOnTop = overlay.value("alwaysOnTop").toBool(config.overlay.alwaysOnTop);
	config.overlay.clickThrough = overlay.value("clickThrough").toBool(config.overlay.clickThrough);

	const QJsonObject viewport = overlay.value("virtualViewport").toObject();
	config.overlay.virtualViewport.enabled = viewport.value("enabled").toBool(config.overlay.virtualViewport.enabled);
	config.overlay.virtualViewport.width = qMax(1, viewport.value("width").toInt(config.overlay.virtualViewport.width));
	config.overlay.virtualViewport.height = qMax(1, viewport.value("height").toInt(config.overlay.virtualViewport.height));
	config.overlay.virtualViewport.anchor = normalizeAnchor(viewport.value("anchor").toString(config.overlay.virtualViewport.anchor));

	const QJsonObject spout = overlay.value("spoutReceiver").toObject();
	config.overlay.spoutReceiver.enabled = spout.value("enabled").toBool(config.overlay.spoutReceiver.enabled);
	config.overlay.spoutReceiver.senderName = spout.value("senderName").toString(config.overlay.spoutReceiver.senderName).trimmed();
	if (config.overlay.spoutReceiver.senderName.isEmpty())
		config.overlay.spoutReceiver.senderName = "OBS Spout2 Output";

	const QJsonObject ndi = overlay.value("ndiReceiver").toObject();
	config.overlay.ndiReceiver.enabled = ndi.value("enabled").toBool(config.overlay.ndiReceiver.enabled);
	config.overlay.ndiReceiver.sourceName = ndi.value("sourceName").toString(config.overlay.ndiReceiver.sourceName).trimmed();
	config.overlay.ndiReceiver.preferExactName = ndi.value("preferExactName").toBool(config.overlay.ndiReceiver.preferExactName);

	const QJsonObject hotkey = overlay.value("hotkey").toObject();
	config.overlay.hotkey.enabled = hotkey.value("enabled").toBool(config.overlay.hotkey.enabled);
	for (const QJsonValue &value : hotkey.value("modifiers").toArray())
		config.overlay.hotkey.modifiers.append(value.toString());
	config.overlay.hotkey.modifiers = normalizeHotkeyModifiers(config.overlay.hotkey.modifiers);
	config.overlay.hotkey.key = hotkey.value("key").toString(config.overlay.hotkey.key).trimmed();
	if (!hotkeyHasBinding(config.overlay.hotkey))
		config.overlay.hotkey.enabled = false;

	const QJsonObject debug = root.value("debug").toObject();
	config.debug.enabled = debug.value("enabled").toBool(config.debug.enabled);

	const QJsonObject ui = root.value("ui").toObject();
	config.ui.language = normalizeAppLanguageSetting(ui.value("language").toString(config.ui.language));

	const QJsonObject onboarding = root.value("onboarding").toObject();
	config.onboarding.completed = onboarding.value("completed").toBool(config.onboarding.completed);
	return config;
}

} // namespace

QString runtimeDirPath()
{
	QDir cursor(QDir::current());
	while (true) {
		if (QFile::exists(cursor.filePath(kConfigFileName)))
			return cursor.absolutePath();
		if (!cursor.cdUp())
			break;
	}

	const QString appDirConfig = QCoreApplication::applicationDirPath() + "/" + kConfigFileName;
	if (QFile::exists(appDirConfig))
		return QCoreApplication::applicationDirPath();

	const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	QDir().mkpath(dataDir);
	return dataDir;
}

QString configFilePath()
{
	return runtimeDirPath() + "/" + kConfigFileName;
}

AppConfig loadOrInitConfig()
{
	const QString path = configFilePath();
	QFile file(path);
	if (!file.exists()) {
		AppConfig defaults;
		saveConfig(defaults);
		return defaults;
	}

	if (!file.open(QIODevice::ReadOnly))
		return AppConfig{};

	const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
	if (!document.isObject()) {
		AppConfig defaults;
		saveConfig(defaults);
		return defaults;
	}

	return fromJson(document.object());
}

bool saveConfig(const AppConfig &config, QString *error)
{
	QFile file(configFilePath());
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		if (error)
			*error = file.errorString();
		return false;
	}
	file.write(QJsonDocument(toJson(config)).toJson(QJsonDocument::Indented));
	return true;
}

int viewportAnchorIndex(const QString &anchor)
{
	const int index = viewportAnchors().indexOf(anchor);
	return index >= 0 ? index : 4;
}

QStringList viewportAnchors()
{
	return {
		"top-left", "top-center", "top-right",
		"middle-left", "middle-center", "middle-right",
		"bottom-left", "bottom-center", "bottom-right",
	};
}

QString normalizeAnchor(const QString &anchor)
{
	return viewportAnchors().contains(anchor) ? anchor : QString("middle-center");
}

QString normalizeAppLanguageSetting(const QString &language)
{
	const QString normalized = language.trimmed().toLower();
	if (normalized == "system" || normalized == "de" || normalized == "en")
		return normalized;
	return QString("system");
}

QString overlayInputModeToString(OverlayInputMode mode)
{
	return mode == OverlayInputMode::Ndi ? "ndi" : "spout";
}

OverlayInputMode overlayInputModeFromString(const QString &value)
{
	return value.trimmed().compare("ndi", Qt::CaseInsensitive) == 0
		? OverlayInputMode::Ndi
		: OverlayInputMode::Spout;
}

QStringList normalizeHotkeyModifiers(const QStringList &modifiers)
{
	QStringList normalized;
	const auto appendIfPresent = [&](const QString &needle) {
		for (const QString &value : modifiers) {
			if (value.trimmed().compare(needle, Qt::CaseInsensitive) == 0) {
				normalized.append(needle);
				return;
			}
		}
	};
	appendIfPresent("ctrl");
	appendIfPresent("alt");
	appendIfPresent("shift");
	appendIfPresent("win");
	return normalized;
}

bool hotkeyHasBinding(const OverlayHotkeyConfig &hotkey)
{
	return !hotkey.key.trimmed().isEmpty();
}
