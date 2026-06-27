#include "app_icon.h"
#include "main_window.h"

#include "app_strings.h"
#include "theme.h"
#include "../native/ndi_overlay.h"
#include "../native/spout_overlay.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QKeySequence>
#include <QMenu>
#include <QMessageBox>
#include <QScreen>
#include <QStringList>
#include <QTimer>

#include <windows.h>

#ifndef WDA_EXCLUDEFROMCAPTURE
constexpr DWORD WDA_EXCLUDEFROMCAPTURE = 0x00000011;
#endif

namespace {

constexpr int kOverlayHotkeyId = 0x504D;

bool applyCaptureExclusion(HWND hwnd)
{
	if (!hwnd)
		return false;
	if (SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE))
		return true;
	return SetWindowDisplayAffinity(hwnd, WDA_MONITOR) == TRUE;
}

bool applyCapturePolicy(HWND hwnd, bool includeInScreenCapture)
{
	if (!hwnd)
		return false;
	if (includeInScreenCapture)
		return SetWindowDisplayAffinity(hwnd, WDA_NONE) == TRUE;
	return applyCaptureExclusion(hwnd);
}

QString monitorId(QScreen *screen, int index)
{
	if (!screen)
		return QString("screen:%1").arg(index);
	const QString name = screen->name().trimmed();
	if (!name.isEmpty())
		return name;
	const QRect geometry = screen->geometry();
	return QString("screen:%1:%2:%3:%4:%5")
		.arg(index)
		.arg(geometry.x())
		.arg(geometry.y())
		.arg(geometry.width())
		.arg(geometry.height());
}

QString hotkeyDisplayString(const OverlayHotkeyConfig &hotkey)
{
	if (!hotkeyHasBinding(hotkey))
		return {};

	int sequence = 0;
	for (const QString &modifier : normalizeHotkeyModifiers(hotkey.modifiers)) {
		if (modifier == "ctrl") sequence |= Qt::CTRL;
		else if (modifier == "alt") sequence |= Qt::ALT;
		else if (modifier == "shift") sequence |= Qt::SHIFT;
		else if (modifier == "win") sequence |= Qt::META;
	}
	const QKeySequence keySequence = QKeySequence::fromString(hotkey.key, QKeySequence::PortableText);
	if (!keySequence.isEmpty())
		sequence |= keySequence[0];
	return QKeySequence(sequence).toString(QKeySequence::NativeText);
}

UINT hotkeyModifiersToNative(const QStringList &modifiers)
{
	UINT native = MOD_NOREPEAT;
	for (const QString &modifier : normalizeHotkeyModifiers(modifiers)) {
		if (modifier == "ctrl") native |= MOD_CONTROL;
		else if (modifier == "alt") native |= MOD_ALT;
		else if (modifier == "shift") native |= MOD_SHIFT;
		else if (modifier == "win") native |= MOD_WIN;
	}
	return native;
}

