#include "arrow_combo_box.h"
#include "hotkey_edit.h"
#include "settings_dialog.h"

#include "app_strings.h"

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDesktopServices>
#include <QFrame>
#include <QGuiApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QPushButton>
#include <QScreen>
#include <QSpinBox>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QFrame *makeSep(QWidget *parent, const char *id = "cardSep")
{
	auto *sep = new QFrame(parent);
	sep->setFrameShape(QFrame::HLine);
	sep->setObjectName(id);
	sep->setFixedHeight(1);
	return sep;
}

QFrame *makeCard(QWidget *parent)
{
	auto *f = new QFrame(parent);
	f->setObjectName("card");
	return f;
}

QLabel *makeSectionTitle(const QString &text, QWidget *parent)
{
	auto *l = new QLabel(text.toUpper(), parent);
	l->setObjectName("sectionTitle");
	return l;
}

QLabel *makeFieldLabel(const QString &text, QWidget *parent)
{
	auto *l = new QLabel(text, parent);
	l->setObjectName("mutedLabel");
	l->setMinimumWidth(108);
	l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	return l;
}

QString monitorId(QScreen *screen, int index)
{
	if (!screen)
		return QString("screen:%1").arg(index);
	const QString name = screen->name().trimmed();
	if (!name.isEmpty())
		return name;
	const QRect geometry = screen->geometry();
	return QString("screen:%1:%2:%3:%4:%5")
		.arg(index)
		.arg(geometry.x())
		.arg(geometry.y())
		.arg(geometry.width())
		.arg(geometry.height());
}

QUrl ndiToolsUrl()
{
	return QUrl("https://ndi.video/tools/");
}

} // namespace

