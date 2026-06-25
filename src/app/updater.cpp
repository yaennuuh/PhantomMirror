#include "updater.h"

#include "app_strings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QtGlobal>

namespace {

QString currentVersion()
{
	return QStringLiteral(PHANTOM_MIRROR_VERSION);
}

QString updateRepo()
{
	return QStringLiteral(PHANTOM_MIRROR_UPDATE_REPO);
}

QString updateAsset()
{
	return QStringLiteral(PHANTOM_MIRROR_UPDATE_ASSET);
}

QNetworkRequest githubRequest(const QUrl &url)
{
	QNetworkRequest request(url);
	request.setRawHeader("Accept", "application/vnd.github+json");
	request.setRawHeader("User-Agent", "Phantom-Mirror-Updater");
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	return request;
}

QString powershellLiteral(const QString &value)
{
	return "'" + QString(value).replace("'", "''") + "'";
}

} // namespace

Updater::Updater(QObject *parent)
	: QObject(parent)
{
}

void Updater::setLanguage(AppLanguage language)
{
	if (language_ == language)
		return;
	language_ = language;
	if (statusKind_ != StatusKind::Idle)
		emit updateStatusChanged(currentStatusText());
}

void Updater::checkForUpdates(bool interactive)
{
	if (busy_)
		return;

	if (updateRepo().trimmed().isEmpty()) {
		setStatus(StatusKind::Failed, "update_repo_missing");
		emit updateFailed(currentStatusText());
		return;
	}

	busy_ = true;
	setStatus(StatusKind::Checking);

	const QUrl url(QString("https://api.github.com/repos/%1/releases/latest").arg(updateRepo()));
	QNetworkReply *reply = network_.get(githubRequest(url));
	connect(reply, &QNetworkReply::finished, this, [this, reply, interactive]() {
		handleReleaseReply(reply, interactive);
	});
}

void Updater::downloadAndInstall(const QString &version, const QUrl &assetUrl)
{
	if (busy_)
		return;

	busy_ = true;
	setStatus(StatusKind::Downloading, version);

	QNetworkReply *reply = network_.get(githubRequest(assetUrl));
	connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
		if (total > 0)
			setStatus(StatusKind::DownloadProgress, QString(), QString(), QString(), (received * 100) / total);
	});
	connect(reply, &QNetworkReply::finished, this, [this, reply, version]() {
		handleDownloadReply(reply, version);
	});
}

void Updater::handleReleaseReply(QNetworkReply *reply, bool interactive)
{
	const auto cleanup = qScopeGuard([reply]() {
		reply->deleteLater();
	});

	if (reply->error() != QNetworkReply::NoError) {
		busy_ = false;
		setStatus(StatusKind::Failed, "update_check", reply->errorString());
		emit updateFailed(currentStatusText());
		return;
	}

	const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
	const QJsonObject release = document.object();
	const QString version = release.value("tag_name").toString();
	if (version.isEmpty()) {
		busy_ = false;
		setStatus(StatusKind::Failed, "release_no_version");
		emit updateFailed(currentStatusText());
		return;
	}

	QUrl assetUrl;
	const QJsonArray assets = release.value("assets").toArray();
	for (const QJsonValue &value : assets) {
		const QJsonObject asset = value.toObject();
		if (asset.value("name").toString() == updateAsset()) {
			assetUrl = QUrl(asset.value("browser_download_url").toString());
			break;
		}
	}

	if (!isNewerVersion(version, currentVersion())) {
		busy_ = false;
		setStatus(StatusKind::NoUpdates);
		if (interactive)
			emit noUpdateAvailable();
		return;
	}

	if (!assetUrl.isValid()) {
		busy_ = false;
		setStatus(StatusKind::Failed, "missing_asset", version, updateAsset());
		emit updateFailed(currentStatusText());
		return;
	}

	busy_ = false;
	setStatus(StatusKind::UpdateFound, version);
	emit updateAvailable(version, assetUrl);
}

