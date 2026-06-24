#pragma once

#include "app_strings.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>

class QNetworkReply;

class Updater : public QObject {
	Q_OBJECT

public:
	explicit Updater(QObject *parent = nullptr);

	void checkForUpdates(bool interactive);
	void downloadAndInstall(const QString &version, const QUrl &assetUrl);
	void setLanguage(AppLanguage language);
	bool isBusy() const { return busy_; }

signals:
	void updateAvailable(const QString &version, const QUrl &assetUrl);
	void noUpdateAvailable();
	void updateStatusChanged(const QString &message);
	void updateFailed(const QString &message);

private:
	enum class StatusKind {
		Idle,
		Checking,
		Downloading,
		DownloadProgress,
		Failed,
		NoUpdates,
		UpdateFound,
		Installing,
	};

	void setStatus(StatusKind kind, const QString &first = QString(), const QString &second = QString(),
				   const QString &third = QString(), qint64 value = 0);
	QString currentStatusText() const;
	void handleReleaseReply(QNetworkReply *reply, bool interactive);
	void handleDownloadReply(QNetworkReply *reply, const QString &version);
	bool isNewerVersion(const QString &candidate, const QString &current) const;
	QString normalizeVersion(const QString &version) const;
	QString updaterScriptPath() const;

	QNetworkAccessManager network_;
	AppLanguage language_ = AppLanguage::English;
	StatusKind statusKind_ = StatusKind::Idle;
	QString statusFirst_;
	QString statusSecond_;
	QString statusThird_;
	qint64 statusValue_ = 0;
	bool busy_ = false;
};