SettingsDialog::SettingsDialog(QWidget *parent)
	: QDialog(parent)
{
	resize(660, 700);
	setMinimumWidth(580);

	auto *root = new QVBoxLayout(this);
	root->setContentsMargins(20, 20, 20, 18);
	root->setSpacing(10);

	// ── Header ────────────────────────────────────────────────────────────
	{
		auto *hdr = new QWidget(this);
		auto *hl = new QHBoxLayout(hdr);
		hl->setContentsMargins(4, 0, 0, 0);
		hl->setSpacing(14);
		hl->setAlignment(Qt::AlignVCenter);

		auto *logo = new QLabel(hdr);
		logo->setFixedSize(48, 48);
		logo->setAlignment(Qt::AlignCenter);
		logo->setPixmap(QApplication::windowIcon().pixmap(48, 48));

		auto *tv = new QVBoxLayout();
		tv->setSpacing(3);
		tv->setContentsMargins(0, 2, 0, 2);
		headerTitle_ = new QLabel(hdr);
		headerTitle_->setStyleSheet("font-size:17px;font-weight:700;color:#e8e8f8;");
		headerSubtitle_ = new QLabel(hdr);
		headerSubtitle_->setStyleSheet("font-size:12px;color:#44446a;");
		versionLabel_ = new QLabel(hdr);
		versionLabel_->setStyleSheet("font-size:12px;color:#6b6b94;");
		tv->addWidget(headerTitle_);
		tv->addWidget(headerSubtitle_);
		tv->addWidget(versionLabel_);

		hl->addWidget(logo);
		hl->addLayout(tv);
		hl->addStretch();
		languageSelector_ = new ArrowComboBox(hdr);
		languageSelector_->setMinimumWidth(140);
		hl->addWidget(languageSelector_, 0, Qt::AlignRight | Qt::AlignVCenter);
		connect(languageSelector_, &QComboBox::currentIndexChanged, this, [this]() {
			if (!syncingLanguageSelector_)
				emit languageChanged(languageSelector_->currentData().toString());
		});
		root->addWidget(hdr);
	}
	root->addWidget(makeSep(this, "dialogSep"));

	// ── Input ─────────────────────────────────────────────────────────────
	{
		auto *card = makeCard(this);
		auto *cl = new QVBoxLayout(card);
		cl->setContentsMargins(18, 14, 18, 16);
		cl->setSpacing(0);
		inputSectionTitle_ = makeSectionTitle(QString(), card);
		cl->addWidget(inputSectionTitle_);
		cl->addSpacing(10);
		cl->addWidget(makeSep(card));
		cl->addSpacing(14);

		auto *row = new QHBoxLayout();
		row->setSpacing(12);
		inputModeSelector_ = new ArrowComboBox(card);
		inputModeLabel_ = makeFieldLabel(QString(), card);
		row->addWidget(inputModeLabel_);
		row->addWidget(inputModeSelector_, 1);
		cl->addLayout(row);
		cl->addSpacing(14);
		includeInScreenCapture_ = new ToggleSwitch(QString(), card);
		cl->addWidget(includeInScreenCapture_);

		connect(inputModeSelector_, &QComboBox::currentIndexChanged, this, [this]() {
			syncSectionVisibility();
		});
		root->addWidget(card);
	}

	// ── Hotkey ────────────────────────────────────────────────────────────
	{
		auto *card = makeCard(this);
		auto *cl = new QVBoxLayout(card);
		cl->setContentsMargins(18, 14, 18, 16);
		cl->setSpacing(0);
		hotkeySectionTitle_ = makeSectionTitle(QString(), card);
		cl->addWidget(hotkeySectionTitle_);
		cl->addSpacing(10);
		cl->addWidget(makeSep(card));
		cl->addSpacing(14);

		auto *row = new QHBoxLayout();
		row->setSpacing(12);
		hotkeyEdit_ = new HotkeyEdit(card);
		hotkeyLabel_ = makeFieldLabel(QString(), card);
		clearHotkeyButton_ = new QPushButton(card);
		row->addWidget(hotkeyLabel_);
		row->addWidget(hotkeyEdit_, 1);
		row->addWidget(clearHotkeyButton_);
		cl->addLayout(row);
		cl->addSpacing(10);

		hotkeyStatus_ = new QLabel(card);
		hotkeyStatus_->setObjectName("statusLabel");
		hotkeyStatus_->setWordWrap(true);
		cl->addWidget(hotkeyStatus_);

		connect(clearHotkeyButton_, &QPushButton::clicked, this, [this]() {
			hotkeyEdit_->clearHotkey();
		});
		root->addWidget(card);
	}

	// ── Spout2 Overlay ────────────────────────────────────────────────────
	{
		auto *card = makeCard(this);
		auto *cl = new QVBoxLayout(card);
		cl->setContentsMargins(18, 14, 18, 16);
		cl->setSpacing(0);
		spoutSectionTitle_ = makeSectionTitle(QString(), card);
		cl->addWidget(spoutSectionTitle_);
		cl->addSpacing(10);
		cl->addWidget(makeSep(card));
		cl->addSpacing(14);

		spoutEnabled_ = new ToggleSwitch(QString(), card);
		cl->addWidget(spoutEnabled_);
		cl->addSpacing(14);

		spoutContent_ = new QWidget(card);
		auto *contentLayout = new QVBoxLayout(spoutContent_);
		contentLayout->setContentsMargins(0, 0, 0, 0);
		contentLayout->setSpacing(0);

		auto *monitorRow = new QHBoxLayout();
		monitorRow->setSpacing(12);
		monitorSelector_ = new ArrowComboBox(card);
		monitorLabel_ = makeFieldLabel(QString(), card);
		monitorRow->addWidget(monitorLabel_);
		monitorRow->addWidget(monitorSelector_, 1);
		contentLayout->addLayout(monitorRow);
		contentLayout->addSpacing(8);

		auto *senderRow = new QHBoxLayout();
		senderRow->setSpacing(12);
		senderSelector_ = new ArrowComboBox(card);
		senderLabel_ = makeFieldLabel(QString(), card);
		refreshSendersButton_ = new QPushButton(card);
		senderRow->addWidget(senderLabel_);
		senderRow->addWidget(senderSelector_, 1);
		senderRow->addWidget(refreshSendersButton_);
		contentLayout->addLayout(senderRow);

		// Test row
		contentLayout->addSpacing(14);
		auto *r3 = new QHBoxLayout();
		r3->setSpacing(12);
		testButton_ = new QPushButton(card);
		testButton_->setProperty("role", "primary");
		spoutStatus_ = new QLabel(card);
		spoutStatus_->setObjectName("statusLabel");
		spoutStatus_->setTextFormat(Qt::RichText);
		spoutStatus_->setWordWrap(true);
		r3->addWidget(testButton_);
		r3->addWidget(spoutStatus_, 1);
		contentLayout->addLayout(r3);
		cl->addWidget(spoutContent_);

		connect(testButton_, &QPushButton::clicked, this, &SettingsDialog::spoutTestRequested);
		connect(refreshSendersButton_, &QPushButton::clicked, this, &SettingsDialog::spoutSendersRefreshRequested);
		connect(spoutEnabled_, &QAbstractButton::toggled, this, [this]() {
			syncSectionVisibility();
		});
		root->addWidget(card);
	}

	// ── NDI Receiver ──────────────────────────────────────────────────────
	{
		auto *card = makeCard(this);
		auto *cl = new QVBoxLayout(card);
		cl->setContentsMargins(18, 14, 18, 16);
		cl->setSpacing(0);
		ndiSectionTitle_ = makeSectionTitle(QString(), card);
		cl->addWidget(ndiSectionTitle_);
		cl->addSpacing(10);
		cl->addWidget(makeSep(card));
		cl->addSpacing(14);

		ndiEnabled_ = new ToggleSwitch(QString(), card);
		cl->addWidget(ndiEnabled_);
		cl->addSpacing(14);

		ndiContent_ = new QWidget(card);
		auto *contentLayout = new QVBoxLayout(ndiContent_);
		contentLayout->setContentsMargins(0, 0, 0, 0);
		contentLayout->setSpacing(0);

		ndiControls_ = new QWidget(card);
		auto *ndiControlsLayout = new QVBoxLayout(ndiControls_);
		ndiControlsLayout->setContentsMargins(0, 0, 0, 0);
		ndiControlsLayout->setSpacing(0);

		auto *sourceRow = new QHBoxLayout();
		sourceRow->setSpacing(12);
		ndiSourceSelector_ = new ArrowComboBox(card);
		ndiSourceLabel_ = makeFieldLabel(QString(), card);
		refreshNdiSourcesButton_ = new QPushButton(card);
		sourceRow->addWidget(ndiSourceLabel_);
		sourceRow->addWidget(ndiSourceSelector_, 1);
		sourceRow->addWidget(refreshNdiSourcesButton_);
		ndiControlsLayout->addLayout(sourceRow);
		ndiControlsLayout->addSpacing(14);

		auto *testRow = new QHBoxLayout();
		testRow->setSpacing(12);
		ndiTestButton_ = new QPushButton(card);
		ndiStatus_ = new QLabel(card);
		ndiStatus_->setObjectName("statusLabel");
		ndiStatus_->setTextFormat(Qt::RichText);
		ndiStatus_->setWordWrap(true);
		testRow->addWidget(ndiTestButton_);
		testRow->addWidget(ndiStatus_, 1);
		ndiControlsLayout->addLayout(testRow);
		contentLayout->addWidget(ndiControls_);
		contentLayout->addSpacing(10);

		ndiRuntimeHint_ = new QLabel(card);
		ndiRuntimeHint_->setObjectName("statusLabel");
		ndiRuntimeHint_->setWordWrap(true);
		contentLayout->addWidget(ndiRuntimeHint_);
		contentLayout->addSpacing(10);

		ndiInstallButton_ = new QPushButton(card);
		ndiInstallButton_->setProperty("role", "primary");
		contentLayout->addWidget(ndiInstallButton_, 0, Qt::AlignLeft);
		cl->addWidget(ndiContent_);

		connect(ndiTestButton_, &QPushButton::clicked, this, &SettingsDialog::ndiTestRequested);
		connect(refreshNdiSourcesButton_, &QPushButton::clicked, this, &SettingsDialog::ndiSourcesRefreshRequested);
		connect(ndiInstallButton_, &QPushButton::clicked, this, []() {
			QDesktopServices::openUrl(ndiToolsUrl());
		});
		connect(ndiEnabled_, &QAbstractButton::toggled, this, [this]() {
			syncSectionVisibility();
		});
		root->addWidget(card);
	}

	// ── Virtueller Anzeigebereich ─────────────────────────────────────────
	{
		auto *card = makeCard(this);
		auto *cl = new QVBoxLayout(card);
		cl->setContentsMargins(18, 14, 18, 16);
		cl->setSpacing(0);
		viewportSectionTitle_ = makeSectionTitle(QString(), card);
		cl->addWidget(viewportSectionTitle_);
		cl->addSpacing(10);
		cl->addWidget(makeSep(card));
		cl->addSpacing(14);

		viewportEnabled_ = new ToggleSwitch(QString(), card);
		cl->addWidget(viewportEnabled_);
		cl->addSpacing(14);

		viewportContent_ = new QWidget(card);
		auto *viewportLayout = new QVBoxLayout(viewportContent_);
		viewportLayout->setContentsMargins(0, 0, 0, 0);
		viewportLayout->setSpacing(0);

		// Size row
		auto *sizeRow = new QHBoxLayout();
		sizeRow->setSpacing(8);
		viewportWidth_ = new QSpinBox(card);
		viewportHeight_ = new QSpinBox(card);
		viewportWidth_->setRange(1, 16384);
		viewportHeight_->setRange(1, 16384);

		viewportWidthLabel_ = new QLabel(card);
		viewportWidthLabel_->setObjectName("mutedLabel");
		viewportHeightLabel_ = new QLabel(card);
		viewportHeightLabel_->setObjectName("mutedLabel");
		viewportWidthUnit_ = new QLabel(card);
		viewportWidthUnit_->setObjectName("mutedLabel");
		viewportHeightUnit_ = new QLabel(card);
		viewportHeightUnit_->setObjectName("mutedLabel");

		sizeRow->addWidget(viewportWidthLabel_);
		sizeRow->addWidget(viewportWidth_);
		sizeRow->addWidget(viewportWidthUnit_);
		sizeRow->addSpacing(14);
		sizeRow->addWidget(viewportHeightLabel_);
		sizeRow->addWidget(viewportHeight_);
		sizeRow->addWidget(viewportHeightUnit_);
		sizeRow->addStretch();
		viewportLayout->addLayout(sizeRow);
		viewportLayout->addSpacing(14);

		// Anchor
		anchorLabel_ = new QLabel(card);
		anchorLabel_->setObjectName("mutedLabel");
		viewportLayout->addWidget(anchorLabel_);
		viewportLayout->addSpacing(8);

		auto *anchorWrap = new QWidget(card);
		auto *ag = new QGridLayout(anchorWrap);
		ag->setSpacing(5);
		ag->setContentsMargins(0, 0, 0, 0);
		buildAnchorGrid(ag);
		viewportLayout->addWidget(anchorWrap, 0, Qt::AlignLeft);
		cl->addWidget(viewportContent_);

		connect(viewportEnabled_, &QAbstractButton::toggled, this, [this]() {
			syncSectionVisibility();
		});

		root->addWidget(card);
	}

	root->addStretch();
	root->addWidget(makeSep(this, "dialogSep"));

	// ── Footer ────────────────────────────────────────────────────────────
	{
		auto *fl = new QHBoxLayout();
		fl->setSpacing(8);
		helpButton_ = new QPushButton(this);
		updateButton_ = new QPushButton(this);
		fl->addWidget(helpButton_);
		fl->addWidget(updateButton_);
		fl->addStretch();
		cancelButton_ = new QPushButton(this);
		saveButton_ = new QPushButton(this);
		saveButton_->setProperty("role", "primary");
		fl->addWidget(cancelButton_);
		fl->addWidget(saveButton_);
		root->addLayout(fl);

		connect(helpButton_,   &QPushButton::clicked, this, &SettingsDialog::onboardingRequested);
		connect(updateButton_, &QPushButton::clicked, this, &SettingsDialog::updateCheckRequested);
		connect(saveButton_,   &QPushButton::clicked, this, &SettingsDialog::save);
		connect(cancelButton_, &QPushButton::clicked, this, &SettingsDialog::hide);
	}

	updateStatus_ = new QLabel(this);
	updateStatus_->setObjectName("mutedLabel");
	updateStatus_->setWordWrap(true);
	root->addWidget(updateStatus_);

	retranslateUi();
	syncSectionVisibility();
}

