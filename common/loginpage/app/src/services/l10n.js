import { createSignal } from "solid-js";
import { parseQueryParams } from "@/utils/url";

const [translations, setTranslations] = createSignal({});

export function initLocale() {
  const en = window.l10n?.en ?? {};
  setTranslations({ ...en });

  const inParams = parseQueryParams();
  if (inParams.lang) applyLocale(inParams.lang);
}

export function t(key) {
  return translations()[key] || key;
}

export function applyLocale(locale) {
  const correct = locale.replaceAll("-", "_");
  let table = window.l10n?.[correct];

  if (!table) {
    const code = /^\w{2}/.exec(correct)?.[0];
    if (code) {
      for (const l in window.l10n || {}) {
        if (l.startsWith(code)) {
          table = window.l10n[l];
          break;
        }
      }
    }
  }

  if (table) setTranslations((prev) => ({ ...prev, ...table }));
}

export function changeLocale(newLocale) {
  applyLocale(newLocale);
}

export function isRTL(code) {
  const rtl = ["ar", "he", "ur"];
  return rtl.some((l) => code.startsWith(l));
}
