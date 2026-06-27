#pragma once

#include <QAbstractButton>
#include <QPropertyAnimation>

class ThemeToggle : public QAbstractButton {
	Q_OBJECT
	Q_PROPERTY(qreal knobPos READ knobPos WRITE setKnobPos)

public:
	explicit ThemeToggle(QWidget *parent = nullptr);

	QSize sizeHint() const override;
	qreal knobPos() const { return knobPos_; }
	void setKnobPos(qreal pos);

protected:
	void paintEvent(QPaintEvent *event) override;
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;

private:
	void syncAnimation(bool checked);
	void drawSun(QPainter &p, const QRectF &rect, const QColor &color) const;
	void drawMoon(QPainter &p, const QRectF &rect, const QColor &color) const;

	qreal knobPos_ = 0.0;
	bool hovered_ = false;
	QPropertyAnimation *anim_ = nullptr;
};