void SettingsDialog::setConfig(const AppConfig &config)
{
	setLanguage(resolveAppLanguage(config.ui.language));

	// Set thumb position immediately (no animation on initial load)
	spoutEnabled_->setThumbPos(config.overlay.spoutReceiver.enabled ? 1.0 : 0.0);
	spoutEnabled_->setChecked(config.overlay.spoutReceiver.enabled);
	const bool ndiEnabled = ndiRuntimeAvailable_ && config.overlay.ndiReceiver.enabled;
	ndiEnabled_->setEnabled(ndiRuntimeAvailable_);
	ndiEnabled_->setThumbPos(ndiEnabled ? 1.0 : 0.0);
	ndiEnabled_->setChecked(ndiEnabled);
	hotkeyEdit_->setHotkeyConfig(config.overlay.hotkey);
	syncingLanguageSelector_ = true;
	languageSelector_->setCurrentIndex(qMax(0, languageSelector_->findData(config.ui.language)));
	syncingLanguageSelector_ = false;
	populateMonitorOptions(config.overlay.monitor);
	repopulateSenderOptions(config.overlay.spoutReceiver.senderName);
	repopulateNdiSourceOptions(config.overlay.ndiReceiver.sourceName);
	const QString inputMode = overlayInputModeToString(config.overlay.input);
	const int inputIndex = inputModeSelector_->findData(inputMode);
	if (inputIndex >= 0)
		inputModeSelector_->setCurrentIndex(inputIndex);
	else
		inputModeSelector_->setCurrentIndex(qMax(0, inputModeSelector_->findData("spout")));
	includeInScreenCapture_->setThumbPos(config.overlay.includeInScreenCapture ? 1.0 : 0.0);
	includeInScreenCapture_->setChecked(config.overlay.includeInScreenCapture);

	viewportEnabled_->setThumbPos(config.overlay.virtualViewport.enabled ? 1.0 : 0.0);
	viewportEnabled_->setChecked(config.overlay.virtualViewport.enabled);
	viewportWidth_->setValue(config.overlay.virtualViewport.width);
	viewportHeight_->setValue(config.overlay.virtualViewport.height);
	const int anchorIndex = viewportAnchorIndex(config.overlay.virtualViewport.anchor);
	if (auto *button = anchorGroup_->button(anchorIndex))
		button->setChecked(true);
	syncSectionVisibility();
}