void Updater::handleDownloadReply(QNetworkReply *reply, const QString &version)
{
	const auto cleanup = qScopeGuard([reply, this]() {
		reply->deleteLater();
		busy_ = false;
	});

	if (reply->error() != QNetworkReply::NoError) {
		setStatus(StatusKind::Failed, "download_failed", reply->errorString());
		emit updateFailed(currentStatusText());
		return;
	}

	const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	const QString zipPath = QDir(tempDir).filePath(QString("Phantom-Mirror-%1.zip").arg(normalizeVersion(version)));
	QFile zipFile(zipPath);
	if (!zipFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		setStatus(StatusKind::Failed, "write_zip", zipFile.errorString());
		emit updateFailed(currentStatusText());
		return;
	}
	zipFile.write(reply->readAll());
	zipFile.close();

	const QString scriptPath = updaterScriptPath();
	QFile script(scriptPath);
	if (!script.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		setStatus(StatusKind::Failed, "write_script", script.errorString());
		emit updateFailed(currentStatusText());
		return;
	}

	const QString installDir = QCoreApplication::applicationDirPath();
	const QString exePath = QCoreApplication::applicationFilePath();
	const qint64 processId = QCoreApplication::applicationPid();
	const QString body = QString(R"ps1(
$ErrorActionPreference = "Stop"
$processId = %1
$zipPath = %2
$installDir = %3
$exePath = %4

Wait-Process -Id $processId -ErrorAction SilentlyContinue
$staging = Join-Path $env:TEMP ("Phantom-Mirror-update-" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force $staging | Out-Null
Expand-Archive -Path $zipPath -DestinationPath $staging -Force
Copy-Item -Path (Join-Path $staging "*") -Destination $installDir -Recurse -Force
Start-Process -FilePath $exePath
Remove-Item -Recurse -Force $staging -ErrorAction SilentlyContinue
Remove-Item -Force $zipPath -ErrorAction SilentlyContinue
Remove-Item -Force $PSCommandPath -ErrorAction SilentlyContinue
)ps1")
				     .arg(processId)
				     .arg(powershellLiteral(zipPath))
				     .arg(powershellLiteral(installDir))
				     .arg(powershellLiteral(exePath));
	script.write(body.toUtf8());
	script.close();

	setStatus(StatusKind::Installing);
	QProcess::startDetached("powershell.exe", {"-NoProfile", "-ExecutionPolicy", "Bypass", "-File", scriptPath});
	QCoreApplication::quit();
}

void Updater::setStatus(StatusKind kind, const QString &first, const QString &second, const QString &third, qint64 value)
{
	statusKind_ = kind;
	statusFirst_ = first;
	statusSecond_ = second;
	statusThird_ = third;
	statusValue_ = value;
	emit updateStatusChanged(currentStatusText());
}

QString Updater::currentStatusText() const
{
	switch (statusKind_) {
	case StatusKind::Idle:
		return QString();
	case StatusKind::Checking:
		return language_ == AppLanguage::German ? "Prüfe auf Updates..." : "Checking for updates...";
	case StatusKind::Downloading:
		return language_ == AppLanguage::German
			? QString("Lade Update %1 herunter...").arg(statusFirst_)
			: QString("Downloading update %1...").arg(statusFirst_);
	case StatusKind::DownloadProgress:
		return language_ == AppLanguage::German
			? QString("Download läuft... %1%").arg(statusValue_)
			: QString("Downloading... %1%").arg(statusValue_);
	case StatusKind::Failed:
		if (statusFirst_ == "update_repo_missing")
			return language_ == AppLanguage::German
				? "Updates sind fuer diesen Build nicht konfiguriert."
				: "Update checks are not configured for this build.";
		if (statusFirst_ == "update_check")
			return language_ == AppLanguage::German
				? QString("Update-Prüfung fehlgeschlagen: %1").arg(statusSecond_)
				: QString("Could not check for updates: %1").arg(statusSecond_);
		if (statusFirst_ == "release_no_version")
			return language_ == AppLanguage::German
				? "GitHub Release enthält keine Version."
				: "The GitHub release does not contain a version.";
		if (statusFirst_ == "missing_asset")
			return language_ == AppLanguage::German
				? QString("Release %1 enthält kein Asset %2.").arg(statusSecond_, statusThird_)
				: QString("Release %1 does not contain the asset %2.").arg(statusSecond_, statusThird_);
		if (statusFirst_ == "download_failed")
			return language_ == AppLanguage::German
				? QString("Update-Download fehlgeschlagen: %1").arg(statusSecond_)
				: QString("Could not download the update: %1").arg(statusSecond_);
		if (statusFirst_ == "write_zip")
			return language_ == AppLanguage::German
				? QString("Update-Datei kann nicht geschrieben werden: %1").arg(statusSecond_)
				: QString("Could not write the update file: %1").arg(statusSecond_);
		if (statusFirst_ == "write_script")
			return language_ == AppLanguage::German
				? QString("Updater-Skript kann nicht geschrieben werden: %1").arg(statusSecond_)
				: QString("Could not write the updater script: %1").arg(statusSecond_);
		return statusSecond_;
	case StatusKind::NoUpdates:
		return language_ == AppLanguage::German ? "Keine Updates verfügbar." : "No updates available.";
	case StatusKind::UpdateFound:
		return language_ == AppLanguage::German
			? QString("Update gefunden: %1").arg(statusFirst_)
			: QString("Update available: %1").arg(statusFirst_);
	case StatusKind::Installing:
		return language_ == AppLanguage::German
			? "Update wird installiert. Phantom Mirror startet danach neu..."
			: "Installing update. Phantom Mirror will restart afterwards...";
	}
	return QString();
}

bool Updater::isNewerVersion(const QString &candidate, const QString &current) const
{
	const QStringList left = normalizeVersion(candidate).split('.');
	const QStringList right = normalizeVersion(current).split('.');
	const int count = qMax(left.size(), right.size());
	for (int index = 0; index < count; ++index) {
		const int a = index < left.size() ? left[index].toInt() : 0;
		const int b = index < right.size() ? right[index].toInt() : 0;
		if (a != b)
			return a > b;
	}
	return false;
}

QString Updater::normalizeVersion(const QString &version) const
{
	QString value = version.trimmed();
	if (value.startsWith('v', Qt::CaseInsensitive))
		value.remove(0, 1);
	return value;
}

QString Updater::updaterScriptPath() const
{
	const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	return QDir(tempDir).filePath("Phantom-Mirror-apply-update.ps1");
}
