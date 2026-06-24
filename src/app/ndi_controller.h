#pragma once

#include "app_config.h"

#include <QObject>
#include <QRect>
#include <QStringList>
#include <QTimer>

#include <windows.h>

struct NdiStatus {
	bool running = false;
	bool connected = false;
	bool sourceFound = false;
	int width = 0;
	int height = 0;
	double fps = 0.0;
	long frame = 0;
	QString sourceName;
	QString lastError;
};

struct NdiRuntimeAvailability {
	bool available = false;
	QString error;
};

class NdiController : public QObject {
	Q_OBJECT

public:
	explicit NdiController(QObject *parent = nullptr);
	~NdiController() override;

	bool start(HWND owner, const AppConfig &config, const QRect &monitorRect);
	bool update(const AppConfig &config, const QRect &monitorRect);
	void stop();
	NdiStatus status() const;
	NdiRuntimeAvailability runtimeAvailability() const;
	QStringList availableSources() const;
	bool isEnabled() const { return enabled_; }

signals:
	void statusChanged(const NdiStatus &status);

private:
	void pollStatus();
	bool apply(HWND owner, const AppConfig &config, const QRect &monitorRect, bool allowStart);
	NdiStatus unavailableStatus(const QString &error) const;

	QTimer pollTimer_;
	bool enabled_ = false;
	QString lastError_;
};