AppConfig SettingsDialog::config() const
{
	AppConfig config;
	config.ui.language = normalizeAppLanguageSetting(languageSelector_->currentData().toString());
	config.overlay.input = overlayInputModeFromString(inputModeSelector_->currentData().toString());
	if (!ndiRuntimeAvailable_ && config.overlay.input == OverlayInputMode::Ndi)
		config.overlay.input = OverlayInputMode::Spout;
	config.overlay.includeInScreenCapture = includeInScreenCapture_->isChecked();
	config.overlay.monitor = monitorSelector_->currentData().toString().trimmed();
	if (config.overlay.monitor.isEmpty())
		config.overlay.monitor = "primary";
	config.overlay.spoutReceiver.enabled = spoutEnabled_->isChecked();
	config.overlay.spoutReceiver.senderName = senderSelector_->currentData().toString().trimmed();
	if (config.overlay.spoutReceiver.senderName.isEmpty())
		config.overlay.spoutReceiver.senderName = "OBS Spout2 Output";
	config.overlay.ndiReceiver.enabled = ndiRuntimeAvailable_ && ndiEnabled_->isChecked();
	config.overlay.ndiReceiver.sourceName = ndiSourceSelector_->currentData().toString().trimmed();
	config.overlay.ndiReceiver.preferExactName = true;
	config.overlay.hotkey = hotkeyEdit_->hotkeyConfig();
	config.overlay.virtualViewport.enabled = viewportEnabled_->isChecked();
	config.overlay.virtualViewport.width = viewportWidth_->value();
	config.overlay.virtualViewport.height = viewportHeight_->value();
	config.overlay.virtualViewport.anchor = viewportAnchors().value(anchorGroup_->checkedId(), "middle-center");
	return config;
}

