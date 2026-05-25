import { createEffect, createMemo, createSignal, onCleanup, batch } from "solid-js";
import { settings, setSettings } from "@/stores/settings";
import { onNativeMessage } from "@/sdk/events";
import { cmd } from "@/sdk/bridge";
import { applyLocale } from "@/services/l10n";

const NATIVE_TO_STORE = {
  "theme-white": "modern-light",
  "theme-night": "modern-dark",
  "theme-light": "light",
  "theme-classic-light": "classic-light",
  "theme-dark": "dark",
  "theme-gray": "gray",
  "theme-system": "system",
  "theme-contrast-dark": "contrast-dark",
};

const STORE_TO_NATIVE = Object.fromEntries(Object.entries(NATIVE_TO_STORE).map(([k, v]) => [v, k]));

const STORE_TO_CSS_CLASS = {
  "modern-light": "",
  "modern-dark": "theme-dark-modern",
  light: "theme-light",
  "classic-light": "theme-light",
  dark: "theme-dark",
  gray: "theme-gray",
  "contrast-dark": "theme-contrast-dark",
  system: "",
};

const FALLBACK_LANGS = [
  { code: "ar_SA", name: "العربية", enname: "Arabic (Saudi Arabia)" },
  { code: "be", name: "Беларуская", enname: "Belarusian" },
  { code: "bg", name: "Български", enname: "Bulgarian" },
  { code: "ca", name: "Català", enname: "Catalan" },
  { code: "cs", name: "Čeština", enname: "Czech" },
  { code: "da", name: "Dansk", enname: "Danish" },
  { code: "de", name: "Deutsch", enname: "German" },
  { code: "el", name: "Ελληνικά", enname: "Greek" },
  { code: "en", name: "English (US)", enname: "English (United States)" },
  { code: "en_GB", name: "English (UK)", enname: "English (United Kingdom)" },
  { code: "es", name: "Español", enname: "Spanish" },
  { code: "et", name: "Eesti", enname: "Estonian" },
  { code: "fi", name: "Suomi", enname: "Finnish" },
  { code: "fr", name: "Français", enname: "French" },
  { code: "ga", name: "Gaeilge", enname: "Irish" },
  { code: "gl", name: "Galego", enname: "Galician" },
  { code: "he", name: "עברית", enname: "Hebrew" },
  { code: "hi", name: "हिन्दी", enname: "Hindi" },
  { code: "hr", name: "Hrvatski", enname: "Croatian" },
  { code: "hu", name: "Magyar", enname: "Hungarian" },
  { code: "hy", name: "Հայերեն", enname: "Armenian" },
  { code: "id", name: "Bahasa Indonesia", enname: "Indonesian" },
  { code: "it", name: "Italiano", enname: "Italian" },
  { code: "ja", name: "日本語", enname: "Japanese" },
  { code: "ko", name: "한국어", enname: "Korean" },
  { code: "lo", name: "ລາວ", enname: "Lao" },
  { code: "lt", name: "Lietuvių", enname: "Lithuanian" },
  { code: "lv", name: "Latviešu", enname: "Latvian" },
  { code: "nl", name: "Nederlands", enname: "Dutch" },
  { code: "no", name: "Norsk", enname: "Norwegian" },
  { code: "pl", name: "Polski", enname: "Polish" },
  { code: "pt_BR", name: "Português (Brasil)", enname: "Portuguese (Brazil)" },
  { code: "pt_PT", name: "Português (Portugal)", enname: "Portuguese (Portugal)" },
  { code: "ro", name: "Română", enname: "Romanian" },
  { code: "ru", name: "Русский", enname: "Russian" },
  { code: "si", name: "සිංහල", enname: "Sinhala" },
  { code: "sk", name: "Slovenčina", enname: "Slovak" },
  { code: "sl", name: "Slovenščina", enname: "Slovenian" },
  { code: "sq", name: "Shqip", enname: "Albanian" },
  { code: "sr_Cyrl_RS", name: "Српски (ћирилица)", enname: "Serbian (Cyrillic)" },
  { code: "sr_Latn_RS", name: "Srpski (latinica)", enname: "Serbian (Latin)" },
  { code: "sv", name: "Svenska", enname: "Swedish" },
  { code: "tr", name: "Türkçe", enname: "Turkish" },
  { code: "uk", name: "Українська", enname: "Ukrainian" },
  { code: "ur", name: "اردو", enname: "Urdu" },
  { code: "vi", name: "Tiếng Việt", enname: "Vietnamese" },
  { code: "zh_CN", name: "中文(简体)", enname: "Chinese (Simplified)" },
  { code: "zh_TW", name: "中文(繁體)", enname: "Chinese (Traditional)" },
];

