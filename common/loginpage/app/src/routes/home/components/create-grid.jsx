import { createDocument } from "@/sdk";
import { For } from "solid-js";

const DOC_TYPES = [
  {
    id: "word",
    titleKey: "newDoc",
    formatLabel: { value: "DOCX", gradientColorStart: "#4298C5", gradientColorEnd: "#2D84B2", bgColorWinXP: "#287ca9" },
    icon: "#docx-big",
  },
  {
    id: "cell",
    titleKey: "newXlsx",
    formatLabel: { value: "XLSX", gradientColorStart: "#5BB514", gradientColorEnd: "#318C2B", bgColorWinXP: "#3aa133" },
    icon: "#xlsx-big",
  },
  {
    id: "slide",
    titleKey: "newPptx",
    formatLabel: { value: "PPTX", gradientColorStart: "#F4893A", gradientColorEnd: "#DE7341", bgColorWinXP: "#f36700" },
    icon: "#pptx-big",
  },
  {
    id: "form",
    titleKey: "newForm",
    formatLabel: { value: "PDF", gradientColorStart: "#F36653", gradientColorEnd: "#D2402D", bgColorWinXP: "#e54d39" },
    icon: "#pdf-big",
  },
];

export default function CreateGrid(props) {
  const onSelect = props.onDocumentSelect || ((id) => createDocument(id));

  return (
    <div class="document-creation-grid">
      <For each={DOC_TYPES}>
        {(item) => (
          <div class="document-creation-item" onClick={() => onSelect(item.id)}>
            <div
              class="format-label"
              style={{
                "--format-bg-start": item.formatLabel.gradientColorStart,
                "--format-bg-end": item.formatLabel.gradientColorEnd,
                "--format-bg-winxp": item.formatLabel.bgColorWinXP,
              }}
            >
              <span>{item.formatLabel.value}</span>
            </div>
            <svg class="icon">
              <use href={item.icon} />
            </svg>
            <div class="title">{props.t(item.titleKey)}</div>
          </div>
        )}
      </For>
    </div>
  );
}
