#pragma once

#include "app_config.h"
#include "app_strings.h"
#include "ndi_controller.h"
#include "onboarding_dialog.h"
#include "settings_dialog.h"
#include "spout_controller.h"
#include "updater.h"

#include <QByteArray>
#include <QMainWindow>
#include <QSystemTrayIcon>

#include <windows.h>

class QAction;
class QMenu;
class QScreen;

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override;
	void showSettingsFromExternalActivation();

protected:
	void closeEvent(QCloseEvent *event) override;
	bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private:
	void applyLanguage();
	void applyOverlayVisibility();
	void toggleOverlayVisibility();
	void refreshHotkeyRegistration();
	void updateHotkeyStatus();
	void setupOverlayWindow();
	void setupTray();
	void openSettings();
	void openOnboarding(bool firstRun);
	void markOnboardingCompleted();
	void checkForUpdates(bool interactive);
	void requestQuitFromSettings();
	void installUpdate(const QString &version, const QUrl &assetUrl);
	void syncNdiAvailability();
	NdiStatus unavailableNdiStatus() const;
	void startOverlay();
	void stopOverlay();
	void reconnectOverlay();
	QScreen *targetScreen() const;
	QRect targetMonitorLogicalRect() const;
	QRect nativeRectForScreen(QScreen *screen) const;
	HWND hwnd() const;

	AppConfig config_;
	SpoutController spout_;
	NdiController ndi_;
	Updater updater_;
	OnboardingDialog onboarding_;
	SettingsDialog settings_;
	QSystemTrayIcon tray_;
	QMenu *trayMenu_ = nullptr;
	QAction *settingsAction_ = nullptr;
	QAction *setupHelpAction_ = nullptr;
	QAction *startOverlayAction_ = nullptr;
	QAction *stopOverlayAction_ = nullptr;
	QAction *reconnectSpoutAction_ = nullptr;
	QAction *checkUpdatesAction_ = nullptr;
	QAction *quitAction_ = nullptr;
	AppLanguage language_ = AppLanguage::English;
	QString hotkeyStatusText_;
	bool overlayVisible_ = true;
	bool overlayRunning_ = false;
	bool hotkeyRegistered_ = false;
	bool quitting_ = false;
};
