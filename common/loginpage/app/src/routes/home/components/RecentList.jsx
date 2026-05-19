import { For, Show } from "solid-js";
import { exploreFile, openRecentFile } from "@/sdk";
import { clsx } from "clsx";
import { t } from "@/services/l10n";
import { ContextMenu, DropdownMenu } from "@/components";
import "./RecentList.styles.css";

function FileMenuItems(props) {
  const Menu = props.menu;
  return (
    <>
      <Menu.Item icon="#gofolder" onSelect={() => exploreFile(props.model)}>
        {t("menuFileExplore")}
      </Menu.Item>
      <Menu.Item
        icon={props.model.pinned ? "#unpin20" : "#pin20"}
        onSelect={() => props.onTogglePin(props.model.fileid)}
      >
        {t(props.model.pinned ? "menuFileUnpin" : "menuFilePin")}
      </Menu.Item>
      <Menu.Item icon="#remove" onSelect={() => props.onRemove(props.model.fileid)}>
        {t("menuRemoveModel")}
      </Menu.Item>
      <Menu.Separator />
      <Menu.Item class="negative" onSelect={props.onClear}>
        {t("menuClear")}
      </Menu.Item>
    </>
  );
}

function RecentRow(props) {
  return (
    <ContextMenu>
      <ContextMenu.Trigger
        as="div"
        class={clsx("row text-normal", props.model.pinned && "pinned", !props.model.exist && "lost")}
        onClick={() => openRecentFile(props.model)}
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
              <DropdownMenu.Trigger
                class="more"
                onClick={(e) => e.stopPropagation()}
              >
                <svg class="icon">
                  <use href="#more" />
                </svg>
              </DropdownMenu.Trigger>
              <DropdownMenu.Content onContextMenu={(e) => e.stopPropagation()}>
                <FileMenuItems
                  menu={DropdownMenu}
                  model={props.model}
                  onTogglePin={props.onTogglePin}
                  onRemove={props.onRemove}
                  onClear={props.onClear}
                />
              </DropdownMenu.Content>
            </DropdownMenu>
          </Show>
        </div>
      </ContextMenu.Trigger>
      <ContextMenu.Content>
        <FileMenuItems
          menu={ContextMenu}
          model={props.model}
          onTogglePin={props.onTogglePin}
          onRemove={props.onRemove}
          onClear={props.onClear}
        />
      </ContextMenu.Content>
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
