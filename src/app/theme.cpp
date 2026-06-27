#include "theme.h"

#include <QApplication>
#include <QSettings>

namespace {

const char *kDarkTheme = R"(
QDialog {
    background-color: #111118;
}
QWidget {
    font-family: 'Segoe UI Variable Text', 'Segoe UI', Arial, sans-serif;
    font-size: 13px;
    color: #e2e8f0;
}
QFrame#card {
    background-color: #191924;
    border: 1px solid #26263a;
    border-radius: 12px;
}
QLabel {
    background: transparent;
    color: #e2e8f0;
}
QLabel#sectionTitle {
    color: #8b5cf6;
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 1.5px;
    background: transparent;
}
QLabel#mutedLabel {
    color: #52527a;
    font-size: 12px;
    background: transparent;
}
QLabel#statusLabel {
    font-size: 12px;
    background: transparent;
}
QLabel#headerTitle {
    font-size: 17px;
    font-weight: 700;
    color: #e8e8f8;
}
QLabel#headerSubtitle {
    font-size: 12px;
    color: #44446a;
}
QLabel#versionLabel {
    font-size: 12px;
    color: #6b6b94;
}
QFrame#cardSep  { background-color: #23233a; border: none; max-height: 1px; }
QFrame#dialogSep{ background-color: #1a1a28; border: none; max-height: 1px; }
QLineEdit {
    background-color: #0e0e18;
    border: 1.5px solid #2a2a40;
    border-radius: 9px;
    padding: 0 14px;
    color: #dde3f0;
    selection-background-color: #7c3aed;
    selection-color: #fff;
    min-height: 40px;
    font-size: 13px;
}
QLineEdit:hover  { border-color: #3c3c5e; }
QLineEdit:focus  { border-color: #7c3aed; background-color: #121222; }
QSpinBox {
    background-color: #0e0e18;
    border: 1.5px solid #2a2a40;
    border-radius: 9px;
    padding: 0 10px;
    color: #dde3f0;
    selection-background-color: #7c3aed;
    min-height: 40px;
    font-size: 13px;
}
QSpinBox:hover  { border-color: #3c3c5e; }
QSpinBox:focus  { border-color: #7c3aed; background-color: #121222; }
QSpinBox::up-button, QSpinBox::down-button {
    width: 0;
    border: none;
    background: transparent;
}
QComboBox {
    background-color: #0e0e18;
    border: 1.5px solid #2a2a40;
    border-radius: 9px;
    padding: 0 42px 0 14px;
    color: #dde3f0;
    min-height: 40px;
    font-size: 13px;
}
QComboBox:hover { border-color: #3c3c5e; }
QComboBox:focus { border-color: #7c3aed; background-color: #121222; }
QComboBox::drop-down {
    border: none;
    width: 34px;
    border-top-right-radius: 8px;
    border-bottom-right-radius: 8px;
    background-color: transparent;
    subcontrol-origin: padding;
    subcontrol-position: top right;
}
QComboBox::down-arrow {
    image: none;
    width: 0;
    height: 0;
}
QComboBox QAbstractItemView {
    background-color: #191924;
    border: 1.5px solid #3a3a56;
    border-radius: 9px;
    selection-background-color: #7c3aed;
    selection-color: #fff;
    color: #dde3f0;
    padding: 4px;
    outline: none;
}
QCheckBox {
    spacing: 10px;
    color: #a0a0c0;
    font-size: 12px;
    background: transparent;
}
QCheckBox::indicator {
    width: 17px; height: 17px;
    border-radius: 5px;
    border: 2px solid #3a3a56;
    background-color: #1a1a2a;
}
QCheckBox::indicator:hover  { border-color: #7c3aed; }
QCheckBox::indicator:checked {
    background-color: #7c3aed;
    border-color: #7c3aed;
}
QPushButton {
    background-color: #1c1c2c;
    color: #9090b8;
    border: 1.5px solid #28283e;
    border-radius: 9px;
    padding: 0 18px;
    font-size: 13px;
    font-weight: 500;
    min-height: 36px;
    min-width: 80px;
}
QPushButton:hover  { background-color: #232336; border-color: #3a3a58; color: #e2e8f0; }
QPushButton:pressed{ background-color: #141420; }
QPushButton:disabled { color: #2a2a44; border-color: #1a1a28; background-color: #111120; }
QPushButton[role="primary"] {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #9061ff, stop:1 #7c3aed);
    border: 1px solid #6d28d9;
    color: #fff;
    font-weight: 600;
}
QPushButton[role="primary"]:hover {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #a070ff, stop:1 #8b5cf6);
    border-color: #8b5cf6;
}
QPushButton[role="primary"]:pressed {
    background-color: #6d28d9;
    border-color: #5b21b6;
}
QPushButton#anchorBtn {
    background-color: #141420;
    border: 1.5px solid #222234;
    border-radius: 6px;
    min-width: 40px; max-width: 40px;
    min-height: 40px; max-height: 40px;
    padding: 0;
    font-size: 16px;
    color: #2e2e50;
}
QPushButton#anchorBtn:hover   { background-color: #1e1e30; border-color: #3a3a58; color: #7070a8; }
QPushButton#anchorBtn:checked {
    background-color: #7c3aed;
    border-color: #8b5cf6;
    color: #fff;
}
QPushButton#anchorBtn:checked:hover { background-color: #8b5cf6; }
QTextBrowser {
    background-color: #191924;
    border: 1px solid #26263a;
    border-radius: 12px;
    color: #c4c4d8;
    selection-background-color: #7c3aed;
    selection-color: #fff;
    padding: 6px;
    font-size: 13px;
}
QScrollBar:vertical { background: transparent; width: 5px; margin: 0; }
QScrollBar::handle:vertical { background-color: #2a2a42; border-radius: 3px; min-height: 20px; }
QScrollBar::handle:vertical:hover { background-color: #3a3a58; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical,  QScrollBar::sub-page:vertical { background:none; height:0; width:0; }
QMenu {
    background-color: #1a1a26;
    border: 1px solid #26263a;
    border-radius: 10px;
    padding: 5px;
    color: #c4c4d8;
}
QMenu::item { padding: 7px 20px; border-radius: 6px; }
QMenu::item:selected { background-color: #232336; color: #e2e8f0; }
QMenu::separator { height: 1px; background-color: #26263a; margin: 4px 8px; }
QMessageBox { background-color: #111118; }
QMessageBox QLabel { color: #e2e8f0; }
QMessageBox QPushButton { min-width: 80px; }
)";

const char *kLightTheme = R"(
QDialog {
    background-color: #f3f5fa;
}
QWidget {
    font-family: 'Segoe UI Variable Text', 'Segoe UI', Arial, sans-serif;
    font-size: 13px;
    color: #162033;
}
QFrame#card {
    background-color: #ffffff;
    border: 1px solid #cfd8e6;
    border-radius: 12px;
}
QLabel {
    background: transparent;
    color: #162033;
}
QLabel#sectionTitle {
    color: #6d28d9;
    font-size: 10px;
    font-weight: 700;
    letter-spacing: 1.5px;
    background: transparent;
}
QLabel#mutedLabel {
    color: #46556f;
    font-size: 12px;
    background: transparent;
}
QLabel#statusLabel {
    font-size: 12px;
    background: transparent;
}
QLabel#headerTitle {
    font-size: 17px;
    font-weight: 700;
    color: #111827;
}
QLabel#headerSubtitle {
    font-size: 12px;
    color: #4b5563;
}
QLabel#versionLabel {
    font-size: 12px;
    color: #55627c;
}
QFrame#cardSep  { background-color: #dde4ef; border: none; max-height: 1px; }
QFrame#dialogSep{ background-color: #d4ddea; border: none; max-height: 1px; }
QLineEdit {
    background-color: #ffffff;
    border: 1.5px solid #bcc9da;
    border-radius: 9px;
    padding: 0 14px;
    color: #162033;
    selection-background-color: #7c3aed;
    selection-color: #fff;
    min-height: 40px;
    font-size: 13px;
}
QLineEdit:hover  { border-color: #8ea0ba; }
QLineEdit:focus  { border-color: #7c3aed; background-color: #fcfbff; }
QSpinBox {
    background-color: #ffffff;
    border: 1.5px solid #bcc9da;
    border-radius: 9px;
    padding: 0 10px;
    color: #162033;
    selection-background-color: #7c3aed;
    min-height: 40px;
    font-size: 13px;
}
QSpinBox:hover  { border-color: #8ea0ba; }
QSpinBox:focus  { border-color: #7c3aed; background-color: #fcfbff; }
QSpinBox::up-button, QSpinBox::down-button {
    width: 0;
    border: none;
    background: transparent;
}
QComboBox {
    background-color: #ffffff;
    border: 1.5px solid #bcc9da;
    border-radius: 9px;
    padding: 0 42px 0 14px;
    color: #162033;
    min-height: 40px;
    font-size: 13px;
}
QComboBox:hover { border-color: #8ea0ba; }
QComboBox:focus { border-color: #7c3aed; background-color: #fcfbff; }
QComboBox::drop-down {
    border: none;
    width: 34px;
    border-top-right-radius: 8px;
    border-bottom-right-radius: 8px;
    background-color: transparent;
    subcontrol-origin: padding;
    subcontrol-position: top right;
}
QComboBox::down-arrow {
    image: none;
    width: 0;
    height: 0;
}
QComboBox QAbstractItemView {
    background-color: #ffffff;
    border: 1.5px solid #bcc9da;
    border-radius: 9px;
    selection-background-color: #7c3aed;
    selection-color: #fff;
    color: #162033;
    padding: 4px;
    outline: none;
}
QCheckBox {
    spacing: 10px;
    color: #334155;
    font-size: 12px;
    background: transparent;
}
QCheckBox::indicator {
    width: 17px; height: 17px;
    border-radius: 5px;
    border: 2px solid #94a3b8;
    background-color: #ffffff;
}
QCheckBox::indicator:hover  { border-color: #7c3aed; }
QCheckBox::indicator:checked {
    background-color: #7c3aed;
    border-color: #7c3aed;
}
QPushButton {
    background-color: #edf2f8;
    color: #334155;
    border: 1.5px solid #c7d2e1;
    border-radius: 9px;
    padding: 0 18px;
    font-size: 13px;
    font-weight: 500;
    min-height: 36px;
    min-width: 80px;
}
QPushButton:hover  { background-color: #e2eaf5; border-color: #94a3b8; color: #0f172a; }
QPushButton:pressed{ background-color: #d7e0ee; }
QPushButton:disabled { color: #7c8aa0; border-color: #d8e0ea; background-color: #eef3f8; }
QPushButton[role="primary"] {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #9061ff, stop:1 #7c3aed);
    border: 1px solid #6d28d9;
    color: #fff;
    font-weight: 600;
}
QPushButton[role="primary"]:hover {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #a070ff, stop:1 #8b5cf6);
    border-color: #8b5cf6;
}
QPushButton[role="primary"]:pressed {
    background-color: #6d28d9;
    border-color: #5b21b6;
}
QPushButton#anchorBtn {
    background-color: #edf2f8;
    border: 1.5px solid #c7d2e1;
    border-radius: 6px;
    min-width: 40px; max-width: 40px;
    min-height: 40px; max-height: 40px;
    padding: 0;
    font-size: 16px;
    color: #64748b;
}
QPushButton#anchorBtn:hover   { background-color: #e2eaf5; border-color: #94a3b8; color: #334155; }
QPushButton#anchorBtn:checked {
    background-color: #7c3aed;
    border-color: #8b5cf6;
    color: #fff;
}
QPushButton#anchorBtn:checked:hover { background-color: #8b5cf6; }
QTextBrowser {
    background-color: #ffffff;
    border: 1px solid #cfd8e6;
    border-radius: 12px;
    color: #334155;
    selection-background-color: #7c3aed;
    selection-color: #fff;
    padding: 6px;
    font-size: 13px;
}
QScrollBar:vertical { background: transparent; width: 5px; margin: 0; }
QScrollBar::handle:vertical { background-color: #aebcd0; border-radius: 3px; min-height: 20px; }
QScrollBar::handle:vertical:hover { background-color: #8ea0ba; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical,  QScrollBar::sub-page:vertical { background:none; height:0; width:0; }
QMenu {
    background-color: #ffffff;
    border: 1px solid #cfd8e6;
    border-radius: 10px;
    padding: 5px;
    color: #334155;
}
QMenu::item { padding: 7px 20px; border-radius: 6px; }
QMenu::item:selected { background-color: #e8eef7; color: #111827; }
QMenu::separator { height: 1px; background-color: #cfd8e6; margin: 4px 8px; }
QMessageBox { background-color: #f3f5fa; }
QMessageBox QLabel { color: #162033; }
QMessageBox QPushButton { min-width: 80px; }
)";

} // namespace

AppTheme resolveAppTheme(const QString &setting)
{
	const QString normalized = normalizeAppTheme(setting);
	if (normalized == "light")
		return AppTheme::Light;
	if (normalized == "dark")
		return AppTheme::Dark;
	return systemAppTheme();
}

QString normalizeAppTheme(const QString &setting)
{
	const QString normalized = setting.trimmed().toLower();
	if (normalized == "light" || normalized == "dark" || normalized == "system")
		return normalized;
	return "system";
}

AppTheme systemAppTheme()
{
#ifdef Q_OS_WIN
	QSettings personalize("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
	                      QSettings::NativeFormat);
	const QVariant value = personalize.value("AppsUseLightTheme");
	if (value.isValid())
		return value.toInt() == 0 ? AppTheme::Dark : AppTheme::Light;
#endif
	return AppTheme::Dark;
}

QString appThemeSetting(AppTheme theme)
{
	return theme == AppTheme::Light ? "light" : "dark";
}

void applyAppTheme(QApplication &app, AppTheme theme)
{
	app.setProperty("appTheme", appThemeSetting(theme));
	app.setStyleSheet(theme == AppTheme::Light ? kLightTheme : kDarkTheme);
}

AppTheme currentAppTheme()
{
	if (qApp && qApp->property("appTheme").toString() == "light")
		return AppTheme::Light;
	return AppTheme::Dark;
}

QString statusColorToken(AppTheme theme, const char *token)
{
	const QString key = QString::fromLatin1(token);
	if (theme == AppTheme::Light) {
		if (key == "successText") return "#334155";
		if (key == "accentText") return "#0f172a";
		if (key == "mutedText") return "#475569";
		if (key == "idleDot") return "#94a3b8";
		if (key == "idleText") return "#55627c";
		if (key == "warningText") return "#7c2d12";
		if (key == "detailText") return "#46556f";
		if (key == "errorText") return "#dc2626";
	}
	if (key == "successText") return "#94a3b8";
	if (key == "accentText") return "#c8d4f0";
	if (key == "mutedText") return "#52527a";
	if (key == "idleDot") return "#2e2e50";
	if (key == "idleText") return "#40406a";
	if (key == "warningText") return "#f59e0b";
	if (key == "detailText") return "#52527a";
	if (key == "errorText") return "#ef4444";
	return "#7c3aed";
}

QString onboardingDocumentStyleSheet(AppTheme theme)
{
	if (theme == AppTheme::Light) {
		return R"(
		body  { color:#334155; font-family:'Segoe UI Variable Text','Segoe UI'; font-size:13px; margin:2px 4px; }
		h3    { color:#6d28d9; font-size:10px; font-weight:700; margin:18px 0 6px 0; letter-spacing:1.4px; }
		p     { color:#46556f; margin:0 0 8px 0; line-height:1.65; }
		ul    { color:#46556f; margin:0 0 8px 16px; padding:0; }
		li    { margin:3px 0; line-height:1.65; }
		b     { color:#111827; font-weight:600; }
		code  { background-color:#f1f5f9; color:#7c3aed; padding:2px 8px;
		        border-radius:5px; font-size:12px; font-family:'Cascadia Code','Consolas',monospace; }
		a     { color:#7c3aed; text-decoration:none; }
	)";
	}

	return R"(
		body  { color:#b0b0cc; font-family:'Segoe UI Variable Text','Segoe UI'; font-size:13px; margin:2px 4px; }
		h3    { color:#8b5cf6; font-size:10px; font-weight:700; margin:18px 0 6px 0; letter-spacing:1.4px; }
		p     { color:#8080a8; margin:0 0 8px 0; line-height:1.65; }
		ul    { color:#8080a8; margin:0 0 8px 16px; padding:0; }
		li    { margin:3px 0; line-height:1.65; }
		b     { color:#c8d4f0; font-weight:600; }
		code  { background-color:#1a1a2a; color:#a78bfa; padding:2px 8px;
		        border-radius:5px; font-size:12px; font-family:'Cascadia Code','Consolas',monospace; }
		a     { color:#7c3aed; text-decoration:none; }
	)";
}