UINT qtKeyToVirtualKey(const QString &portableKey)
{
	const QKeySequence keySequence = QKeySequence::fromString(portableKey, QKeySequence::PortableText);
	if (keySequence.isEmpty())
		return 0;

	const int qtKey = keySequence[0] & ~Qt::KeyboardModifierMask;
	if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)
		return static_cast<UINT>('A' + (qtKey - Qt::Key_A));
	if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)
		return static_cast<UINT>('0' + (qtKey - Qt::Key_0));
	if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F24)
		return static_cast<UINT>(VK_F1 + (qtKey - Qt::Key_F1));

	switch (qtKey) {
	case Qt::Key_Space: return VK_SPACE;
	case Qt::Key_Tab: return VK_TAB;
	case Qt::Key_Backtab: return VK_TAB;
	case Qt::Key_Return:
	case Qt::Key_Enter: return VK_RETURN;
	case Qt::Key_Escape: return VK_ESCAPE;
	case Qt::Key_Backspace: return VK_BACK;
	case Qt::Key_Insert: return VK_INSERT;
	case Qt::Key_Delete: return VK_DELETE;
	case Qt::Key_Home: return VK_HOME;
	case Qt::Key_End: return VK_END;
	case Qt::Key_PageUp: return VK_PRIOR;
	case Qt::Key_PageDown: return VK_NEXT;
	case Qt::Key_Left: return VK_LEFT;
	case Qt::Key_Right: return VK_RIGHT;
	case Qt::Key_Up: return VK_UP;
	case Qt::Key_Down: return VK_DOWN;
	case Qt::Key_Plus:
	case Qt::Key_Equal: return VK_OEM_PLUS;
	case Qt::Key_Minus: return VK_OEM_MINUS;
	case Qt::Key_Comma: return VK_OEM_COMMA;
	case Qt::Key_Period: return VK_OEM_PERIOD;
	case Qt::Key_Slash: return VK_OEM_2;
	case Qt::Key_Semicolon: return VK_OEM_1;
	case Qt::Key_Apostrophe: return VK_OEM_7;
	case Qt::Key_BracketLeft: return VK_OEM_4;
	case Qt::Key_BracketRight: return VK_OEM_6;
	case Qt::Key_Backslash: return VK_OEM_5;
	case Qt::Key_QuoteLeft: return VK_OEM_3;
	default: return 0;
	}
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, config_(loadOrInitConfig())
	, onboarding_()
	, settings_()
{
	language_ = resolveAppLanguage(config_.ui.language);
	syncNdiAvailability();
	settings_.setAvailableSenders(spout_.availableSenders());
	settings_.setAvailableNdiSources(ndi_.availableSources());
	settings_.setConfig(config_);
	applyTheme();
	onboarding_.setLanguage(language_);
	onboarding_.setLanguageSetting(config_.ui.language);
	updater_.setLanguage(language_);
	setupOverlayWindow();
	setupTray();
	refreshHotkeyRegistration();
	updateHotkeyStatus();

	connect(&settings_, &SettingsDialog::configSaved, this, [this](const AppConfig &nextConfig) {
		config_ = nextConfig;
		syncNdiAvailability();
		refreshHotkeyRegistration();
		updateHotkeyStatus();
		applyLanguage();
		reconnectOverlay();
	});
	connect(&settings_, &SettingsDialog::languageChanged, this, [this](const QString &languageSetting) {
		config_.ui.language = normalizeAppLanguageSetting(languageSetting);
		QString error;
		if (!saveConfig(config_, &error)) {
			settings_.setUpdateStatus(formatSaveError(language_, error));
			return;
		}
		applyLanguage();
	});
	connect(&settings_, &SettingsDialog::themeChanged, this, [this](const QString &themeSetting) {
		config_.ui.theme = normalizeAppThemeSetting(themeSetting);
		QString error;
		if (!saveConfig(config_, &error)) {
			settings_.setUpdateStatus(formatSaveError(language_, error));
			return;
		}
		applyTheme();
	});
	connect(&settings_, &SettingsDialog::spoutTestRequested, this, [this]() {
		settings_.setSpoutStatus(spout_.status());
	});
	connect(&settings_, &SettingsDialog::spoutSendersRefreshRequested, this, [this]() {
		settings_.setAvailableSenders(spout_.availableSenders());
	});
	connect(&settings_, &SettingsDialog::ndiTestRequested, this, [this]() {
		if (!ndi_.runtimeAvailability().available) {
			settings_.setNdiStatus(unavailableNdiStatus());
			return;
		}
		settings_.setNdiStatus(ndi_.status());
	});
	connect(&settings_, &SettingsDialog::ndiSourcesRefreshRequested, this, [this]() {
		syncNdiAvailability();
		settings_.setAvailableNdiSources(ndi_.availableSources());
	});
	connect(&settings_, &SettingsDialog::onboardingRequested, this, [this]() {
		openOnboarding(false);
	});
	connect(&settings_, &SettingsDialog::updateCheckRequested, this, [this]() {
		checkForUpdates(true);
	});
	connect(&settings_, &SettingsDialog::quitRequested, this, &MainWindow::requestQuitFromSettings);
	connect(&onboarding_, &OnboardingDialog::completed, this, &MainWindow::markOnboardingCompleted);
	connect(&onboarding_, &OnboardingDialog::languageChanged, this, [this](const QString &languageSetting) {
		config_.ui.language = normalizeAppLanguageSetting(languageSetting);
		QString error;
		if (!saveConfig(config_, &error)) {
			settings_.setUpdateStatus(formatSaveError(language_, error));
			return;
		}
		applyLanguage();
	});
	connect(&spout_, &SpoutController::statusChanged, &settings_, &SettingsDialog::setSpoutStatus);
	connect(&ndi_, &NdiController::statusChanged, &settings_, &SettingsDialog::setNdiStatus);
	connect(&updater_, &Updater::updateStatusChanged, &settings_, &SettingsDialog::setUpdateStatus);
	connect(&updater_, &Updater::updateFailed, this, [this](const QString &message) {
		settings_.setUpdateStatus(message);
	});
	connect(&updater_, &Updater::noUpdateAvailable, this, [this]() {
		if (settings_.isVisible())
			QMessageBox::information(&settings_, text(TextId::UpdateDialogTitle, language_),
									 text(TextId::UpdateNoUpdates, language_));
	});
	connect(&updater_, &Updater::updateAvailable, this, &MainWindow::installUpdate);

	startOverlay();
	if (!config_.onboarding.completed) {
		QTimer::singleShot(500, this, [this]() {
			openOnboarding(true);
		});
	}
	QTimer::singleShot(3000, this, [this]() {
		checkForUpdates(false);
	});
}

