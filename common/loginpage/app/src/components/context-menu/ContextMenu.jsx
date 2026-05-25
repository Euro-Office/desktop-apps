import { Show, splitProps } from "solid-js";
import { ContextMenu as ContextMenuPrimitive } from "@kobalte/core/context-menu";
import { mergeClass } from "@/utils/classes";
import styles from "./ContextMenu.module.css";

function Root(props) {
  return <ContextMenuPrimitive {...props} />;
}

function Trigger(props) {
  return <ContextMenuPrimitive.Trigger {...props} />;
}

function Content(props) {
  const [, rest] = splitProps(props, ["class"]);
  return (
    <ContextMenuPrimitive.Portal>
      <ContextMenuPrimitive.Content classList={mergeClass(styles.menu, props.class)} {...rest} />
    </ContextMenuPrimitive.Portal>
  );
}

function Item(props) {
  const [, rest] = splitProps(props, ["class", "icon", "children"]);
  return (
    <ContextMenuPrimitive.Item classList={mergeClass(styles.menuItem, props.class)} {...rest}>
      <Show when={props.icon}>
        <svg class={styles.menuIcon}>
          <use href={props.icon} />
        </svg>
      </Show>
      {props.children}
    </ContextMenuPrimitive.Item>
  );
}

function Separator(props) {
  const [, rest] = splitProps(props, ["class"]);
  return <ContextMenuPrimitive.Separator classList={mergeClass(styles.menuSeparator, props.class)} {...rest} />;
}

export const ContextMenu = Object.assign(Root, { Trigger, Content, Item, Separator });
