import { t } from "@/services/l10n.js";
import { openLocal, dropOfficeFiles } from "@/sdk";

export function FileDropZone() {
  function handleDrop(e) {
    dropOfficeFiles();
    e.preventDefault();
    return false;
  }

  function handleDragOver(e) {
    e.dataTransfer.dropEffect = "copy";
    e.preventDefault();
    return false;
  }

  return (
    <div class="file-drop-zone" onDrop={handleDrop} onDragOver={handleDragOver}>
      <div class="content">
        <div>
          <h3>{t("dropZoneTitle")}</h3>
          <p class="text-normal">{t("dropZoneText")}</p>
        </div>
        <button class="btn btn-accent" onClick={() => openLocal()}>
          {t("actOpenLocal")}
        </button>
      </div>
    </div>
  );
}
