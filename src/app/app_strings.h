#pragma once

#include <QString>

class QScreen;

enum class AppLanguage {
	English,
	German,
};

enum class TextId {
	LanguageSectionTitle,
	LanguageLabel,
	LanguageOptionSystem,
	LanguageOptionGerman,
	LanguageOptionEnglish,
	SettingsWindowTitle,
	SettingsHeaderTitle,
	SettingsHeaderSubtitle,
	SettingsVersionLabel,
	HotkeySectionTitle,
	HotkeyLabel,
	HotkeyPlaceholder,
	HotkeyClear,
	HotkeyUnset,
	HotkeyActive,
	HotkeyInactive,
	HotkeyConflict,
	HotkeySavedInactive,
	InputSectionTitle,
	InputModeLabel,
	InputModeSpout,
	InputModeNdi,
	IncludeInScreenCapture,
	SpoutSectionTitle,
	SpoutEnabled,
	MonitorLabel,
	SenderLabel,
	SenderRefresh,
	SenderCustom,
	SenderCustomNameLabel,
	SenderNameLabel,
	SenderPlaceholder,
	TestConnection,
	SpoutNotTested,
	NdiSectionTitle,
	NdiEnabled,
	NdiSourceLabel,
	NdiSourceCustom,
	NdiSourceNameLabel,
	NdiSourcePlaceholder,
	NdiNotTested,
	NdiConnected,
	NdiWaiting,
	NdiReceiverStopped,
	NdiRuntimeMissing,
	NdiInstallRequired,
	NdiInstallAction,
	ViewportSectionTitle,
	ViewportEnabled,
	WidthLabel,
	HeightLabel,
	PixelsSuffix,
	AnchorPositionLabel,
	DebugSectionTitle,
	DebugEnabled,
	SetupHelp,
	SupportOnTwitch,
	Updates,
	Cancel,
	Save,
	OnboardingWindowTitle,
	OnboardingHeaderTitle,
	OnboardingHeaderSubtitle,
	OnboardingDontShow,
	OnboardingClose,
	OnboardingOpenSettings,
	TraySettings,
	TraySetupHelp,
	TrayStartOverlay,
	TrayStopOverlay,
	TrayReconnectSpout,
	TrayCheckForUpdates,
	TrayQuit,
	SettingsQuitApp,
	QuitConfirmTitle,
	QuitConfirmText,
	UpdateDialogTitle,
	UpdateNoUpdates,
	UpdatePromptQuestion,
	UpdateAvailableStatus,
	OnboardingSaveErrorPrefix,
	SaveErrorTitle,
	SaveErrorText,
	SpoutConnected,
	SpoutWaiting,
	SpoutReceiverStopped,
	MonitorName,
	DisplayName,
};

AppLanguage systemAppLanguage();
AppLanguage resolveAppLanguage(const QString &setting);
QString normalizeAppLanguage(const QString &setting);
QString text(TextId id, AppLanguage language);
QString monitorLabel(QScreen *screen, int index, AppLanguage language);
QString onboardingHtml(AppLanguage language);
QString creatorSupportHtml(AppLanguage language);
QString formatUpdatePrompt(AppLanguage language, const QString &version);
QString formatUpdateAvailableStatus(AppLanguage language, const QString &version);
QString formatOnboardingSaveError(AppLanguage language, const QString &error);
QString formatSaveError(AppLanguage language, const QString &error);
