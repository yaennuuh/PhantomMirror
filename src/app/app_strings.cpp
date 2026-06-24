#include "app_strings.h"

#include <QLocale>
#include <QRect>
#include <QScreen>

namespace {

QString englishText(TextId id)
{
	switch (id) {
	case TextId::LanguageSectionTitle: return "Interface";
	case TextId::LanguageLabel: return "Language";
	case TextId::LanguageOptionSystem: return "System";
	case TextId::LanguageOptionGerman: return "Deutsch";
	case TextId::LanguageOptionEnglish: return "English";
	case TextId::SettingsWindowTitle: return "Phantom Mirror Settings";
	case TextId::SettingsHeaderTitle: return "Phantom Mirror Settings";
	case TextId::SettingsHeaderSubtitle: return "Configure overlay and connection";
	case TextId::SettingsVersionLabel: return "Version %1";
	case TextId::HotkeySectionTitle: return "Overlay Hotkey";
	case TextId::HotkeyLabel: return "Toggle overlay";
	case TextId::HotkeyPlaceholder: return "Press a hotkey";
	case TextId::HotkeyClear: return "Clear";
	case TextId::HotkeyUnset: return "No hotkey configured";
	case TextId::HotkeyActive: return "Hotkey active: %1";
	case TextId::HotkeyInactive: return "Hotkey disabled";
	case TextId::HotkeyConflict: return "Hotkey could not be activated: %1";
	case TextId::HotkeySavedInactive: return "Saved, but currently inactive";
	case TextId::InputSectionTitle: return "Input";
	case TextId::InputModeLabel: return "Active input";
	case TextId::InputModeSpout: return "Spout2 (local PC)";
	case TextId::InputModeNdi: return "NDI (network)";
	case TextId::IncludeInScreenCapture: return "Include overlay in screen capture";
	case TextId::SpoutSectionTitle: return "Spout2 (Local PC)";
	case TextId::SpoutEnabled: return "Use local Spout2 source";
	case TextId::MonitorLabel: return "Target display";
	case TextId::SenderLabel: return "Sender";
	case TextId::SenderRefresh: return "Refresh";
	case TextId::SenderCustom: return "Custom";
	case TextId::SenderCustomNameLabel: return "Custom name";
	case TextId::SenderNameLabel: return "Sender name";
	case TextId::SenderPlaceholder: return "OBS Spout2 Output";
	case TextId::TestConnection: return "Test connection";
	case TextId::SpoutNotTested: return "Not tested";
	case TextId::NdiSectionTitle: return "NDI (Network)";
	case TextId::NdiEnabled: return "Use NDI source from network";
	case TextId::NdiSourceLabel: return "Source";
	case TextId::NdiSourceCustom: return "Custom";
	case TextId::NdiSourceNameLabel: return "Source name";
	case TextId::NdiSourcePlaceholder: return "Remote NDI Source";
	case TextId::NdiNotTested: return "Not tested";
	case TextId::NdiConnected: return "Connected";
	case TextId::NdiWaiting: return "Waiting for NDI source...";
	case TextId::NdiReceiverStopped: return "Receiver not started";
	case TextId::NdiRuntimeMissing: return "NDI Runtime is not installed";
	case TextId::NdiInstallRequired: return "Install NDI Tools before enabling NDI on this computer.";
	case TextId::NdiInstallAction: return "Install NDI";
	case TextId::ViewportSectionTitle: return "Virtual Viewport";
	case TextId::ViewportEnabled: return "Enable virtual viewport";
	case TextId::WidthLabel: return "Width";
	case TextId::HeightLabel: return "Height";
	case TextId::PixelsSuffix: return "px";
	case TextId::AnchorPositionLabel: return "Anchor position";
	case TextId::DebugSectionTitle: return "Debug";
	case TextId::DebugEnabled: return "Enable debug logging";
	case TextId::SetupHelp: return "Setup Help";
	case TextId::Updates: return "Updates";
	case TextId::Cancel: return "Cancel";
	case TextId::Save: return "Save";
	case TextId::OnboardingWindowTitle: return "Set up Phantom Mirror";
	case TextId::OnboardingHeaderTitle: return "Set up Phantom Mirror";
	case TextId::OnboardingHeaderSubtitle: return "Set up Spout2 or NDI for your overlay";
	case TextId::OnboardingDontShow: return "Do not show automatically on startup again";
	case TextId::OnboardingClose: return "Close";
	case TextId::OnboardingOpenSettings: return "Open Settings";
	case TextId::TraySettings: return "Settings";
	case TextId::TraySetupHelp: return "Setup Help";
	case TextId::TrayStartOverlay: return "Start Overlay";
	case TextId::TrayStopOverlay: return "Stop Overlay";
	case TextId::TrayReconnectSpout: return "Reconnect Input";
	case TextId::TrayCheckForUpdates: return "Check for Updates";
	case TextId::TrayQuit: return "Quit";
	case TextId::UpdateDialogTitle: return "Phantom Mirror Update";
	case TextId::UpdateNoUpdates: return "No updates available.";
	case TextId::UpdatePromptQuestion: return "Update %1 is available. Download and install now?";
	case TextId::UpdateAvailableStatus: return "Update %1 available.";
	case TextId::OnboardingSaveErrorPrefix: return "Could not save onboarding status: %1";
	case TextId::SaveErrorTitle: return "Phantom Mirror";
	case TextId::SaveErrorText: return "Could not save configuration: %1";
	case TextId::SpoutConnected: return "Connected";
	case TextId::SpoutWaiting: return "Waiting for sender...";
	case TextId::SpoutReceiverStopped: return "Receiver not started";
	case TextId::MonitorName: return "Monitor";
	case TextId::DisplayName: return "Display";
	}
	return {};
}

QString germanText(TextId id)
{
	switch (id) {
	case TextId::LanguageSectionTitle: return "Oberfläche";
	case TextId::LanguageLabel: return "Sprache";
	case TextId::LanguageOptionSystem: return "System";
	case TextId::LanguageOptionGerman: return "Deutsch";
	case TextId::LanguageOptionEnglish: return "English";
	case TextId::SettingsWindowTitle: return "Phantom Mirror Settings";
	case TextId::SettingsHeaderTitle: return "Phantom Mirror Settings";
	case TextId::SettingsHeaderSubtitle: return "Overlay & Verbindung konfigurieren";
	case TextId::SettingsVersionLabel: return "Version %1";
	case TextId::HotkeySectionTitle: return "Overlay-Hotkey";
	case TextId::HotkeyLabel: return "Overlay umschalten";
	case TextId::HotkeyPlaceholder: return "Hotkey drücken";
	case TextId::HotkeyClear: return "Leeren";
	case TextId::HotkeyUnset: return "Kein Hotkey konfiguriert";
	case TextId::HotkeyActive: return "Hotkey aktiv: %1";
	case TextId::HotkeyInactive: return "Hotkey deaktiviert";
	case TextId::HotkeyConflict: return "Hotkey konnte nicht aktiviert werden: %1";
	case TextId::HotkeySavedInactive: return "Gespeichert, aber aktuell inaktiv";
	case TextId::InputSectionTitle: return "Eingang";
	case TextId::InputModeLabel: return "Aktiver Eingang";
	case TextId::InputModeSpout: return "Spout2 (lokaler PC)";
	case TextId::InputModeNdi: return "NDI (Netzwerk)";
	case TextId::IncludeInScreenCapture: return "Overlay in Bildschirmaufnahme einbeziehen";
	case TextId::SpoutSectionTitle: return "Spout2 (Lokaler PC)";
	case TextId::SpoutEnabled: return "Lokale Spout2-Quelle verwenden";
	case TextId::MonitorLabel: return "Zielmonitor";
	case TextId::SenderLabel: return "Sender";
	case TextId::SenderRefresh: return "Aktualisieren";
	case TextId::SenderCustom: return "Custom";
	case TextId::SenderCustomNameLabel: return "Custom Name";
	case TextId::SenderNameLabel: return "Sender Name";
	case TextId::SenderPlaceholder: return "OBS Spout2 Output";
	case TextId::TestConnection: return "Verbindung testen";
	case TextId::SpoutNotTested: return "Nicht getestet";
	case TextId::NdiSectionTitle: return "NDI (Netzwerk)";
	case TextId::NdiEnabled: return "NDI-Quelle aus Netzwerk verwenden";
	case TextId::NdiSourceLabel: return "Quelle";
	case TextId::NdiSourceCustom: return "Custom";
	case TextId::NdiSourceNameLabel: return "Quellname";
	case TextId::NdiSourcePlaceholder: return "Entfernte NDI-Quelle";
	case TextId::NdiNotTested: return "Nicht getestet";
	case TextId::NdiConnected: return "Verbunden";
	case TextId::NdiWaiting: return "Wartet auf NDI-Quelle...";
	case TextId::NdiReceiverStopped: return "Receiver nicht gestartet";
	case TextId::NdiRuntimeMissing: return "NDI-Runtime ist nicht installiert";
	case TextId::NdiInstallRequired: return "Installiere zuerst die NDI Tools, bevor NDI auf diesem Computer aktiviert wird.";
	case TextId::NdiInstallAction: return "NDI installieren";
	case TextId::ViewportSectionTitle: return "Virtueller Anzeigebereich";
	case TextId::ViewportEnabled: return "Virtuellen Anzeigebereich aktivieren";
	case TextId::WidthLabel: return "Breite";
	case TextId::HeightLabel: return "Höhe";
	case TextId::PixelsSuffix: return "px";
	case TextId::AnchorPositionLabel: return "Ankerposition";
	case TextId::DebugSectionTitle: return "Debug";
	case TextId::DebugEnabled: return "Debug-Logging aktivieren";
	case TextId::SetupHelp: return "Setup-Hilfe";
	case TextId::Updates: return "Updates";
	case TextId::Cancel: return "Abbrechen";
	case TextId::Save: return "Speichern";
	case TextId::OnboardingWindowTitle: return "Phantom Mirror einrichten";
	case TextId::OnboardingHeaderTitle: return "Phantom Mirror einrichten";
	case TextId::OnboardingHeaderSubtitle: return "Spout2 oder NDI fuer dein Overlay einrichten";
	case TextId::OnboardingDontShow: return "Beim Start nicht mehr automatisch anzeigen";
	case TextId::OnboardingClose: return "Schließen";
	case TextId::OnboardingOpenSettings: return "Settings öffnen";
	case TextId::TraySettings: return "Settings";
	case TextId::TraySetupHelp: return "Setup-Hilfe";
	case TextId::TrayStartOverlay: return "Start Overlay";
	case TextId::TrayStopOverlay: return "Stop Overlay";
	case TextId::TrayReconnectSpout: return "Eingang neu verbinden";
	case TextId::TrayCheckForUpdates: return "Nach Updates suchen";
	case TextId::TrayQuit: return "Beenden";
	case TextId::UpdateDialogTitle: return "Phantom Mirror Update";
	case TextId::UpdateNoUpdates: return "Keine Updates verfügbar.";
	case TextId::UpdatePromptQuestion: return "Update %1 ist verfügbar. Jetzt herunterladen und installieren?";
	case TextId::UpdateAvailableStatus: return "Update %1 verfügbar.";
	case TextId::OnboardingSaveErrorPrefix: return "Onboarding-Status konnte nicht gespeichert werden: %1";
	case TextId::SaveErrorTitle: return "Phantom Mirror";
	case TextId::SaveErrorText: return "Konfiguration konnte nicht gespeichert werden: %1";
	case TextId::SpoutConnected: return "Verbunden";
	case TextId::SpoutWaiting: return "Wartet auf Sender...";
	case TextId::SpoutReceiverStopped: return "Receiver nicht gestartet";
	case TextId::MonitorName: return "Monitor";
	case TextId::DisplayName: return "Anzeige";
	}
	return {};
}

} // namespace

