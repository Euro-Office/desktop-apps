import { createStore } from "solid-js/store";
import { makePersisted } from "@solid-primitives/storage";

const [settings, setSettings] = makePersisted(
  createStore({
    userName: "",
    openMode: false,
    lang: "en",
    scaling: "auto",
    checkUpdates: "day",
    launchMode: "tab",
    spellCheck: true,
    gpuMode: true,
  }),
  { name: "settings" },
);

export { settings, setSettings };
