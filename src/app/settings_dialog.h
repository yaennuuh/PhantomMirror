#pragma once

#include "app_config.h"
#include "ndi_controller.h"
#include "app_strings.h"
#include "spout_controller.h"
#include "theme.h"
#include "theme_toggle.h"
#include "toggle_switch.h"

#include <QDialog>
class QButtonGroup;
class QComboBox;
class QGridLayout;
class HotkeyEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class QWidget;

class SettingsDialog : public QDialog {
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget *parent = nullptr);
	void setConfig(const AppConfig &config);
	void setAvailableSenders(const QStringList &senders);
	void setAvailableNdiSources(const QStringList &sources);
	AppConfig config() const;
	void setSpoutStatus(const SpoutStatus &status);
	void setNdiStatus(const NdiStatus &status);
	void setNdiAvailability(bool available, const QString &error = {});
	void setHotkeyStatus(const QString &status);
	void setUpdateStatus(const QString &status);

signals:
	void configSaved(const AppConfig &config);
	void languageChanged(const QString &languageSetting);
	void themeChanged(const QString &themeSetting);
	void onboardingRequested();
	void spoutSendersRefreshRequested();
	void spoutTestRequested();
	void ndiSourcesRefreshRequested();
	void ndiTestRequested();
	void updateCheckRequested();
	void quitRequested();

private:
	void setLanguage(AppLanguage language);
	AppTheme selectedTheme() const;
	void retranslateUi();
	void repopulateSenderOptions(const QString &selectedSender);
	void repopulateNdiSourceOptions(const QString &selectedSource);
	void syncSectionVisibility();
	void populateMonitorOptions(const QString &selectedMonitor);
	void save();
	void buildAnchorGrid(QGridLayout *layout);

	AppLanguage language_ = AppLanguage::English;
	bool syncingLanguageSelector_ = false;
	SpoutStatus lastSpoutStatus_;
	bool hasSpoutStatus_ = false;
	NdiStatus lastNdiStatus_;
	bool hasNdiStatus_ = false;
	bool ndiRuntimeAvailable_ = true;
	QString ndiRuntimeError_;
	QString updateStatusText_;
	QLabel *headerTitle_ = nullptr;
	QLabel *headerSubtitle_ = nullptr;
	QLabel *versionLabel_ = nullptr;
	QComboBox *languageSelector_ = nullptr;
	ThemeToggle *themeToggle_ = nullptr;
	QLabel *inputSectionTitle_ = nullptr;
	QComboBox *inputModeSelector_ = nullptr;
	QLabel *inputModeLabel_ = nullptr;
	ToggleSwitch *includeInScreenCapture_ = nullptr;
	QLabel *hotkeySectionTitle_ = nullptr;
	QLabel *hotkeyLabel_ = nullptr;
	HotkeyEdit *hotkeyEdit_ = nullptr;
	QPushButton *clearHotkeyButton_ = nullptr;
	QLabel *hotkeyStatus_ = nullptr;
	QLabel *spoutSectionTitle_ = nullptr;
	QWidget *spoutContent_ = nullptr;
	QComboBox *monitorSelector_ = nullptr;
	QLabel *monitorLabel_ = nullptr;
	ToggleSwitch *spoutEnabled_ = nullptr;
	QStringList availableSenders_;
	QComboBox *senderSelector_ = nullptr;
	QLabel *senderLabel_ = nullptr;
	QPushButton *refreshSendersButton_ = nullptr;
	QPushButton *testButton_ = nullptr;
	QLabel *spoutStatus_ = nullptr;
	QStringList availableNdiSources_;
	QLabel *ndiSectionTitle_ = nullptr;
	QWidget *ndiContent_ = nullptr;
	ToggleSwitch *ndiEnabled_ = nullptr;
	QWidget *ndiControls_ = nullptr;
	QComboBox *ndiSourceSelector_ = nullptr;
	QLabel *ndiSourceLabel_ = nullptr;
	QPushButton *refreshNdiSourcesButton_ = nullptr;
	QPushButton *ndiTestButton_ = nullptr;
	QLabel *ndiStatus_ = nullptr;
	QLabel *ndiRuntimeHint_ = nullptr;
	QPushButton *ndiInstallButton_ = nullptr;
	QLabel *viewportSectionTitle_ = nullptr;
	QWidget *viewportContent_ = nullptr;
	ToggleSwitch *viewportEnabled_ = nullptr;
	QSpinBox *viewportWidth_ = nullptr;
	QSpinBox *viewportHeight_ = nullptr;
	QLabel *viewportWidthLabel_ = nullptr;
	QLabel *viewportHeightLabel_ = nullptr;
	QLabel *viewportWidthUnit_ = nullptr;
	QLabel *viewportHeightUnit_ = nullptr;
	QLabel *anchorLabel_ = nullptr;
	QButtonGroup *anchorGroup_ = nullptr;
	QPushButton *creatorSupportButton_ = nullptr;
	QPushButton *helpButton_ = nullptr;
	QPushButton *updateButton_ = nullptr;
	QPushButton *quitButton_ = nullptr;
	QPushButton *cancelButton_ = nullptr;
	QPushButton *saveButton_ = nullptr;
	QLabel *creatorSupport_ = nullptr;
	QLabel *updateStatus_ = nullptr;
};
