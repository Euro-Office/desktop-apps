import { createStore } from "solid-js/store";
import { makePersisted } from "@solid-primitives/storage";

const defaultSettings = {
  userName: "",
  country: "GB",
  lang: "en",
  scaling: "auto",
  checkUpdates: "ask",
  theme: "modern-light",
  launchMode: "tab",
  spellCheck: "auto",
  gpuMode: true,
  useai: true,
};

const [settings, setSettings] = makePersisted(
  createStore(defaultSettings),
  { name: "settings" },
);

export { settings, setSettings, defaultSettings };
