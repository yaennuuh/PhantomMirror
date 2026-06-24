#pragma once

#include <QString>
#include <QStringList>

struct VirtualViewportConfig {
	bool enabled = false;
	int width = 1920;
	int height = 1080;
	QString anchor = "middle-center";
};

struct SpoutReceiverConfig {
	bool enabled = false;
	QString senderName = "OBS Spout2 Output";
};

enum class OverlayInputMode {
	Spout,
	Ndi,
};

struct NdiReceiverConfig {
	bool enabled = false;
	QString sourceName;
	bool preferExactName = true;
};

struct OverlayHotkeyConfig {
	bool enabled = false;
	QStringList modifiers;
	QString key;
};

struct OverlayConfig {
	OverlayInputMode input = OverlayInputMode::Spout;
	QString monitor = "primary";
	bool includeInScreenCapture = false;
	bool alwaysOnTop = true;
	bool clickThrough = true;
	VirtualViewportConfig virtualViewport;
	SpoutReceiverConfig spoutReceiver;
	NdiReceiverConfig ndiReceiver;
	OverlayHotkeyConfig hotkey;
};

struct DebugConfig {
	bool enabled = false;
};

struct UiConfig {
	QString language = "system";
};

struct OnboardingConfig {
	bool completed = false;
};

struct AppConfig {
	OverlayConfig overlay;
	DebugConfig debug;
	UiConfig ui;
	OnboardingConfig onboarding;
};

QString runtimeDirPath();
QString configFilePath();
AppConfig loadOrInitConfig();
bool saveConfig(const AppConfig &config, QString *error = nullptr);
int viewportAnchorIndex(const QString &anchor);
QStringList viewportAnchors();
QString normalizeAnchor(const QString &anchor);
QString normalizeAppLanguageSetting(const QString &language);
QString overlayInputModeToString(OverlayInputMode mode);
OverlayInputMode overlayInputModeFromString(const QString &value);
QStringList normalizeHotkeyModifiers(const QStringList &modifiers);
bool hotkeyHasBinding(const OverlayHotkeyConfig &hotkey);