void SettingsDialog::setAvailableSenders(const QStringList &senders)
{
	availableSenders_ = senders;
	availableSenders_.removeDuplicates();
	const QString currentSender = senderSelector_->currentData().toString().trimmed();
	repopulateSenderOptions(currentSender);
}

void SettingsDialog::setAvailableNdiSources(const QStringList &sources)
{
	availableNdiSources_ = sources;
	availableNdiSources_.removeDuplicates();
	const QString currentSource = ndiSourceSelector_->currentData().toString().trimmed();
	repopulateNdiSourceOptions(currentSource);
}

void SettingsDialog::populateMonitorOptions(const QString &selectedMonitor)
{
	monitorSelector_->clear();

	const QList<QScreen *> screens = QGuiApplication::screens();
	int selectedIndex = -1;
	for (int i = 0; i < screens.size(); ++i) {
		QScreen *screen = screens.at(i);
		const QString value = monitorId(screen, i);
		monitorSelector_->addItem(::monitorLabel(screen, i, language_), value);
		if ((selectedMonitor == "primary" && screen == QGuiApplication::primaryScreen()) || value == selectedMonitor)
			selectedIndex = i;
	}

	if (selectedIndex < 0 && monitorSelector_->count() > 0) {
		QScreen *primary = QGuiApplication::primaryScreen();
		for (int i = 0; i < screens.size(); ++i) {
			if (screens.at(i) == primary) {
				selectedIndex = i;
				break;
			}
		}
	}
	if (selectedIndex < 0 && monitorSelector_->count() > 0)
		selectedIndex = 0;
	if (selectedIndex >= 0)
		monitorSelector_->setCurrentIndex(selectedIndex);
}

void SettingsDialog::setLanguage(AppLanguage language)
{
	if (language_ == language)
		return;
	language_ = language;
	retranslateUi();
}

