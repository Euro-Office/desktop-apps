import { cmd } from "./bridge";
import { parseFileFormat } from "./format";

const sdk = window.AscDesktopEditor;

export function listRecents() {
  return new Promise((resolve) => {
    const handler = (params) => {
      sdk.remove("onupdaterecents", handler);
      resolve(Array.isArray(params) ? params : []);
    };
    sdk.on("onupdaterecents", handler);
    sdk.LocalFileRecents();
  });
}

export function listRecovers() {
  return new Promise((resolve) => {
    const handler = (params) => {
      sdk.remove("onupdaterecovers", handler);
      resolve(Array.isArray(params) ? params : []);
    };
    sdk.on("onupdaterecovers", handler);
    sdk.LocalFileRecovers();
  });
}

export function pinRecent(id, pinned) {
  cmd("recent:pinned", { id, pinned });
}

export function removeRecent(id) {
  sdk.LocalFileRemoveRecent(parseInt(id));
}

export function removeRecover(id) {
  sdk.LocalFileRemoveRecover(parseInt(id));
}

export function clearRecents() {
  sdk.LocalFileRemoveAllRecents();
}

export function clearRecovers() {
  sdk.LocalFileRemoveAllRecovers();
}

const isWin = /Win/.test(navigator.platform);
const reBaseName = isWin ? /([^\\/]+\.[a-zA-Z0-9]{1,})$/ : /([^/]+\.[a-zA-Z0-9]{1,})$/;
const reFolderName = isWin ? /([^\\/]+)$/ : /([^/]+)$/;

const decoder = document.createElement("div");
function decodeHtml(s) {
  decoder.innerHTML = s;
  return decoder.textContent;
}

function hashCode(s) {
  let h = 0;
  for (let i = 0; i < s.length; i++) {
    h = ((h << 5) - h + s.charCodeAt(i)) | 0;
  }
  return h;
}

export function pinnedFolders(action, path) {
  const key = "pinnedFolders";
  const list = JSON.parse(localStorage.getItem(key) || "[]");
  if (action === "list") return list;
  if (action === "check") return list.includes(path);
  const i = list.indexOf(path);
  if (action === "toggle") {
    i === -1 ? list.push(path) : list.splice(i, 1);
  }
  localStorage.setItem(key, JSON.stringify(list));
  return list;
}

export function parseRecent(arr) {
  const files = [];
  const dirs = [];
  for (const f of arr || []) {
    let p = f.path || "";
    if (isWin && /^\w:[\\/]/.test(p) && /\\{2,}/.test(p)) {
      p = p.replace(/\\{2,}/g, "\\");
    }
    const m = reBaseName.exec(p);
    if (!m) continue;
    const fullName = m[1];
    const dot = fullName.lastIndexOf(".");
    const name = dot >= 0 ? fullName.slice(0, dot) : fullName;
    const ext = dot >= 0 ? fullName.slice(dot) : "";
    const dir = p.slice(0, p.length - fullName.length - 1);
    const id = f.id;
    files.push({
      uid: `file-${id}`,
      fileid: id,
      pinid: f.pin ? -id : id,
      type: f.type,
      format: parseFileFormat(f.type),
      name,
      ext,
      fullName,
      descr: dir,
      path: decodeHtml(p),
      date: f.modifyed || "",
      cloud: f.cloud || "",
      pinned: !!f.pin,
      crypted: !!f.crypted,
      exist: f.exist !== false,
      hash: f.hash || 0,
    });
    if (!dirs.includes(dir)) dirs.push(dir);
  }
  return files;
}

export function parseRecentDirs(arr, t) {
  const seen = new Set();
  const dirs = [];
  for (const f of arr || []) {
    let p = f.path || "";
    if (isWin && /^\w:[\\/]/.test(p) && /\\{2,}/.test(p)) {
      p = p.replace(/\\{2,}/g, "\\");
    }
    const m = reBaseName.exec(p);
    if (!m) continue;
    const dir = p.slice(0, p.length - m[1].length - 1);
    if (!seen.has(dir)) {
      seen.add(dir);
      dirs.push(dir);
    }
  }
  for (const pinned of pinnedFolders("list")) {
    if (!seen.has(pinned)) {
      seen.add(pinned);
      dirs.push(pinned);
    }
  }
  const out = [];
  for (const full of dirs) {
    const m = reFolderName.exec(full);
    let name = m ? m[1] : full;
    let parent = m ? full.slice(0, full.length - name.length - 1) : t ? t("textMyComputer") : "My Computer";
    const id = hashCode(full);
    const isPinned = pinnedFolders("check", full);
    out.push({
      uid: `folder-${id}`,
      type: "folder",
      full,
      name,
      descr: parent,
      pinid: isPinned ? -id : id,
      pinned: isPinned,
    });
  }
  return out;
}
