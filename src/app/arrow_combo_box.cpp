#include "arrow_combo_box.h"
#include "theme.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionComboBox>
#include <QStylePainter>

ArrowComboBox::ArrowComboBox(QWidget *parent)
	: QComboBox(parent)
{
}

void ArrowComboBox::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QStylePainter painter(this);
	QStyleOptionComboBox option;
	initStyleOption(&option);
	painter.drawComplexControl(QStyle::CC_ComboBox, option);
	painter.drawControl(QStyle::CE_ComboBoxLabel, option);

	painter.setRenderHint(QPainter::Antialiasing, true);
	const QColor arrowColor = currentAppTheme() == AppTheme::Light ? QColor("#475569") : QColor("#dde3f0");
	painter.setPen(QPen(arrowColor, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

	const int iconSize = 8;
	const int centerX = width() - 18;
	const int centerY = height() / 2 + 1;
	QPainterPath path;
	path.moveTo(centerX - iconSize / 2, centerY - 2);
	path.lineTo(centerX, centerY + 2);
	path.lineTo(centerX + iconSize / 2, centerY - 2);
	painter.drawPath(path);
}
