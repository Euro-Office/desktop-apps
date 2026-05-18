const sdk = window.AscDesktopEditor;

const subscribers = {};

sdk.on = function (type, fn) {
  if (!subscribers[type]) subscribers[type] = [];
  subscribers[type].push(fn);
};

sdk.remove = function (type, fn) {
  if (subscribers[type]) {
    subscribers[type] = subscribers[type].filter((f) => f !== fn);
  }
};

sdk.fire = function (type, ...args) {
  (subscribers[type] || []).forEach((fn) => fn(...args));
};

const _events = [
  "onchildframemessage",
  "onupdaterecents",
  "onupdaterecovers",
  "on_native_message",
  "on_check_auth",
  "onChangeCryptoMode",
  "onfeaturesavailable",
  "onaddtemplates",
];

for (const e of _events) {
  window[e] = function (...args) {
    sdk.fire(e, ...args);
  };
}

export function cmd(name, param) {
  sdk.execCommand(name, typeof param === "string" ? param : JSON.stringify(param));
}
