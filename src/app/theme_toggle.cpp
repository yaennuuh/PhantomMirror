#include "theme_toggle.h"

#include "theme.h"

#include <QEnterEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

#include <cmath>

namespace {

constexpr int kTrackWidth = 72;
constexpr int kTrackHeight = 36;
constexpr int kKnobSize = 30;
constexpr int kMargin = 3;

QColor blend(const QColor &from, const QColor &to, qreal t)
{
	auto lerp = [t](int a, int b) {
		return qRound(a + (b - a) * t);
	};
	return QColor(lerp(from.red(), to.red()),
	              lerp(from.green(), to.green()),
	              lerp(from.blue(), to.blue()),
	              lerp(from.alpha(), to.alpha()));
}

} // namespace

ThemeToggle::ThemeToggle(QWidget *parent)
	: QAbstractButton(parent)
	, anim_(new QPropertyAnimation(this, "knobPos", this))
{
	setCheckable(true);
	setCursor(Qt::PointingHandCursor);
	setFocusPolicy(Qt::StrongFocus);
	anim_->setDuration(180);
	anim_->setEasingCurve(QEasingCurve::InOutCubic);
	connect(this, &QAbstractButton::toggled, this, &ThemeToggle::syncAnimation);
}

QSize ThemeToggle::sizeHint() const
{
	return QSize(kTrackWidth, kTrackHeight);
}

void ThemeToggle::setKnobPos(qreal pos)
{
	knobPos_ = pos;
	update();
}

void ThemeToggle::syncAnimation(bool checked)
{
	const qreal end = checked ? 1.0 : 0.0;
	if (qFuzzyCompare(knobPos_ + 1.0, end + 1.0))
		return;
	anim_->stop();
	anim_->setStartValue(knobPos_);
	anim_->setEndValue(end);
	anim_->start();
}

void ThemeToggle::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	const QRectF trackRect(0, 0, kTrackWidth, kTrackHeight);
	const QColor darkTrack("#23233a");
	const QColor lightTrack("#d7e3f4");
	const QColor track = hovered_
		? blend(isChecked() ? lightTrack : darkTrack, QColor("#7c3aed"), isChecked() ? 0.12 : 0.18)
		: (isChecked() ? lightTrack : darkTrack);
	const QColor border = isChecked() ? QColor("#9fb8dd") : QColor("#47476a");

	p.setPen(QPen(border, 1.2));
	p.setBrush(track);
	p.drawRoundedRect(trackRect.adjusted(0.6, 0.6, -0.6, -0.6), kTrackHeight / 2.0, kTrackHeight / 2.0);

	const qreal knobX = kMargin + knobPos_ * (kTrackWidth - kKnobSize - 2.0 * kMargin);
	const QRectF knobRect(knobX, kMargin, kKnobSize, kKnobSize);
	p.setPen(Qt::NoPen);
	p.setBrush(currentAppTheme() == AppTheme::Light ? QColor(15, 23, 42, 22) : QColor(0, 0, 0, 42));
	p.drawEllipse(knobRect.translated(0.0, 1.4));
	p.setBrush(isChecked() ? QColor("#ffffff") : QColor("#f3f4f6"));
	p.drawEllipse(knobRect);

	const QRectF leftIconRect(11, 10, 16, 16);
	const QRectF rightIconRect(kTrackWidth - 27, 10, 16, 16);
	if (isChecked()) {
		drawMoon(p, leftIconRect, QColor("#1e293b"));
		drawSun(p, knobRect.adjusted(6.5, 6.5, -6.5, -6.5), QColor("#ea580c"));
	} else {
		drawMoon(p, knobRect.adjusted(6.5, 6.5, -6.5, -6.5), QColor("#334155"));
		drawSun(p, rightIconRect, QColor("#facc15"));
	}
}

void ThemeToggle::enterEvent(QEnterEvent *event)
{
	hovered_ = true;
	update();
	QAbstractButton::enterEvent(event);
}

void ThemeToggle::leaveEvent(QEvent *event)
{
	hovered_ = false;
	update();
	QAbstractButton::leaveEvent(event);
}

void ThemeToggle::drawSun(QPainter &p, const QRectF &rect, const QColor &color) const
{
	p.save();
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap));
	p.setBrush(Qt::NoBrush);
	const QPointF center = rect.center();
	const qreal inner = rect.width() * 0.26;
	p.drawEllipse(center, inner, inner);
	for (int i = 0; i < 8; ++i) {
		const qreal angle = qDegreesToRadians(45.0 * i);
		const QPointF from(center.x() + std::cos(angle) * (inner + 1.6),
		                   center.y() + std::sin(angle) * (inner + 1.6));
		const QPointF to(center.x() + std::cos(angle) * (rect.width() * 0.52),
		                 center.y() + std::sin(angle) * (rect.height() * 0.52));
		p.drawLine(from, to);
	}
	p.restore();
}

void ThemeToggle::drawMoon(QPainter &p, const QRectF &rect, const QColor &color) const
{
	p.save();
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::NoPen);
	p.setBrush(color);
	QPainterPath outer;
	outer.addEllipse(rect);
	QPainterPath cutout;
	cutout.addEllipse(rect.adjusted(rect.width() * 0.28, rect.height() * 0.02, rect.width() * 0.02, rect.height() * 0.02));
	p.drawPath(outer.subtracted(cutout));
	p.restore();
}
