#include "toggle_switch.h"

#include <QEnterEvent>
#include <QPainter>

ToggleSwitch::ToggleSwitch(const QString &label, QWidget *parent)
	: QAbstractButton(parent)
	, anim_(new QPropertyAnimation(this, "thumbPos", this))
{
	setText(label);
	setCheckable(true);
	setCursor(Qt::PointingHandCursor);
	anim_->setDuration(160);
	anim_->setEasingCurve(QEasingCurve::InOutQuad);
	connect(this, &QAbstractButton::toggled, this, &ToggleSwitch::onToggled);
}

QSize ToggleSwitch::sizeHint() const
{
	const int tw = text().isEmpty() ? 0 : kGap + fontMetrics().horizontalAdvance(text());
	return QSize(kTrackW + tw, qMax(kTrackH + 6, fontMetrics().height() + 4));
}

void ToggleSwitch::setThumbPos(qreal pos)
{
	thumbPos_ = pos;
	update();
}

void ToggleSwitch::onToggled(bool on)
{
	const qreal end = on ? 1.0 : 0.0;
	if (qFuzzyCompare(thumbPos_ + 1.0, end + 1.0))
		return;
	anim_->stop();
	anim_->setStartValue(thumbPos_);
	anim_->setEndValue(end);
	anim_->start();
}

void ToggleSwitch::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	// Interpolate track color: off=#222236, on=#7c3aed
	const QColor off(0x22, 0x22, 0x36);
	const QColor on(0x7c, 0x3a, 0xed);
	auto lerp = [](int a, int b, qreal t) { return qRound(a + (b - a) * t); };
	QColor track(lerp(off.red(), on.red(), thumbPos_),
				 lerp(off.green(), on.green(), thumbPos_),
				 lerp(off.blue(), on.blue(), thumbPos_));

	if (!isEnabled())
		track = track.darker(160);
	else if (hovered_)
		track = track.lighter(118);

	// Track
	const int ty = (height() - kTrackH) / 2;
	p.setPen(Qt::NoPen);
	p.setBrush(track);
	p.drawRoundedRect(QRectF(0, ty, kTrackW, kTrackH), kTrackH / 2.0, kTrackH / 2.0);

	// Thumb — white circle with subtle shadow
	const qreal tx = kMargin + thumbPos_ * (kTrackW - kThumb - 2.0 * kMargin);
	const qreal ty2 = (height() - kThumb) / 2.0;
	p.setPen(Qt::NoPen);
	// Shadow
	if (isEnabled()) {
		p.setBrush(QColor(0, 0, 0, 40));
		p.drawEllipse(QRectF(tx + 0.5, ty2 + 1.5, kThumb, kThumb));
	}
	// Thumb fill
	p.setBrush(isEnabled() ? Qt::white : QColor(0x44, 0x44, 0x60));
	p.drawEllipse(QRectF(tx, ty2, kThumb, kThumb));

	// Label
	if (!text().isEmpty()) {
		p.setPen(isEnabled() ? QColor(0xd4, 0xd4, 0xe8) : QColor(0x44, 0x44, 0x60));
		p.setFont(font());
		p.drawText(QRect(kTrackW + kGap, 0, width() - kTrackW - kGap, height()),
				   Qt::AlignLeft | Qt::AlignVCenter, text());
	}
}

void ToggleSwitch::enterEvent(QEnterEvent *event)
{
	hovered_ = true;
	update();
	QAbstractButton::enterEvent(event);
}

void ToggleSwitch::leaveEvent(QEvent *event)
{
	hovered_ = false;
	update();
	QAbstractButton::leaveEvent(event);
}