void SettingsDialog::retranslateUi()
{
	const QString currentLanguage = languageSelector_->currentData().toString();
	const QString currentSender = senderSelector_->currentData().toString().trimmed();
	const QString currentNdiSource = ndiSourceSelector_->currentData().toString().trimmed();
	setWindowTitle(text(TextId::SettingsWindowTitle, language_));
	headerTitle_->setText(text(TextId::SettingsHeaderTitle, language_));
	headerSubtitle_->setText(text(TextId::SettingsHeaderSubtitle, language_));
	versionLabel_->setText(text(TextId::SettingsVersionLabel, language_).arg(QStringLiteral(PHANTOM_MIRROR_VERSION)));
	syncingLanguageSelector_ = true;
	languageSelector_->clear();
	languageSelector_->addItem(text(TextId::LanguageOptionSystem, language_), "system");
	languageSelector_->addItem(text(TextId::LanguageOptionGerman, language_), "de");
	languageSelector_->addItem(text(TextId::LanguageOptionEnglish, language_), "en");
	languageSelector_->setCurrentIndex(qMax(0, languageSelector_->findData(currentLanguage.isEmpty() ? "system" : currentLanguage)));
	syncingLanguageSelector_ = false;
	const QString currentInputMode = inputModeSelector_->currentData().toString();
	inputSectionTitle_->setText(text(TextId::InputSectionTitle, language_).toUpper());
	inputModeLabel_->setText(text(TextId::InputModeLabel, language_));
	{
		const QSignalBlocker blocker(inputModeSelector_);
	inputModeSelector_->clear();
	inputModeSelector_->addItem(text(TextId::InputModeSpout, language_), "spout");
	if (ndiRuntimeAvailable_)
		inputModeSelector_->addItem(text(TextId::InputModeNdi, language_), "ndi");
	inputModeSelector_->setCurrentIndex(qMax(0, inputModeSelector_->findData(currentInputMode.isEmpty() ? "spout" : currentInputMode)));
	}
	includeInScreenCapture_->setText(text(TextId::IncludeInScreenCapture, language_));
	hotkeySectionTitle_->setText(text(TextId::HotkeySectionTitle, language_).toUpper());
	hotkeyLabel_->setText(text(TextId::HotkeyLabel, language_));
	hotkeyEdit_->setPlaceholderText(text(TextId::HotkeyPlaceholder, language_));
	clearHotkeyButton_->setText(text(TextId::HotkeyClear, language_));
	spoutSectionTitle_->setText(text(TextId::SpoutSectionTitle, language_).toUpper());
	spoutEnabled_->setText(text(TextId::SpoutEnabled, language_));
	monitorLabel_->setText(text(TextId::MonitorLabel, language_));
	senderLabel_->setText(text(TextId::SenderLabel, language_));
	refreshSendersButton_->setText(text(TextId::SenderRefresh, language_));
	testButton_->setText(text(TextId::TestConnection, language_));
	ndiSectionTitle_->setText(text(TextId::NdiSectionTitle, language_).toUpper());
	ndiEnabled_->setText(text(TextId::NdiEnabled, language_));
	ndiSourceLabel_->setText(text(TextId::NdiSourceLabel, language_));
	refreshNdiSourcesButton_->setText(text(TextId::SenderRefresh, language_));
	ndiTestButton_->setText(text(TextId::TestConnection, language_));
	ndiInstallButton_->setText(text(TextId::NdiInstallAction, language_));
	viewportSectionTitle_->setText(text(TextId::ViewportSectionTitle, language_).toUpper());
	viewportEnabled_->setText(text(TextId::ViewportEnabled, language_));
	viewportWidthLabel_->setText(text(TextId::WidthLabel, language_));
	viewportHeightLabel_->setText(text(TextId::HeightLabel, language_));
	viewportWidthUnit_->setText(text(TextId::PixelsSuffix, language_));
	viewportHeightUnit_->setText(text(TextId::PixelsSuffix, language_));
	anchorLabel_->setText(text(TextId::AnchorPositionLabel, language_));
	helpButton_->setText(text(TextId::SetupHelp, language_));
	updateButton_->setText(text(TextId::Updates, language_));
	cancelButton_->setText(text(TextId::Cancel, language_));
	saveButton_->setText(QString("  %1  ").arg(text(TextId::Save, language_)));

	const QString selectedMonitor = monitorSelector_->currentData().toString();
	populateMonitorOptions(selectedMonitor);
	repopulateSenderOptions(currentSender);
	repopulateNdiSourceOptions(currentNdiSource);
	if (hasSpoutStatus_)
		setSpoutStatus(lastSpoutStatus_);
	else
		spoutStatus_->setText(QString(
			"<span style='color:#2e2e50'>&#9679;</span>"
			"<span style='color:#40406a'>  %1</span>").arg(text(TextId::SpoutNotTested, language_).toHtmlEscaped()));
	if (hasNdiStatus_)
		setNdiStatus(lastNdiStatus_);
	else
		ndiStatus_->setText(QString(
			"<span style='color:#2e2e50'>&#9679;</span>"
			"<span style='color:#40406a'>  %1</span>").arg(text(TextId::NdiNotTested, language_).toHtmlEscaped()));
	if (!ndiRuntimeAvailable_) {
		QString hint = QString("<span style='color:#f59e0b'>&#9679;</span>"
				       "<span style='color:#ef4444'>  %1</span><br/><span style='color:#52527a'>%2</span>")
				   .arg(text(TextId::NdiRuntimeMissing, language_).toHtmlEscaped(),
				        text(TextId::NdiInstallRequired, language_).toHtmlEscaped());
		if (!ndiRuntimeError_.trimmed().isEmpty())
			hint += QString("<br/><span style='color:#40406a'>%1</span>").arg(ndiRuntimeError_.toHtmlEscaped());
		ndiRuntimeHint_->setText(hint);
	} else {
		ndiRuntimeHint_->clear();
	}
	if (hotkeyStatus_->text().isEmpty())
		hotkeyStatus_->setText(QString("<span style='color:#40406a'>%1</span>")
			.arg(text(TextId::HotkeyUnset, language_).toHtmlEscaped()));
	updateStatus_->setText(updateStatusText_);
}

