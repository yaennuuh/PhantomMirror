#include "onboarding_dialog.h"

#include "app_icon.h"
#include "arrow_combo_box.h"
#include "app_strings.h"
#include "toggle_switch.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

OnboardingDialog::OnboardingDialog(QWidget *parent)
	: QDialog(parent)
{
	resize(600, 580);
	setMinimumWidth(540);

	auto *root = new QVBoxLayout(this);
	root->setContentsMargins(24, 24, 24, 20);
	root->setSpacing(16);

	// ── Header ────────────────────────────────────────────────────────────
	{
		auto *hdr = new QWidget(this);
		auto *hl = new QHBoxLayout(hdr);
		hl->setContentsMargins(0, 0, 0, 0);
		hl->setSpacing(14);

		auto *logo = new QLabel(hdr);
		logo->setFixedSize(48, 48);
		logo->setAlignment(Qt::AlignCenter);
		logo->setPixmap(appLogoPixmap(48));

		auto *tv = new QVBoxLayout();
		tv->setSpacing(3);
		headerTitle_ = new QLabel(hdr);
		headerTitle_->setStyleSheet("font-size:17px;font-weight:700;color:#e8e8f8;");
		headerSubtitle_ = new QLabel(hdr);
		headerSubtitle_->setStyleSheet("font-size:12px;color:#44446a;");
		tv->addWidget(headerTitle_);
		tv->addWidget(headerSubtitle_);

		hl->addWidget(logo);
		hl->addLayout(tv);
		hl->addStretch();
		languageSelector_ = new ArrowComboBox(hdr);
		languageSelector_->setMinimumWidth(140);
		hl->addWidget(languageSelector_, 0, Qt::AlignRight | Qt::AlignVCenter);
		root->addWidget(hdr);
	}

	auto makeSep = [this]() {
		auto *sep = new QFrame(this);
		sep->setFrameShape(QFrame::HLine);
		sep->setObjectName("dialogSep");
		sep->setFixedHeight(1);
		return sep;
	};
	root->addWidget(makeSep());

	// ── Content ───────────────────────────────────────────────────────────
	content_ = new QTextBrowser(this);
	content_->setOpenExternalLinks(true);
	content_->document()->setDefaultStyleSheet(R"(
		body  { color:#b0b0cc; font-family:'Segoe UI Variable Text','Segoe UI'; font-size:13px; margin:2px 4px; }
		h3    { color:#8b5cf6; font-size:10px; font-weight:700; margin:18px 0 6px 0; letter-spacing:1.4px; }
		p     { color:#8080a8; margin:0 0 8px 0; line-height:1.65; }
		ul    { color:#8080a8; margin:0 0 8px 16px; padding:0; }
		li    { margin:3px 0; line-height:1.65; }
		b     { color:#c8d4f0; font-weight:600; }
		code  { background-color:#1a1a2a; color:#a78bfa; padding:2px 8px;
		        border-radius:5px; font-size:12px; font-family:'Cascadia Code','Consolas',monospace; }
		a     { color:#7c3aed; text-decoration:none; }
	)");
	root->addWidget(content_, 1);

	root->addWidget(makeSep());

	// ── Footer ────────────────────────────────────────────────────────────
	dontShow_ = new ToggleSwitch(QString(), this);
	dontShow_->setThumbPos(1.0);
	dontShow_->setChecked(true);
	root->addWidget(dontShow_);
	root->addSpacing(4);

	auto *br = new QHBoxLayout();
	br->setSpacing(8);
	closeButton_ = new QPushButton(this);
	settingsButton_ = new QPushButton(this);
	settingsButton_->setProperty("role", "primary");
	br->addStretch();
	br->addWidget(closeButton_);
	br->addWidget(settingsButton_);
	root->addLayout(br);

	connect(settingsButton_, &QPushButton::clicked, this, [this]() {
		if (dontShow_->isChecked())
			emit completed();
		accept();
	});
	connect(closeButton_, &QPushButton::clicked, this, [this]() {
		if (dontShow_->isChecked())
			emit completed();
		reject();
	});
	connect(languageSelector_, &QComboBox::currentIndexChanged, this, [this]() {
		if (!syncingLanguageSelector_)
			emit languageChanged(languageSelector_->currentData().toString());
	});

	retranslateUi();
}

void OnboardingDialog::setLanguage(AppLanguage language)
{
	if (language_ == language)
		return;
	language_ = language;
	retranslateUi();
}

void OnboardingDialog::setLanguageSetting(const QString &languageSetting)
{
	if (!languageSelector_)
		return;
	syncingLanguageSelector_ = true;
	languageSelector_->setCurrentIndex(qMax(0, languageSelector_->findData(normalizeAppLanguage(languageSetting))));
	syncingLanguageSelector_ = false;
}

void OnboardingDialog::retranslateUi()
{
	const QString currentLanguage = languageSelector_ ? languageSelector_->currentData().toString() : QString("system");
	setWindowTitle(text(TextId::OnboardingWindowTitle, language_));
	headerTitle_->setText(text(TextId::OnboardingHeaderTitle, language_));
	headerSubtitle_->setText(text(TextId::OnboardingHeaderSubtitle, language_));
	syncingLanguageSelector_ = true;
	languageSelector_->clear();
	languageSelector_->addItem(text(TextId::LanguageOptionSystem, language_), "system");
	languageSelector_->addItem(text(TextId::LanguageOptionGerman, language_), "de");
	languageSelector_->addItem(text(TextId::LanguageOptionEnglish, language_), "en");
	languageSelector_->setCurrentIndex(qMax(0, languageSelector_->findData(currentLanguage.isEmpty() ? "system" : currentLanguage)));
	syncingLanguageSelector_ = false;
	content_->setHtml(onboardingHtml(language_));
	dontShow_->setText(text(TextId::OnboardingDontShow, language_));
	closeButton_->setText(text(TextId::OnboardingClose, language_));
	settingsButton_->setText(QString("  %1  ").arg(text(TextId::OnboardingOpenSettings, language_)));
}
