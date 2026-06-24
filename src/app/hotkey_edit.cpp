#include "hotkey_edit.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>

namespace {

QString hotkeyDisplayText(const OverlayHotkeyConfig &config)
{
	if (!hotkeyHasBinding(config))
		return {};

	int sequence = 0;
	for (const QString &modifier : normalizeHotkeyModifiers(config.modifiers)) {
		if (modifier == "ctrl") sequence |= Qt::CTRL;
		else if (modifier == "alt") sequence |= Qt::ALT;
		else if (modifier == "shift") sequence |= Qt::SHIFT;
		else if (modifier == "win") sequence |= Qt::META;
	}

	const QKeySequence keySequence = QKeySequence::fromString(config.key, QKeySequence::PortableText);
	if (!keySequence.isEmpty())
		sequence |= keySequence[0];

	return QKeySequence(sequence).toString(QKeySequence::NativeText);
}

} // namespace

HotkeyEdit::HotkeyEdit(QWidget *parent)
	: QLineEdit(parent)
{
	setReadOnly(true);
	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_InputMethodEnabled, false);
	setContextMenuPolicy(Qt::NoContextMenu);
	setCursor(Qt::PointingHandCursor);
	updateDisplay();
}

bool HotkeyEdit::event(QEvent *event)
{
	if (event && event->type() == QEvent::ShortcutOverride) {
		event->accept();
		return true;
	}
	return QLineEdit::event(event);
}

void HotkeyEdit::setHotkeyConfig(const OverlayHotkeyConfig &config)
{
	config_ = config;
	config_.modifiers = normalizeHotkeyModifiers(config_.modifiers);
	config_.enabled = hotkeyHasBinding(config_);
	updateDisplay();
}

OverlayHotkeyConfig HotkeyEdit::hotkeyConfig() const
{
	return config_;
}

void HotkeyEdit::clearHotkey()
{
	config_ = OverlayHotkeyConfig{};
	updateDisplay();
}

void HotkeyEdit::keyPressEvent(QKeyEvent *event)
{
	if (!event) {
		QLineEdit::keyPressEvent(event);
		return;
	}

	if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete || event->key() == Qt::Key_Escape) {
		clearHotkey();
		event->accept();
		return;
	}

	if (isModifierOnlyKey(event->key())) {
		event->accept();
		return;
	}

	OverlayHotkeyConfig next;
	if (event->modifiers() & Qt::ControlModifier) next.modifiers.append("ctrl");
	if (event->modifiers() & Qt::AltModifier) next.modifiers.append("alt");
	if (event->modifiers() & Qt::ShiftModifier) next.modifiers.append("shift");
	if (event->modifiers() & Qt::MetaModifier) next.modifiers.append("win");
	next.modifiers = normalizeHotkeyModifiers(next.modifiers);
	next.key = QKeySequence(event->key()).toString(QKeySequence::PortableText);
	next.enabled = hotkeyHasBinding(next);
	config_ = next;
	updateDisplay();
	event->accept();
}

void HotkeyEdit::mousePressEvent(QMouseEvent *event)
{
	setFocus(Qt::MouseFocusReason);
	selectAll();
	if (event)
		event->accept();
}

void HotkeyEdit::focusInEvent(QFocusEvent *event)
{
	QLineEdit::focusInEvent(event);
	selectAll();
}

void HotkeyEdit::updateDisplay()
{
	setText(hotkeyDisplayText(config_));
}

bool HotkeyEdit::isModifierOnlyKey(int key)
{
	return key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt ||
	       key == Qt::Key_Meta || key == Qt::Key_AltGr;
}