function applyThemeToDom(storeThemeId) {
  const cssClass = STORE_TO_CSS_CLASS[storeThemeId] ?? "";
  const el = document.documentElement;
  el.className = el.className.replace(/theme-[\w-]+/gi, "").trim();
  if (cssClass) el.classList.add(cssClass);
}

function applyScaling(scaling) {
  const html = document.documentElement;
  html.className = html.className.replace(/\bscale-\d+\b/g, "").trim();

  if (scaling === "auto") {
    document.body.style.zoom = "";
    return;
  }

  const value = parseInt(scaling, 10);
  if (isNaN(value)) return;

  if (value !== 100) html.classList.add(`scale-${value}`);
  document.body.style.zoom = value / 100;
}

function nativeScalingToStore(raw) {
  return raw == null || raw === 0 ? "auto" : String(raw);
}

function htmlDecode(str) {
  const ta = document.createElement("textarea");
  ta.innerHTML = str;
  return ta.value;
}

const [availableLangs, setAvailableLangs] = createSignal(FALLBACK_LANGS);
export { availableLangs };

let _needsRestart = () => false;
let _toggleAI = () => {};

export const needsRestart = () => _needsRestart();
export const toggleAI = () => _toggleAI();

export function initSettingsSync() {
  const [nativeSnapshot, setNativeSnapshot] = createSignal(null);
  let initialized = false;

  _needsRestart = createMemo(() => {
    const snap = nativeSnapshot();
    if (!snap) return false;
    return (
      settings.lang !== (snap.locale?.current ?? settings.lang) ||
      settings.scaling !== nativeScalingToStore(snap.uiscaling) ||
      settings.gpuMode !== !!snap.usegpu
    );
  });

  _toggleAI = () => {
    const newValue = !settings.useai;
    setSettings("useai", newValue);
    cmd("settings:apply", JSON.stringify({ useai: newValue }));
  };

  createEffect(() => applyThemeToDom(settings.theme));
  createEffect(() => applyLocale(settings.lang));
  createEffect(() => applyScaling(settings.scaling));

  createEffect(() => {
    const lang = settings.lang;
    const scaling = settings.scaling;
    const theme = settings.theme;
    const updates = settings.checkUpdates;
    const launch = settings.launchMode;
    const spell = settings.spellCheck;
    const gpu = settings.gpuMode;

    if (!initialized) return;

    const snap = nativeSnapshot();
    const restart =
      snap !== null &&
      (lang !== (snap.locale?.current ?? lang) || scaling !== nativeScalingToStore(snap.uiscaling) || gpu !== !!snap.usegpu);

    const payload = {
      lang,
      uitheme: STORE_TO_NATIVE[theme] ?? theme,
      autoupdatemode: updates,
      docopenmode: launch,
      spellcheckdetect: spell,
      usegpu: gpu,
      restart,
    };

    payload.uiscaling = scaling === "auto" ? "0" : scaling;

    cmd("settings:apply", JSON.stringify(payload));
  });

  const unsub = onNativeMessage((msgCmd, param) => {
    if (msgCmd === "settings:init") {
      let payload;
      try {
        payload = JSON.parse(htmlDecode(param));
      } catch {
        return;
      }

      if (payload.locale?.langs) {
        const langs = Object.entries(payload.locale.langs).map(([code, info]) => ({
          code,
          name: info.name,
          enname: info.enname,
        }));
        if (langs.length > 0) setAvailableLangs(langs);
      }

      batch(() => {
        if (payload.locale?.current) setSettings("lang", payload.locale.current);
        if ("uiscaling" in payload) setSettings("scaling", nativeScalingToStore(payload.uiscaling));
        if (payload.uitheme) setSettings("theme", NATIVE_TO_STORE[payload.uitheme] ?? payload.uitheme);
        if (payload.autoupdatemode) setSettings("checkUpdates", payload.autoupdatemode);
        if (payload.docopenmode) setSettings("launchMode", payload.docopenmode);
        if (payload.spellcheckdetect) setSettings("spellCheck", payload.spellcheckdetect);
        if (payload.usegpu != null) setSettings("gpuMode", !!payload.usegpu);
        if (payload.useai != null) setSettings("useai", !!payload.useai);
      });
      initialized = true;
      setNativeSnapshot(payload);
    } else if (msgCmd === "uitheme:changed") {
      setSettings("theme", NATIVE_TO_STORE[param] ?? param);
    }
  });

  onCleanup(unsub);
}
