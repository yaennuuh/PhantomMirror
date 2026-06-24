#pragma once

#include <QAbstractButton>
#include <QPropertyAnimation>

class ToggleSwitch : public QAbstractButton {
	Q_OBJECT
	Q_PROPERTY(qreal thumbPos READ thumbPos WRITE setThumbPos)

public:
	explicit ToggleSwitch(const QString &label = {}, QWidget *parent = nullptr);

	QSize sizeHint() const override;
	qreal thumbPos() const { return thumbPos_; }
	void setThumbPos(qreal pos);

protected:
	void paintEvent(QPaintEvent *event) override;
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;

private:
	void onToggled(bool on);

	qreal thumbPos_ = 0.0;
	bool hovered_ = false;
	QPropertyAnimation *anim_;

	static constexpr int kTrackW = 42;
	static constexpr int kTrackH = 24;
	static constexpr int kThumb = 18;
	static constexpr int kMargin = 3;
	static constexpr int kGap = 12;
};
