import { render } from "solid-js/web";
import App from "./app.jsx";
import { initLocale } from "./services/l10n";
import { onNativeMessage, dropOfficeFiles } from "./sdk";

initLocale();

const noconnectCss = document.createElement("link");
noconnectCss.rel = "stylesheet";
noconnectCss.href = "../../noconnect/styles.css";
document.head.appendChild(noconnectCss);

const dispose = render(() => <App />, document.getElementById("root"));

document.getElementById("wrap").ondrop = function (e) {
  dropOfficeFiles();
  e.preventDefault();
  return false;
};

document.getElementById("wrap").ondragover = function (e) {
  e.dataTransfer.dropEffect = "copy";
  e.preventDefault();
  return false;
};

function hideLoader() {
  const mask = document.getElementById("loading-mask");
  if (mask) mask.style.display = "none";
}

onNativeMessage((cmd) => {
  if (cmd === "app:ready") {
    hideLoader();
  }
});

setTimeout(() => {
  window.AscDesktopEditor.LocalFileRecents();
  window.AscDesktopEditor.LocalFileRecovers();
  window.AscDesktopEditor.execCommand("app:onready", "");
}, 50);

if (import.meta.hot) {
  import.meta.hot.accept();
  import.meta.hot.dispose(() => dispose());
}