MainWindow::~MainWindow()
{
	UnregisterHotKey(hwnd(), kOverlayHotkeyId);
	spout_.stop();
	ndi_.stop();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (!quitting_) {
		event->ignore();
		hide();
		return;
	}
	QMainWindow::closeEvent(event);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
	Q_UNUSED(eventType);
	MSG *msg = static_cast<MSG *>(message);
	if (msg && msg->message == WM_HOTKEY && msg->wParam == kOverlayHotkeyId) {
		toggleOverlayVisibility();
		if (result)
			*result = 0;
		return true;
	}
	return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::applyOverlayVisibility()
{
	applyCapturePolicy(hwnd(), config_.overlay.includeInScreenCapture);
	ShowWindow(hwnd(), overlayVisible_ ? SW_SHOWNOACTIVATE : SW_HIDE);
	phantom_mirror_spout_set_visible(overlayVisible_ ? 1 : 0);
	phantom_mirror_ndi_set_visible(overlayVisible_ ? 1 : 0);
}

void MainWindow::toggleOverlayVisibility()
{
	if (!overlayRunning_)
		return;
	overlayVisible_ = !overlayVisible_;
	applyOverlayVisibility();
}

void MainWindow::refreshHotkeyRegistration()
{
	UnregisterHotKey(hwnd(), kOverlayHotkeyId);
	hotkeyRegistered_ = false;

	if (!config_.overlay.hotkey.enabled || !hotkeyHasBinding(config_.overlay.hotkey))
		return;

	const UINT modifiers = hotkeyModifiersToNative(config_.overlay.hotkey.modifiers);
	const UINT vk = qtKeyToVirtualKey(config_.overlay.hotkey.key);
	if (vk == 0)
		return;

	hotkeyRegistered_ = RegisterHotKey(hwnd(), kOverlayHotkeyId, modifiers, vk) == TRUE;
	if (!hotkeyRegistered_ && (modifiers & MOD_NOREPEAT)) {
		const UINT fallbackModifiers = modifiers & ~MOD_NOREPEAT;
		hotkeyRegistered_ = RegisterHotKey(hwnd(), kOverlayHotkeyId, fallbackModifiers, vk) == TRUE;
	}
}

void MainWindow::updateHotkeyStatus()
{
	if (!config_.overlay.hotkey.enabled || !hotkeyHasBinding(config_.overlay.hotkey)) {
		hotkeyStatusText_ = QString("<span style='color:#40406a'>%1</span>")
			.arg(text(TextId::HotkeyUnset, language_).toHtmlEscaped());
	} else {
		const QString display = hotkeyDisplayString(config_.overlay.hotkey).toHtmlEscaped();
		if (hotkeyRegistered_) {
			hotkeyStatusText_ = QString("<span style='color:#10b981'>&#9679;</span>"
				"<span style='color:#94a3b8'>  %1</span>")
				.arg(text(TextId::HotkeyActive, language_).arg(display).toHtmlEscaped());
		} else {
			hotkeyStatusText_ = QString("<span style='color:#f59e0b'>&#9679;</span>"
				"<span style='color:#ef4444'>  %1</span><br/><span style='color:#52527a'>%2</span>")
				.arg(text(TextId::HotkeyConflict, language_).arg(display).toHtmlEscaped(),
				     text(TextId::HotkeySavedInactive, language_).toHtmlEscaped());
		}
	}
	settings_.setHotkeyStatus(hotkeyStatusText_);
}

void MainWindow::setupOverlayWindow()
{
	setWindowTitle("Phantom Mirror");
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_ShowWithoutActivating, true);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);
	setGeometry(targetMonitorLogicalRect());
	show();

	HWND handle = hwnd();
	LONG_PTR style = GetWindowLongPtrW(handle, GWL_EXSTYLE);
	style |= WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
	SetWindowLongPtrW(handle, GWL_EXSTYLE, style);
	applyCapturePolicy(handle, config_.overlay.includeInScreenCapture);
	SetWindowPos(handle, HWND_TOPMOST, x(), y(), width(), height(), SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void MainWindow::setupTray()
{
	trayMenu_ = new QMenu(this);
	settingsAction_ = trayMenu_->addAction(QString(), this, &MainWindow::openSettings);
	setupHelpAction_ = trayMenu_->addAction(QString(), this, [this]() {
		openOnboarding(false);
	});
	trayMenu_->addSeparator();
	startOverlayAction_ = trayMenu_->addAction(QString(), this, &MainWindow::startOverlay);
	stopOverlayAction_ = trayMenu_->addAction(QString(), this, &MainWindow::stopOverlay);
	reconnectSpoutAction_ = trayMenu_->addAction(QString(), this, &MainWindow::reconnectOverlay);
	checkUpdatesAction_ = trayMenu_->addAction(QString(), this, [this]() {
		checkForUpdates(true);
	});
	trayMenu_->addSeparator();
	quitAction_ = trayMenu_->addAction(QString(), this, [this]() {
		quitting_ = true;
		spout_.stop();
		qApp->quit();
	});

	tray_.setContextMenu(trayMenu_);
	tray_.setToolTip("Phantom Mirror");
	tray_.setIcon(appIcon());
	connect(&tray_, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::Trigger)
			openSettings();
	});
	tray_.show();
	applyLanguage();
}