void SettingsDialog::repopulateSenderOptions(const QString &selectedSender)
{
	senderSelector_->clear();
	for (const QString &sender : availableSenders_) {
		if (!sender.trimmed().isEmpty())
			senderSelector_->addItem(sender, sender);
	}
	int selectedIndex = senderSelector_->findData(selectedSender);
	if (selectedIndex >= 0) {
		senderSelector_->setCurrentIndex(selectedIndex);
	} else if (!selectedSender.trimmed().isEmpty()) {
		senderSelector_->addItem(selectedSender, selectedSender);
		senderSelector_->setCurrentIndex(senderSelector_->count() - 1);
	} else if (senderSelector_->count() > 0) {
		senderSelector_->setCurrentIndex(0);
	}
}

void SettingsDialog::repopulateNdiSourceOptions(const QString &selectedSource)
{
	ndiSourceSelector_->clear();
	for (const QString &source : availableNdiSources_) {
		if (!source.trimmed().isEmpty())
			ndiSourceSelector_->addItem(source, source);
	}
	int selectedIndex = ndiSourceSelector_->findData(selectedSource);
	if (selectedIndex >= 0) {
		ndiSourceSelector_->setCurrentIndex(selectedIndex);
	} else if (!selectedSource.trimmed().isEmpty()) {
		ndiSourceSelector_->addItem(selectedSource, selectedSource);
		ndiSourceSelector_->setCurrentIndex(ndiSourceSelector_->count() - 1);
	} else if (ndiSourceSelector_->count() > 0) {
		ndiSourceSelector_->setCurrentIndex(0);
	}
}

void SettingsDialog::syncSectionVisibility()
{
	const bool ndiActive = overlayInputModeFromString(inputModeSelector_->currentData().toString()) == OverlayInputMode::Ndi;
	if (spoutContent_)
		spoutContent_->setVisible(spoutEnabled_ && spoutEnabled_->isChecked());
	if (ndiContent_)
		ndiContent_->setVisible(!ndiRuntimeAvailable_ || (ndiEnabled_ && ndiEnabled_->isChecked()));
	if (ndiEnabled_)
		ndiEnabled_->setVisible(ndiRuntimeAvailable_);
	if (ndiControls_)
		ndiControls_->setVisible(ndiRuntimeAvailable_);
	if (viewportContent_)
		viewportContent_->setVisible(viewportEnabled_ && viewportEnabled_->isChecked());
	if (ndiSourceSelector_)
		ndiSourceSelector_->setEnabled(ndiRuntimeAvailable_);
	if (refreshNdiSourcesButton_)
		refreshNdiSourcesButton_->setEnabled(ndiRuntimeAvailable_);
	if (ndiTestButton_)
		ndiTestButton_->setEnabled(ndiRuntimeAvailable_);
	if (ndiRuntimeHint_)
		ndiRuntimeHint_->setVisible(!ndiRuntimeAvailable_);
	if (ndiInstallButton_)
		ndiInstallButton_->setVisible(!ndiRuntimeAvailable_);
	if (testButton_)
		testButton_->setProperty("role", ndiActive ? "" : "primary");
	if (ndiTestButton_)
		ndiTestButton_->setProperty("role", ndiActive ? "primary" : "");
	style()->unpolish(testButton_);
	style()->polish(testButton_);
	style()->unpolish(ndiTestButton_);
	style()->polish(ndiTestButton_);
}

