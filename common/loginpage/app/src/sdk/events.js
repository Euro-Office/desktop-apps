const sdk = window.AscDesktopEditor;

export function onNativeMessage(cb) {
  const wrapped = (cmd, param) => cb(cmd, param);
  sdk.on("on_native_message", wrapped);
  return () => sdk.remove("on_native_message", wrapped);
}

export function onRecentsChanged(cb) {
  const wrapped = (params) => cb(params);
  sdk.on("onupdaterecents", wrapped);
  return () => sdk.remove("onupdaterecents", wrapped);
}

export function onRecoversChanged(cb) {
  const wrapped = (params) => cb(params);
  sdk.on("onupdaterecovers", wrapped);
  return () => sdk.remove("onupdaterecovers", wrapped);
}
