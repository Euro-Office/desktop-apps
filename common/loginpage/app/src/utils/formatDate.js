const rtf = new Intl.RelativeTimeFormat(undefined, { numeric: "auto" });
const dtf = new Intl.DateTimeFormat(undefined, {
  year: "numeric",
  month: "short",
  day: "numeric"
});
const dtfTime = new Intl.DateTimeFormat(undefined, {
  hour: "2-digit",
  minute: "2-digit"
});

function parseSdkDate(s) {
  if (!s) return null;
  const m = /^(\d{2})\.(\d{2})\.(\d{4})\s+(\d{2}):(\d{2})(?::(\d{2}))?$/.exec(s);
  if (!m) return null;
  const [, dd, mm, yyyy, hh, mi, ss] = m;
  const d = new Date(+yyyy, +mm - 1, +dd, +hh, +mi, +ss || 0);
  return isNaN(d) ? null : d;
}

export function formatDate(input) {
  if (!input) return "";
  const date = input instanceof Date ? input : parseSdkDate(input);
  if (!date) return "";

  const now = new Date();
  const diffMs = date.getTime() - now.getTime();
  const diffSec = Math.round(diffMs / 1000);
  const diffMin = Math.round(diffSec / 60);
  const diffHour = Math.round(diffMin / 60);

  // Calendar-day difference (ignores time of day)
  const startOfDay = (d) => new Date(d.getFullYear(), d.getMonth(), d.getDate()).getTime();
  const diffDay = Math.round((startOfDay(date) - startOfDay(now)) / 86400000);

  if (Math.abs(diffSec) < 60) return rtf.format(diffSec, "second");
  if (Math.abs(diffMin) < 60) return rtf.format(diffMin, "minute");
  if (diffDay === 0) return rtf.format(diffHour, "hour");      // today
  if (diffDay === -1 || diffDay === 1) return rtf.format(diffDay, "day"); // "yesterday"/"tomorrow"
  if (Math.abs(diffDay) < 7) return rtf.format(diffDay, "day");

  // Older: show date (+ time if same year)
  if (date.getFullYear() === now.getFullYear()) {
    return `${dtf.format(date)} ${dtfTime.format(date)}`;
  }
  return dtf.format(date);
}
