import { t } from "@/services/l10n.js";
import { openLocal, dropOfficeFiles } from "@/sdk";
import "./FileDropZone.styles.css";

export function FileDropZone() {
  // todo: mb delete?
  function handleDrop(e) {
    e.stopPropagation();
    dropOfficeFiles();
    e.preventDefault();
    return false;
  }

  // todo: mb delete?
  function handleDragOver(e) {
    e.stopPropagation();
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
          {t("dropZoneOpenAction")}
        </button>
      </div>
    </div>
  );
}
