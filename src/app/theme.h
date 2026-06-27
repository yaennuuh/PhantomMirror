#pragma once

#include <QString>

class QApplication;
class QTextBrowser;

enum class AppTheme {
	Dark,
	Light,
};

AppTheme systemAppTheme();
AppTheme resolveAppTheme(const QString &setting);
QString normalizeAppTheme(const QString &setting);
QString appThemeSetting(AppTheme theme);
void applyAppTheme(QApplication &app, AppTheme theme);
AppTheme currentAppTheme();
QString statusColorToken(AppTheme theme, const char *token);
QString onboardingDocumentStyleSheet(AppTheme theme);
