import { t } from "@/services/l10n.js";
import { openLocal, dropOfficeFiles } from "@/sdk";

export default function DnDZone() {
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
    <div class="dnd-zone" onDrop={handleDrop} onDragOver={handleDragOver}>
      <div class="content">
        <p class="text-normal">{t("labelDropFile")}</p>
        <button class="btn btn--primary" onClick={() => openLocal()}>
          {t("labelSelectFile")}
        </button>
      </div>
    </div>
  );
}
