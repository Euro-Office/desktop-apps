const rtf = new Intl.RelativeTimeFormat(undefined, { numeric: "auto" });
const dtf = new Intl.DateTimeFormat(undefined, {
  year: "numeric",
  month: "short",
  day: "numeric",
});
const dtfTime = new Intl.DateTimeFormat(undefined, {
  hour: "2-digit",
  minute: "2-digit",
});

const SDK_DATE_RE = /^(\d{2})\.(\d{2})\.(\d{4})\s+(\d{2}):(\d{2})(?::(\d{2}))?$/;

function parseSdkDate(s) {
  const m = s && SDK_DATE_RE.exec(s);
  if (!m) return null;
  const d = new Date(+m[3], +m[2] - 1, +m[1], +m[4], +m[5], +m[6] || 0);
  return isNaN(d) ? null : d;
}

const startOfDay = (d) => new Date(d.getFullYear(), d.getMonth(), d.getDate()).getTime();

export function formatDate(input) {
  const date = input instanceof Date ? input : parseSdkDate(input);
  if (!date) return "";

  const now = new Date();
  const diffSec = Math.round((date - now) / 1000);
  const diffMin = Math.round(diffSec / 60);
  const diffHour = Math.round(diffMin / 60);
  const diffDay = Math.round((startOfDay(date) - startOfDay(now)) / 86_400_000);

  if (Math.abs(diffSec) < 60) return rtf.format(diffSec, "second");
  if (Math.abs(diffMin) < 60) return rtf.format(diffMin, "minute");
  if (diffDay === 0) return rtf.format(diffHour, "hour");
  if (Math.abs(diffDay) < 7) return rtf.format(diffDay, "day");

  return date.getFullYear() === now.getFullYear() ? `${dtf.format(date)} ${dtfTime.format(date)}` : dtf.format(date);
}
