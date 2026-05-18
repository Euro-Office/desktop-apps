import { For, Show } from "solid-js";
import { openRecentFile, exploreFile } from "@/sdk";
import { clsx } from "clsx";
import { t } from "@/services/l10n";
import { ContextMenu } from "@kobalte/core/context-menu";
import { DropdownMenu } from "@kobalte/core/dropdown-menu";
import "./RecentList.styles.css";

function buildItems(model, onTogglePin, onRemove, onClear) {
  const items = [];

  items.push({
    key: "open",
    label: t("menuFileOpen"),
    onSelect: () => openRecentFile(model),
  });

  items.push({
    key: model.pinned ? "unpin" : "pin",
    label: t(model.pinned ? "menuFileUnpin" : "menuFilePin"),
    onSelect: () => onTogglePin(model.fileid),
  });

  const showExplore = (model.cloud || model.descr) && model.exist;
  if (showExplore) {
    items.push({
      key: "explore",
      label: t("menuFileExplore"),
      onSelect: () => exploreFile(model),
    });
  }

  items.push({
    key: "remove",
    label: t("menuRemoveModel"),
    onSelect: () => onRemove(model.fileid),
  });

  items.push({ key: "sep", separator: true });

  items.push({
    key: "clear",
    label: t("menuClear"),
    onSelect: () => onClear(),
  });

  return items;
}

function FileMenuContent(props) {
  const items = () => buildItems(props.model, props.onTogglePin, props.onRemove, props.onClear);

  return (
    <For each={items()}>
      {(item) => (
        <Show when={!item.separator} fallback={<ContextMenu.Separator class="context-menu-separator" />}>
          <ContextMenu.Item class={`context-menu-item${item.key === "clear" ? " negative" : ""}`} onSelect={item.onSelect}>
            {item.label}
          </ContextMenu.Item>
        </Show>
      )}
    </For>
  );
}

function RecentRow(props) {
  let suppressRowClick = false;

  return (
    <ContextMenu>
      <ContextMenu.Trigger
        as="div"
        class={clsx("row text-normal", props.model.pinned && "pinned", !props.model.exist && "lost")}
        onClick={() => {
          if (suppressRowClick) {
            suppressRowClick = false;
            return;
          }
          openRecentFile(props.model);
        }}
      >
        <div class="col-name" title={props.model.fullName}>
          <div class="icon">
            <svg class="icon">
              <use href={`#${props.model.type === "folder" ? "folder-small" : props.model.format}`} />
            </svg>
            <Show when={props.model.crypted}>
              <svg class="icon shield">
                <use href="#shield" />
              </svg>
            </Show>
          </div>
          <p class="name">
            {props.model.name}
            <span class="ext">{props.model.ext}</span>
          </p>
        </div>
        <div class="col-location" title={props.model.descr}>
          {props.model.descr}
        </div>
        <Show when={props.model.type !== "folder"}>
          <div class="col-date" title={props.model.date}>
            <p>{props.model.dateLabel}</p>
          </div>
        </Show>
        <div class="col-actions">
          <button
            class="pin"
            onClick={(e) => {
              e.stopPropagation();
              props.onTogglePin(props.model.fileid);
            }}
          >
            <svg class="icon">
              <use href="#pin" />
            </svg>
          </button>
          <Show when={props.model.type !== "folder"}>
            <DropdownMenu>
              <DropdownMenu.Trigger as="button" class="more" onClick={(e) => e.stopPropagation()}>
                <svg class="icon">
                  <use href="#more" />
                </svg>
              </DropdownMenu.Trigger>
              <DropdownMenu.Portal>
                <DropdownMenu.Content
                  class="file-context-menu"
                  onMouseDown={() => {
                    suppressRowClick = true;
                  }}
                >
                  <FileMenuContent
                    model={props.model}
                    onTogglePin={props.onTogglePin}
                    onRemove={props.onRemove}
                    onClear={props.onClear}
                  />
                </DropdownMenu.Content>
              </DropdownMenu.Portal>
            </DropdownMenu>
          </Show>
        </div>
      </ContextMenu.Trigger>
      <ContextMenu.Portal>
        <ContextMenu.Content
          class="file-context-menu"
          onMouseDown={() => {
            suppressRowClick = true;
          }}
        >
          <FileMenuContent
            model={props.model}
            onTogglePin={props.onTogglePin}
            onRemove={props.onRemove}
            onClear={props.onClear}
          />
        </ContextMenu.Content>
      </ContextMenu.Portal>
    </ContextMenu>
  );
}

export function RecentList(props) {
  const recents = () => props.recents || [];
  const hasRecents = () => recents().length > 0;

  return (
    <Show when={hasRecents()}>
      <div id="box-recent">
        <div class="file-list-head text-normal">
          <div class="col-name">{t("colFileName")}</div>
          <div class="col-location">{t("colLocation")}</div>
          <div class="col-date">{t("colLastOpened")}</div>
        </div>
        <div class="file-list-body scrollable">
          <For each={recents()}>
            {(model) => (
              <RecentRow
                model={model}
                onTogglePin={props.onTogglePin}
                onRemove={props.onRemove}
                onClear={props.onClear}
              />
            )}
          </For>
        </div>
      </div>
    </Show>
  );
}
