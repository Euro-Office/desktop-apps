const sdk = window.AscDesktopEditor;

export function onNativeMessage(cb) {
  sdk.on("on_native_message", cb);
  return () => sdk.remove("on_native_message", cb);
}

export function onRecentsChanged(cb) {
  sdk.on("onupdaterecents", cb);
  return () => sdk.remove("onupdaterecents", cb);
}

export function onRecoversChanged(cb) {
  sdk.on("onupdaterecovers", cb);
  return () => sdk.remove("onupdaterecovers", cb);
}