void MainWindow::openSettings()
{
	syncNdiAvailability();
	settings_.setAvailableSenders(spout_.availableSenders());
	settings_.setAvailableNdiSources(ndi_.availableSources());
	settings_.setConfig(config_);
	settings_.setHotkeyStatus(hotkeyStatusText_);
	settings_.show();
	settings_.raise();
	settings_.activateWindow();
}

void MainWindow::showSettingsFromExternalActivation()
{
	if (onboarding_.isVisible()) {
		onboarding_.showNormal();
		onboarding_.raise();
		onboarding_.activateWindow();
	}
	openSettings();
}

void MainWindow::openOnboarding(bool firstRun)
{
	onboarding_.show();
	onboarding_.raise();
	onboarding_.activateWindow();
	if (!firstRun)
		return;
	connect(&onboarding_, &QDialog::accepted, this, &MainWindow::openSettings, Qt::SingleShotConnection);
}

void MainWindow::markOnboardingCompleted()
{
	config_.onboarding.completed = true;
	QString error;
	if (!saveConfig(config_, &error))
		settings_.setUpdateStatus(formatOnboardingSaveError(language_, error));
}

void MainWindow::checkForUpdates(bool interactive)
{
	if (interactive)
		openSettings();
	updater_.checkForUpdates(interactive);
}

void MainWindow::installUpdate(const QString &version, const QUrl &assetUrl)
{
	const QMessageBox::StandardButton answer = QMessageBox::question(
		&settings_,
		text(TextId::UpdateDialogTitle, language_),
		formatUpdatePrompt(language_, version));
	if (answer != QMessageBox::Yes) {
		settings_.setUpdateStatus(formatUpdateAvailableStatus(language_, version));
		return;
	}
	updater_.downloadAndInstall(version, assetUrl);
}

void MainWindow::requestQuitFromSettings()
{
	const QMessageBox::StandardButton answer = QMessageBox::question(
		&settings_,
		text(TextId::QuitConfirmTitle, language_),
		text(TextId::QuitConfirmText, language_),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No);
	if (answer != QMessageBox::Yes)
		return;

	quitting_ = true;
	spout_.stop();
	ndi_.stop();
	tray_.hide();
	qApp->quit();
}

void MainWindow::syncNdiAvailability()
{
	const NdiRuntimeAvailability availability = ndi_.runtimeAvailability();
	settings_.setNdiAvailability(availability.available, availability.error);
}

