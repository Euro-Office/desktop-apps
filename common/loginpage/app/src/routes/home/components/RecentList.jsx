import { For, Show } from "solid-js";
import { openRecentFile, openRecoveryFile, exploreFile } from "@/sdk";
import { clsx } from "clsx";
import { t } from "@/services/l10n";
import { ContextMenu } from "@kobalte/core/context-menu";
import { DropdownMenu } from "@kobalte/core/dropdown-menu";
import './RecentList.styles.css'

let suppressRowClick = false;

function buildItems(model, isRecovery, onTogglePin, onRemove, onClear) {
  const items = [];

  items.push({
    key: "open",
    label: t("menuFileOpen"),
    onSelect: () => {
      if (isRecovery) openRecoveryFile(model);
      else openRecentFile(model);
    },
  });

  if (!isRecovery) {
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
  const items = () => buildItems(props.model, props.isRecovery, props.onTogglePin, props.onRemove, props.onClear);

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
  const model = props.model;

  return (
    <ContextMenu>
      <ContextMenu.Trigger
        as="div"
        class={clsx("row text-normal", model.pinned && "pinned", !model.exist && "lost")}
        onClick={() => {
          if (suppressRowClick) {
            suppressRowClick = false;
            return;
          }
          if (props.isRecovery) openRecoveryFile(model);
          else openRecentFile(model);
        }}
      >
        <div class="col-name" title={model.fullName}>
          <div class="icon">
            <svg class="icon">
              <use href={`#${model.type === "folder" ? "folder-small" : model.format}`} />
            </svg>
            <Show when={model.crypted}>
              <svg class="icon shield">
                <use href="#shield" />
              </svg>
            </Show>
          </div>
          <p class="name">
            {model.name}
            <span class="ext">{model.ext}</span>
          </p>
        </div>
        <div class="col-location" title={model.descr}>
          {model.descr}
        </div>
        <Show when={model.type !== "folder"}>
          <div class="col-date" title={model.date}>
            <p>{model.dateLabel}</p>
          </div>
        </Show>
        <div class='col-actions'>
          <button
            class='pin'
            onClick={(e) => {
              e.stopPropagation();
              props.onTogglePin(model.fileid);
            }}
          >
            <svg class="icon">
              <use href="#pin" />
            </svg>
          </button>
          <Show when={model.type !== "folder"}>
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
                      model={model}
                      isRecovery={props.isRecovery}
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
            model={model}
            isRecovery={props.isRecovery}
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
  const recovers = () => props.recovers || [];

  const hasRecovers = () => recovers().length > 0;
  const hasRecents = () => recents().length > 0;

  return (
    <>
      <Show when={hasRecovers()}>
        <div id="box-recovery">
          <div class="file-list-title">
            <h3>{t("listRecoveryTitle")}</h3>
          </div>
          <div class="file-list-head text-normal">
            <div class="col-name">{t("colFileName")}</div>
            <div class="col-location">{t("colLocation")}</div>
            <div class="col-date">{t("colLastOpened")}</div>
          </div>
          <div class="file-list-body scrollable">
            <For each={recovers()}>
              {(model) => (
                <RecentRow
                  model={model}
                  isRecovery={true}
                  onTogglePin={props.onTogglePin}
                  onRemove={(fileid) => props.onRemove(fileid, true)}
                  onClear={() => props.onClearAll(true)}
                />
              )}
            </For>
          </div>
        </div>
      </Show>

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
                  isRecovery={false}
                  onTogglePin={props.onTogglePin}
                  onRemove={(fileid) => props.onRemove(fileid, false)}
                  onClear={() => props.onClearAll(false)}
                />
              )}
            </For>
          </div>
        </div>
      </Show>
    </>
  );
}