void SettingsDialog::setSpoutStatus(const SpoutStatus &status)
{
	lastSpoutStatus_ = status;
	hasSpoutStatus_ = true;
	QString html;
	if (status.running && (status.connected || status.senderFound)) {
		html = "<span style='color:#10b981'>&#9679;</span>"
			   "<span style='color:#94a3b8'>  " + text(TextId::SpoutConnected, language_);
		if (!status.senderName.isEmpty())
			html += QString(" &mdash; <b style='color:#c8d4f0'>%1</b>")
						.arg(status.senderName.toHtmlEscaped());
		if (status.width > 0 && status.height > 0)
			html += QString("  <span style='color:#52527a'>%1&times;%2</span>")
						.arg(status.width).arg(status.height);
		if (status.fps > 0.0)
			html += QString("  <span style='color:#52527a'>%1 FPS</span>")
						.arg(status.fps, 0, 'f', 1);
		html += "</span>";
	} else if (status.running) {
		html = QString("<span style='color:#f59e0b'>&#9679;</span>"
			   "<span style='color:#52527a'>  %1</span>")
				   .arg(text(TextId::SpoutWaiting, language_).toHtmlEscaped());
	} else {
		html = QString("<span style='color:#2e2e50'>&#9679;</span>"
			   "<span style='color:#40406a'>  %1</span>")
				   .arg(text(TextId::SpoutReceiverStopped, language_).toHtmlEscaped());
	}
	if (!status.lastError.isEmpty())
		html += QString(" <span style='color:#ef4444'>&middot; %1</span>")
					.arg(status.lastError.toHtmlEscaped());
	spoutStatus_->setText(html);
}

void SettingsDialog::setNdiStatus(const NdiStatus &status)
{
	lastNdiStatus_ = status;
	hasNdiStatus_ = true;
	QString html;
	if (status.running && (status.connected || status.sourceFound)) {
		html = "<span style='color:#10b981'>&#9679;</span>"
			   "<span style='color:#94a3b8'>  " + text(TextId::NdiConnected, language_);
		if (!status.sourceName.isEmpty())
			html += QString(" &mdash; <b style='color:#c8d4f0'>%1</b>")
						.arg(status.sourceName.toHtmlEscaped());
		if (status.width > 0 && status.height > 0)
			html += QString("  <span style='color:#52527a'>%1&times;%2</span>")
						.arg(status.width).arg(status.height);
		if (status.fps > 0.0)
			html += QString("  <span style='color:#52527a'>%1 FPS</span>")
						.arg(status.fps, 0, 'f', 1);
		html += "</span>";
	} else if (status.running) {
		html = QString("<span style='color:#f59e0b'>&#9679;</span>"
			       "<span style='color:#52527a'>  %1</span>")
				   .arg(text(TextId::NdiWaiting, language_).toHtmlEscaped());
	} else {
		html = QString("<span style='color:#2e2e50'>&#9679;</span>"
			       "<span style='color:#40406a'>  %1</span>")
				   .arg(text(TextId::NdiReceiverStopped, language_).toHtmlEscaped());
	}
	if (!status.lastError.isEmpty())
		html += QString(" <span style='color:#ef4444'>&middot; %1</span>")
					.arg(status.lastError.toHtmlEscaped());
	ndiStatus_->setText(html);
}

void SettingsDialog::setNdiAvailability(bool available, const QString &error)
{
	ndiRuntimeAvailable_ = available;
	ndiRuntimeError_ = error.trimmed();
	if (ndiEnabled_) {
		ndiEnabled_->setEnabled(ndiRuntimeAvailable_);
		if (!ndiRuntimeAvailable_) {
			ndiEnabled_->setThumbPos(0.0);
			ndiEnabled_->setChecked(false);
		}
	}
	retranslateUi();
	syncSectionVisibility();
}

void SettingsDialog::setUpdateStatus(const QString &status)
{
	updateStatusText_ = status;
	updateStatus_->setText(status);
}

void SettingsDialog::setHotkeyStatus(const QString &status)
{
	hotkeyStatus_->setText(status);
}

void SettingsDialog::save()
{
	QString error;
	const AppConfig nextConfig = config();
	if (!saveConfig(nextConfig, &error)) {
		QMessageBox::warning(this, text(TextId::SaveErrorTitle, language_),
							 formatSaveError(language_, error));
		return;
	}
	emit configSaved(nextConfig);
}

void SettingsDialog::buildAnchorGrid(QGridLayout *layout)
{
	anchorGroup_ = new QButtonGroup(this);
	const QStringList labels{"↖", "↑", "↗", "←", "·", "→", "↙", "↓", "↘"};
	const QStringList anchors = viewportAnchors();
	for (int i = 0; i < anchors.size(); ++i) {
		auto *btn = new QPushButton(labels.value(i), this);
		btn->setObjectName("anchorBtn");
		btn->setCheckable(true);
		btn->setToolTip(anchors.value(i));
		anchorGroup_->addButton(btn, i);
		layout->addWidget(btn, i / 3, i % 3);
	}
}
