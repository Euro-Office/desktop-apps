const sdk = window.AscDesktopEditor;

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

const subscribers = { any: [] };

function notifySubscribers(action, type, arg, context) {
  const pubtype = type || "any";
  const pubsubscribers = subscribers[pubtype];
  const max = pubsubscribers ? pubsubscribers.length : 0;

  for (let i = 0; i < max; i += 1) {
    if (action === "publish") {
      pubsubscribers[i].fn.apply(pubsubscribers[i].context, arg);
    } else {
      if (pubsubscribers[i].fn === arg && pubsubscribers[i].context === context) {
        pubsubscribers.splice(i, 1);
      }
    }
  }
}

sdk.on = function (type, fn, context) {
  type = type || "any";
  fn = typeof fn === "function" ? fn : context[fn];
  if (typeof subscribers[type] === "undefined") {
    subscribers[type] = [];
  }
  subscribers[type].push({ fn: fn, context: context || this });
};

sdk.remove = function (type, fn, context) {
  notifySubscribers("unsubscribe", type, fn, context);
};

sdk.fire = function (type, publication) {
  notifySubscribers("publish", type, publication);
};

sdk.command = function (...args) {
  window.AscDesktopEditor.execCommand.apply(window.AscDesktopEditor, args);
};

for (const e of _events) {
  window[e] = function (...args) {
    notifySubscribers("publish", e, args);
  };
}

export function cmd(name, param) {
  sdk.execCommand(name, typeof param === "string" ? param : JSON.stringify(param));
}
