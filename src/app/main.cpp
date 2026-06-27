#include "app_icon.h"
#include "main_window.h"
#include "theme.h"

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStyleFactory>

#include <windows.h>

namespace {

constexpr auto kSingleInstanceMutexName = L"Local\\PhantomMirror.SingleInstance";
constexpr auto kActivationServerName = "PhantomMirror.SingleInstance";

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
	app.setWindowIcon(appIcon());
	app.setQuitOnLastWindowClosed(false);
	app.setStyle(QStyleFactory::create("Fusion"));
	applyAppTheme(app, resolveAppTheme(loadOrInitConfig().ui.theme));

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
