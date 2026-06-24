#pragma once

#include "app_config.h"

#include <QEvent>
#include <QLineEdit>

class HotkeyEdit : public QLineEdit {
	Q_OBJECT

public:
	explicit HotkeyEdit(QWidget *parent = nullptr);

	void setHotkeyConfig(const OverlayHotkeyConfig &config);
	OverlayHotkeyConfig hotkeyConfig() const;
	void clearHotkey();

protected:
	bool event(QEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void focusInEvent(QFocusEvent *event) override;

private:
	void updateDisplay();
	static bool isModifierOnlyKey(int key);

	OverlayHotkeyConfig config_;
};