NdiStatus MainWindow::unavailableNdiStatus() const
{
	const NdiRuntimeAvailability availability = ndi_.runtimeAvailability();
	NdiStatus status;
	status.lastError = text(TextId::NdiInstallRequired, language_);
	if (!availability.error.trimmed().isEmpty())
		status.lastError += QString(" (%1)").arg(availability.error.trimmed());
	return status;
}

void MainWindow::applyLanguage()
{
	language_ = resolveAppLanguage(config_.ui.language);
	syncNdiAvailability();
	settings_.setConfig(config_);
	updateHotkeyStatus();
	onboarding_.setLanguage(language_);
	onboarding_.setLanguageSetting(config_.ui.language);
	updater_.setLanguage(language_);
	setWindowTitle("Phantom Mirror");

	if (settingsAction_)
		settingsAction_->setText(text(TextId::TraySettings, language_));
	if (setupHelpAction_)
		setupHelpAction_->setText(text(TextId::TraySetupHelp, language_));
	if (startOverlayAction_)
		startOverlayAction_->setText(text(TextId::TrayStartOverlay, language_));
	if (stopOverlayAction_)
		stopOverlayAction_->setText(text(TextId::TrayStopOverlay, language_));
	if (reconnectSpoutAction_)
		reconnectSpoutAction_->setText(text(TextId::TrayReconnectSpout, language_));
	if (checkUpdatesAction_)
		checkUpdatesAction_->setText(text(TextId::TrayCheckForUpdates, language_));
	if (quitAction_)
		quitAction_->setText(text(TextId::TrayQuit, language_));
}

void MainWindow::applyTheme()
{
	applyAppTheme(*qApp, resolveAppTheme(config_.ui.theme));
	settings_.setConfig(config_);
	onboarding_.setTheme(resolveAppTheme(config_.ui.theme));
}

void MainWindow::startOverlay()
{
	syncNdiAvailability();
	setGeometry(targetMonitorLogicalRect());
	overlayVisible_ = true;
	const QRect target = nativeRectForScreen(targetScreen());
	if (config_.overlay.input == OverlayInputMode::Ndi) {
		spout_.stop();
		if (!ndi_.start(hwnd(), config_, target))
			settings_.setNdiStatus(unavailableNdiStatus());
		overlayRunning_ = ndi_.isEnabled();
	} else {
		ndi_.stop();
		spout_.start(hwnd(), config_, target);
		overlayRunning_ = spout_.isEnabled();
	}
	applyOverlayVisibility();
}

void MainWindow::stopOverlay()
{
	overlayVisible_ = false;
	overlayRunning_ = false;
	applyOverlayVisibility();
	spout_.stop();
	ndi_.stop();
}

void MainWindow::reconnectOverlay()
{
	syncNdiAvailability();
	const bool wasVisible = overlayVisible_;
	spout_.stop();
	ndi_.stop();
	overlayRunning_ = false;
	startOverlay();
	overlayVisible_ = wasVisible;
	applyOverlayVisibility();
}

QScreen *MainWindow::targetScreen() const
{
	const QList<QScreen *> screens = QGuiApplication::screens();
	if (config_.overlay.monitor != "primary") {
		for (int i = 0; i < screens.size(); ++i) {
			QScreen *screen = screens.at(i);
			if (screen && monitorId(screen, i) == config_.overlay.monitor)
				return screen;
		}
	}
	if (QScreen *screen = QGuiApplication::primaryScreen())
		return screen;
	return screens.isEmpty() ? nullptr : screens.front();
}

QRect MainWindow::targetMonitorLogicalRect() const
{
	if (QScreen *screen = targetScreen())
		return screen->geometry();
	return QRect(0, 0, 1920, 1080);
}

QRect MainWindow::nativeRectForScreen(QScreen *screen) const
{
	const QRect fallback = targetMonitorLogicalRect();
	if (!screen)
		return fallback;

	const QPoint center = screen->geometry().center();
	const POINT point{center.x(), center.y()};
	const HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info{};
	info.cbSize = sizeof(info);
	if (monitor && GetMonitorInfoW(monitor, &info)) {
		const RECT rect = info.rcMonitor;
		return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
	}
	return fallback;
}

HWND MainWindow::hwnd() const
{
	return reinterpret_cast<HWND>(winId());
}
