#include "main_window.h"

#include <QApplication>
#include <QIcon>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStyleFactory>

#include <windows.h>

namespace {

constexpr auto kSingleInstanceMutexName = L"Local\\PhantomMirror.SingleInstance";
constexpr auto kActivationServerName = "PhantomMirror.SingleInstance";

QIcon makeAppIcon()
{
	const QIcon icon(":/icons/icon.ico");
	return icon.isNull() ? QIcon("resources/icon.ico") : icon;
}

class SingleInstanceGuard {
public:
	SingleInstanceGuard()
		: mutex_(CreateMutexW(nullptr, TRUE, kSingleInstanceMutexName))
		, alreadyRunning_(mutex_ && GetLastError() == ERROR_ALREADY_EXISTS)
	{
	}

	~SingleInstanceGuard()
	{
		if (mutex_)
			CloseHandle(mutex_);
	}

	bool alreadyRunning() const
	{
		return alreadyRunning_;
	}

private:
	HANDLE mutex_ = nullptr;
	bool alreadyRunning_ = false;
};

void notifyRunningInstance()
{
	QLocalSocket socket;
	socket.connectToServer(kActivationServerName);
	if (!socket.waitForConnected(300))
		return;

	socket.write("show-settings");
	socket.flush();
	socket.waitForBytesWritten(300);
	socket.disconnectFromServer();
}

const char *kDarkTheme = R"(
QDialog {
    background-color: #111118;
}
QWidget {
    font-family: 'Segoe UI Variable Text', 'Segoe UI', Arial, sans-serif;
    font-size: 13px;
    color: #e2e8f0;
}
/* ── Cards ─────────────────────────────────────────────────────────── */
QFrame#card {
    background-color: #191924;
    border: 1px solid #26263a;
    border-radius: 12px;
}
/* ── Labels ─────────────────────────────────────────────────────────── */
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
/* ── Dividers ────────────────────────────────────────────────────────── */
QFrame#cardSep  { background-color: #23233a; border: none; max-height: 1px; }
QFrame#dialogSep{ background-color: #1a1a28; border: none; max-height: 1px; }
/* ── Line Edit ───────────────────────────────────────────────────────── */
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
/* ── SpinBox ─────────────────────────────────────────────────────────── */
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
/* ── ComboBox ────────────────────────────────────────────────────────── */
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
/* ── Checkbox (fallback, used in onboarding) ─────────────────────────── */
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
/* ── Buttons ─────────────────────────────────────────────────────────── */
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
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #9061ff, stop:1 #7c3aed);
    border: 1px solid #6d28d9;
    color: #fff;
    font-weight: 600;
}
QPushButton[role="primary"]:hover {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #a070ff, stop:1 #8b5cf6);
    border-color: #8b5cf6;
}
QPushButton[role="primary"]:pressed {
    background-color: #6d28d9;
    border-color: #5b21b6;
}
/* ── Anchor grid ─────────────────────────────────────────────────────── */
QPushButton#anchorBtn {
    background-color: #141420;
    border: 1.5px solid #222234;
    border-radius: 6px;
    min-width:  40px; max-width:  40px;
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
/* ── TextBrowser ─────────────────────────────────────────────────────── */
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
/* ── Scrollbars ──────────────────────────────────────────────────────── */
QScrollBar:vertical { background: transparent; width: 5px; margin: 0; }
QScrollBar::handle:vertical { background-color: #2a2a42; border-radius: 3px; min-height: 20px; }
QScrollBar::handle:vertical:hover { background-color: #3a3a58; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,
QScrollBar::add-page:vertical,  QScrollBar::sub-page:vertical { background:none; height:0; width:0; }
/* ── Context Menu ────────────────────────────────────────────────────── */
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
/* ── MessageBox ──────────────────────────────────────────────────────── */
QMessageBox { background-color: #111118; }
QMessageBox QLabel { color: #e2e8f0; }
QMessageBox QPushButton { min-width: 80px; }
)";

} // namespace

int main(int argc, char *argv[])
{
	SingleInstanceGuard singleInstance;
	if (singleInstance.alreadyRunning()) {
		notifyRunningInstance();
		return 1;
	}

	QApplication app(argc, argv);
	QApplication::setApplicationName("Phantom Mirror");
	QApplication::setOrganizationName("Phantom Mirror");
	app.setWindowIcon(makeAppIcon());
	app.setQuitOnLastWindowClosed(false);
	app.setStyle(QStyleFactory::create("Fusion"));
	app.setStyleSheet(kDarkTheme);

	MainWindow window;

	QLocalServer::removeServer(kActivationServerName);
	QLocalServer activationServer;
	if (activationServer.listen(kActivationServerName)) {
		QObject::connect(&activationServer, &QLocalServer::newConnection, &window, [&activationServer, &window]() {
			while (QLocalSocket *socket = activationServer.nextPendingConnection()) {
				QObject::connect(socket, &QLocalSocket::readyRead, socket, [&window, socket]() {
					if (socket->readAll().contains("show-settings"))
						window.showSettingsFromExternalActivation();
				});
				QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
				QObject::connect(socket, &QLocalSocket::errorOccurred, socket, &QObject::deleteLater);
			}
		});
	}

	return app.exec();
}