AppLanguage systemAppLanguage()
{
	return QLocale::system().language() == QLocale::German ? AppLanguage::German : AppLanguage::English;
}

AppLanguage resolveAppLanguage(const QString &setting)
{
	const QString normalized = normalizeAppLanguage(setting);
	if (normalized == "de")
		return AppLanguage::German;
	if (normalized == "en")
		return AppLanguage::English;
	return systemAppLanguage();
}

QString normalizeAppLanguage(const QString &setting)
{
	const QString normalized = setting.trimmed().toLower();
	if (normalized == "de" || normalized == "en" || normalized == "system")
		return normalized;
	return "system";
}

QString text(TextId id, AppLanguage language)
{
	return language == AppLanguage::German ? germanText(id) : englishText(id);
}

QString monitorLabel(QScreen *screen, int index, AppLanguage language)
{
	const QRect geometry = screen ? screen->geometry() : QRect();
	const QString name = screen ? screen->name().trimmed() : QString();
	const QString suffix = name.isEmpty() ? QString() : QString(" - %1").arg(name);
	return QString("%1 %2%3 (%4x%5)")
		.arg(text(language == AppLanguage::German ? TextId::MonitorName : TextId::DisplayName, language))
		.arg(index + 1)
		.arg(suffix)
		.arg(geometry.width())
		.arg(geometry.height());
}

