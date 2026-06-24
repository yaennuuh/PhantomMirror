#pragma once

#include "app_config.h"

#include <QObject>
#include <QRect>
#include <QStringList>
#include <QTimer>

#include <windows.h>

struct SpoutStatus {
	bool running = false;
	bool connected = false;
	bool senderFound = false;
	int width = 0;
	int height = 0;
	double fps = 0.0;
	long frame = 0;
	QString senderName;
	QString lastError;
};

class SpoutController : public QObject {
	Q_OBJECT

public:
	explicit SpoutController(QObject *parent = nullptr);
	~SpoutController() override;

	bool start(HWND owner, const AppConfig &config, const QRect &monitorRect);
	bool update(const AppConfig &config, const QRect &monitorRect);
	void stop();
	SpoutStatus status() const;
	QStringList availableSenders() const;
	bool isEnabled() const { return enabled_; }

signals:
	void statusChanged(const SpoutStatus &status);

private:
	void pollStatus();
	bool apply(HWND owner, const AppConfig &config, const QRect &monitorRect, bool allowStart);

	QTimer pollTimer_;
	bool enabled_ = false;
};
