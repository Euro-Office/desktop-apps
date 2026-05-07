import { cmd } from "./bridge";

const sdk = window.AscDesktopEditor;

export function createDocument(kind) {
  cmd("create:new", kind);
}

export function openLocal() {
  cmd("open:folder", "");
}

export function openRecentFile(model) {
  cmd("open:recent", {
    id: model.fileid,
    name: model.name,
    path: model.path,
    type: model.type,
    cloud: model.cloud,
    hash: model.hash,
    recovery: false,
  });
}

export function openRecoveryFile(model) {
  cmd("open:recent", {
    id: model.fileid,
    name: model.name,
    path: model.path,
    type: model.type,
    cloud: model.cloud,
    hash: model.hash,
    recovery: true,
  });
}

export function exploreFile(model) {
  cmd("files:explore", {
    path: model.path,
    id: model.fileid,
    hash: model.hash,
  });
}

export function checkFiles(hashMap) {
  cmd("files:check", hashMap);
}

export function dropOfficeFiles() {
  sdk.DropOfficeFiles();
}
