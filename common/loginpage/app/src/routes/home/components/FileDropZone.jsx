import { t } from "@/services/l10n.js";
import { openLocal } from "@/sdk";
import "./FileDropZone.styles.css";
import { Button } from "@/components";

export function FileDropZone() {
  return (
    <div class="file-drop-zone">
      <div class="content">
        <div>
          <h3>{t("dropZoneTitle")}</h3>
          <p class="text-normal">{t("dropZoneText")}</p>
        </div>
        <Button variant="accent" onClick={openLocal}>
          {t("dropZoneOpenAction")}
        </Button>
      </div>
    </div>
  );
}
