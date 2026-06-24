#pragma once

#include "app_strings.h"

#include <QDialog>

class QComboBox;
class QLabel;
class QPushButton;
class QTextBrowser;
class ToggleSwitch;

class OnboardingDialog : public QDialog {
	Q_OBJECT

public:
	explicit OnboardingDialog(QWidget *parent = nullptr);
	void setLanguage(AppLanguage language);
	void setLanguageSetting(const QString &languageSetting);

signals:
	void completed();
	void languageChanged(const QString &languageSetting);

private:
	void retranslateUi();

	AppLanguage language_ = AppLanguage::English;
	bool syncingLanguageSelector_ = false;
	QComboBox *languageSelector_ = nullptr;
	QLabel *headerTitle_ = nullptr;
	QLabel *headerSubtitle_ = nullptr;
	QTextBrowser *content_ = nullptr;
	ToggleSwitch *dontShow_ = nullptr;
	QPushButton *closeButton_ = nullptr;
	QPushButton *settingsButton_ = nullptr;
};