QString onboardingHtml(AppLanguage language)
{
	if (language == AppLanguage::German) {
		return R"html(
<h3>SCHNELLSTART</h3>
<p>Phantom Mirror zeigt einen transparenten Vollbild-Overlay auf deinem gewählten Monitor an.
Du kannst als Quelle entweder <b>Spout2</b> vom selben PC oder <b>NDI</b> aus dem Netzwerk verwenden.</p>

<h3>OPTION A &mdash; SPOUT2 VOM SELBEN PC</h3>
<p>Installiere oder aktiviere in OBS ein Spout2-Output-Plugin, zum Beispiel <b>win-spout</b>.
Aktiviere dort die Ausgabe und merke dir den Sender-Namen, meistens <code>OBS Spout2 Output</code>.</p>

<h3>OPTION B &mdash; NDI AUS DEM NETZWERK</h3>
<p>Wenn dein Signal von einem anderen Rechner kommt, installiere auf diesem PC die
<b>NDI Tools / NDI Runtime</b>. Danach kann Phantom Mirror verfügbare NDI-Quellen im Netzwerk erkennen.</p>

<h3>IN DEN SETTINGS EINRICHTEN</h3>
<p>Öffne die Settings über das Tray-Icon. Wähle unter <b>Aktiver Eingang</b> entweder
<b>Spout2 (lokaler PC)</b> oder <b>NDI (Netzwerk)</b>. Aktiviere danach den passenden Bereich
und wähle den Sender bzw. die Quelle aus.</p>

<h3>VERBINDUNG PRÜFEN</h3>
<p>Klicke auf <b>Verbindung testen</b>. Wenn die Quelle läuft, zeigt Phantom Mirror den Namen,
die Auflösung und die FPS an. Wenn nichts gefunden wird, prüfe zuerst Sender-Name,
NDI-Verfügbarkeit und ob die Quelle bereits sendet.</p>

<h3>ANZEIGE ANPASSEN</h3>
<p>Unter <b>Zielmonitor</b> legst du fest, auf welchem Display das Overlay erscheint.
Mit <b>Virtueller Anzeigebereich</b> kannst du die Fläche auf eine feste Breite/Höhe begrenzen
und über die Ankerposition ausrichten.</p>

<h3>NÜTZLICH IM ALLTAG</h3>
<p>Der <b>Overlay-Hotkey</b> blendet das Overlay schnell ein oder aus. Mit
<b>Overlay in Bildschirmaufnahme einbeziehen</b> steuerst du, ob die Ausgabe in Screen-Captures sichtbar ist.
Updates kannst du jederzeit direkt in den Settings prüfen.</p>
)html";
	}

	return R"html(
