const lang = {};

export function initLocale() {
  const en = window.l10n && window.l10n.en ? window.l10n.en : {};
  Object.assign(lang, en);

  const inParams = parseQueryParams();
  if (inParams.lang) {
    applyLocale(inParams.lang);
  }
}

export function t(key) {
  return lang[key] || key;
}

export function applyLocale(locale) {
  const correct = locale.replaceAll("-", "_");
  let table = window.l10n && window.l10n[correct];

  if (!table) {
    const code = /^\w{2}/.exec(correct)[0];
    for (const l in window.l10n || {}) {
      if (l.startsWith(code)) {
        table = window.l10n[l];
        break;
      }
    }
  }

  if (table) {
    Object.assign(lang, table);
  }
}

export function changeLocale(newLocale) {
  applyLocale(newLocale);
}

export function isRTL(code) {
  const rtl = ["ar", "he", "ur"];
  return rtl.some((l) => code.startsWith(l));
}

function parseQueryParams() {
  const q = window.location.search.substring(1);
  const params = {};
  q.split("&").forEach((pair) => {
    const [k, v] = pair.split("=");
    if (k) params[decodeURIComponent(k)] = decodeURIComponent((v || "").replace(/\+/g, " "));
  });
  return params;
}
