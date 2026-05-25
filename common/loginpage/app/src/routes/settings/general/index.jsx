import { createMemo, batch } from "solid-js";
import { t } from "@/services/l10n";
import { Select, Switch, TextField, Button } from "@/components";
import { settings, setSettings, defaultSettings } from "@/stores/settings";
import { availableLangs, toggleAI } from "@/services/settings-sync";
import styles from "./index.module.css";

export default function GeneralSettings() {
  const countryOptions = createMemo(() => [
    { value: "GB", label: "Great Britain" },
    { value: "US", label: "United States" },
  ]);

  const languageOptions = createMemo(() => availableLangs().map((l) => ({ value: l.code, label: l.name })));

  const scalingOptions = createMemo(() => [
    { value: "auto", label: t("settOptScalingAuto") },
    { value: "100", label: "100%" },
    { value: "125", label: "125%" },
    { value: "150", label: "150%" },
    { value: "175", label: "175%" },
    { value: "200", label: "200%" },
    { value: "225", label: "225%" },
    { value: "250", label: "250%" },
    { value: "275", label: "275%" },
    { value: "300", label: "300%" },
    { value: "350", label: "350%" },
    { value: "400", label: "400%" },
    { value: "450", label: "450%" },
    { value: "500", label: "500%" },
  ]);

  const updatesOptions = createMemo(() => [
    { value: "ask", label: t("settOptAUpdateAsk") },
    { value: "silent", label: t("settOptAUpdateSilent") },
    { value: "disabled", label: t("settOptAUpdateDisabled") },
  ]);

  const themeOptions = createMemo(() => [
    { value: "modern-light", label: t("settOptThemeWhite") },
    { value: "modern-dark", label: t("settOptThemeNight") },
    { value: "light", label: t("settOptThemeLight") },
    { value: "classic-light", label: t("settOptThemeClassicLight") },
    { value: "dark", label: t("settOptThemeDark") },
    { value: "gray", label: t("settOptThemeGray") },
    { value: "system", label: t("settOptThemeSystem") },
    { value: "contrast-dark", label: t("settOptThemeContrastDark") },
  ]);

  const openFileOptions = createMemo(() => [
    { value: "tab", label: t("settOptLaunchInTab") },
    { value: "window", label: t("settOptLaunchInWindow") },
  ]);

  const spellOptions = createMemo(() => [
    { value: "auto", label: t("settOptScalingAuto") },
    { value: "enabled", label: t("settOptEnabled") },
    { value: "disabled", label: t("settOptDisabled") },
  ]);

  function handleReset() {
    batch(() => {
      for (const [k, v] of Object.entries(defaultSettings)) {
        setSettings(k, v);
      }
    });
  }

  return (
    <div class={styles.page}>
      <div class={styles.title}>
        <h1>{t("settingsGeneral")}</h1>
      </div>
      <div class={styles.form}>
        <div class={styles.formHeader}>
          <button class={styles.resetLink} onClick={handleReset}>
            {t("settResetUserName")}
          </button>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settUserName")}</span>
          <div class={styles.fieldControl}>
            <TextField class={styles.fullWidth} value={settings.userName} onChange={(v) => setSettings("userName", v)}>
              <TextField.Input placeholder={t("settingsUserNamePlaceholder")} />
            </TextField>
          </div>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settingsCountry")}</span>
          <div class={styles.fieldControl}>
            <Select options={countryOptions()} value={settings.country} onChange={(v) => setSettings("country", v)} disallowEmptySelection>
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settLanguage")} *</span>
          <div class={styles.fieldControl}>
            <Select options={languageOptions()} value={settings.lang} onChange={(v) => setSettings("lang", v)} disallowEmptySelection>
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settScaling")} *</span>
          <div class={styles.fieldControl}>
            <Select options={scalingOptions()} value={settings.scaling} onChange={(v) => setSettings("scaling", v)} disallowEmptySelection>
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settAUpdateMode")}</span>
          <div class={styles.fieldControl}>
            <Select
              options={updatesOptions()}
              value={settings.checkUpdates}
              onChange={(v) => setSettings("checkUpdates", v)}
              disallowEmptySelection
            >
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settUITheme")}</span>
          <div class={styles.fieldControl}>
            <Select options={themeOptions()} value={settings.theme} onChange={(v) => setSettings("theme", v)} disallowEmptySelection>
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settOptLaunchMode")}</span>
          <div class={styles.fieldControl}>
            <Select
              options={openFileOptions()}
              value={settings.launchMode}
              onChange={(v) => setSettings("launchMode", v)}
              disallowEmptySelection
            >
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settSpellcheckDetection")}</span>
          <div class={styles.fieldControl}>
            <Select
              options={spellOptions()}
              value={settings.spellCheck}
              onChange={(v) => setSettings("spellCheck", v)}
              disallowEmptySelection
            >
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.divider} />

        <div class={styles.fieldRowAuto}>
          <label for="gpu-mode" class={styles.fieldLabelBlock}>
            {t("settGpuUseMode")} *<span class={styles.fieldDescription}>{t("settingsGpuUseModeDescr")}</span>
          </label>
          <Switch id="gpu-mode" checked={settings.gpuMode} onChange={(v) => setSettings("gpuMode", v)} />
        </div>

        <div class={styles.divider} />

        <div class={styles.fieldRowAuto}>
          <div class={styles.fieldLabelBlock}>
            <span class={styles.fieldLabel}>{t(settings.useai ? "settingsDisableAI" : "settingsEnableAI")}</span>
            <span class={styles.fieldDescription}>{t(settings.useai ? "settingsDisableAIDescr" : "settingsEnableAIDescr")}</span>
          </div>
          <div class={styles.fieldControlEnd}>
            <Button variant="accent" onClick={toggleAI}>
              {t(settings.useai ? "settingsDisableAIButton" : "settingsEnableAIButton")}
            </Button>
          </div>
        </div>
      </div>
    </div>
  );
}
