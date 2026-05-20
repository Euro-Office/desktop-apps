import { render } from "solid-js/web";
import App from "./app.jsx";
import { initLocale } from "./services/l10n";
import { onNativeMessage, dropOfficeFiles } from "./sdk";

initLocale();

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

onNativeMessage((cmd) => {
  if (cmd === "app:ready") {
    const mask = document.getElementById("loading-mask");
    if (mask) mask.style.display = "none";
  }
});

if (import.meta.hot) {
  import.meta.hot.accept();
  import.meta.hot.dispose(() => dispose());
}