<h3>QUICK START</h3>
<p>Phantom Mirror places a transparent fullscreen overlay on your selected display.
Your source can come from <b>Spout2</b> on the same PC or <b>NDI</b> over the network.</p>

<h3>OPTION A &mdash; SPOUT2 ON THE SAME PC</h3>
<p>Install or enable an OBS Spout2 output plugin such as <b>win-spout</b>.
Enable its output and note the sender name, usually <code>OBS Spout2 Output</code>.</p>

<h3>OPTION B &mdash; NDI OVER THE NETWORK</h3>
<p>If your feed comes from another computer, install the <b>NDI Tools / NDI Runtime</b>
on this machine first. Phantom Mirror can then discover available NDI sources on your network.</p>

<h3>SET IT UP IN SETTINGS</h3>
<p>Open Settings from the tray icon. Under <b>Active input</b>, choose either
<b>Spout2 (local PC)</b> or <b>NDI (network)</b>. Then enable the matching section
and select the sender or source you want to use.</p>

<h3>TEST THE CONNECTION</h3>
<p>Click <b>Test connection</b>. When the source is live, Phantom Mirror will show the source name,
resolution, and FPS. If nothing is found, check the sender name, NDI availability,
and whether the source is already transmitting.</p>

<h3>ADJUST THE DISPLAY</h3>
<p>Use <b>Target display</b> to choose which monitor receives the overlay.
With <b>Virtual Viewport</b>, you can limit the visible area to a fixed width and height
and align it with the anchor position.</p>

<h3>USEFUL EVERYDAY CONTROLS</h3>
<p>The <b>Overlay Hotkey</b> lets you quickly show or hide the overlay.
<b>Include overlay in screen capture</b> controls whether the overlay appears in screen recordings.
You can also check for updates directly from Settings at any time.</p>
)html";
}

QString formatUpdatePrompt(AppLanguage language, const QString &version)
{
	return text(TextId::UpdatePromptQuestion, language).arg(version);
}

QString formatUpdateAvailableStatus(AppLanguage language, const QString &version)
{
	return text(TextId::UpdateAvailableStatus, language).arg(version);
}

QString formatOnboardingSaveError(AppLanguage language, const QString &error)
{
	return text(TextId::OnboardingSaveErrorPrefix, language).arg(error);
}

QString formatSaveError(AppLanguage language, const QString &error)
{
	return text(TextId::SaveErrorText, language).arg(error);
}
