import { createSignal, createEffect } from "solid-js";
import { makePersisted } from "@solid-primitives/storage";
import { onNativeMessage } from "@/sdk";
import { parseQueryParams } from "@/utils/url";

function resolveTheme(raw) {
  let name = raw;
  let type = "";
  if (raw && /^{".+"}+$/.test(raw)) {
    try {
      const obj = JSON.parse(raw);
      name = obj.id || "theme-dark";
      type = obj.type || "";
    } catch {}
  }
  type = type === "dark" || /theme-(?:[a-z]+-)?dark(?:-[a-z]*)?/.test(name || "") ? "theme-type-dark" : "theme-type-light";
  return { name, type };
}

export function createTheme() {
  const inParams = parseQueryParams();
  const initialRaw = inParams.uitheme || localStorage.getItem("ui-theme");
  const initial = resolveTheme(initialRaw);

  const [name, setName] = makePersisted(createSignal(initial.name || "theme-light"), {
    name: "ui-theme-name",
  });
  const [type, setType] = createSignal(initial.type || "theme-type-light");

  function applyTheme(themeName, themeType) {
    document.body.className = document.body.className
      .replace(/theme-[a-z-]*/gi, "")
      .replace(/theme-type-\w+/gi, "")
      .trim();
    if (themeName) document.body.classList.add(themeName);
    if (themeType) document.body.classList.add(themeType);
  }

  applyTheme(name(), type());

  createEffect(() => {
    applyTheme(name(), type());
  });

  const dispose = onNativeMessage((cmd, param) => {
    if (cmd === "ui:theme") {
      const resolved = resolveTheme(param);
      setName(resolved.name);
      setType(resolved.type);
    }
  });

  return { name, setName, type, setType, dispose };
}
