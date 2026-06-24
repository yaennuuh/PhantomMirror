#include "app_icon.h"

#include <QCoreApplication>
#include <QDir>

namespace {

QIcon loadIconCandidate(const QString &path)
{
	QIcon icon(path);
	return icon.isNull() ? QIcon() : icon;
}

QIcon resolveAppIcon()
{
	if (const QIcon icon = loadIconCandidate(":/icons/icon.ico"); !icon.isNull())
		return icon;

	if (const QIcon icon = loadIconCandidate(QCoreApplication::applicationFilePath()); !icon.isNull())
		return icon;

	const QString appDir = QCoreApplication::applicationDirPath();
	if (const QIcon icon = loadIconCandidate(QDir(appDir).filePath("Phantom Mirror.exe")); !icon.isNull())
		return icon;
	if (const QIcon icon = loadIconCandidate(QDir(appDir).filePath("icon.ico")); !icon.isNull())
		return icon;
	if (const QIcon icon = loadIconCandidate("resources/icon.ico"); !icon.isNull())
		return icon;

	return {};
}

} // namespace

QIcon appIcon()
{
	static const QIcon icon = resolveAppIcon();
	return icon;
}

QPixmap appLogoPixmap(int size)
{
	const QIcon icon = appIcon();
	if (icon.isNull())
		return {};

	QPixmap pixmap = icon.pixmap(size, size);
	if (!pixmap.isNull())
		return pixmap;

	for (const int candidateSize : {256, 128, 64, 48, 32, 24, 16}) {
		pixmap = icon.pixmap(candidateSize, candidateSize);
		if (!pixmap.isNull())
			return pixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	return {};
}
