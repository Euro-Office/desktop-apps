import { Show, splitProps } from "solid-js";
import { ContextMenu as Primitive } from "@kobalte/core/context-menu";
import { cx } from "@knst/clsv";
import "./ContextMenu.styles.css";

function Root(props) {
  return <Primitive {...props} />;
}

function Trigger(props) {
  return <Primitive.Trigger {...props} />;
}

function Content(props) {
  const [, rest] = splitProps(props, ["class"]);
  return (
    <Primitive.Portal>
      <Primitive.Content class={cx("menu", props.class)} {...rest} />
    </Primitive.Portal>
  );
}

function Item(props) {
  const [, rest] = splitProps(props, ["class", "icon", "children"]);
  return (
    <Primitive.Item class={cx("menu-item", props.class)} {...rest}>
      <Show when={props.icon}>
        <svg class="menu-icon">
          <use href={props.icon} />
        </svg>
      </Show>
      {props.children}
    </Primitive.Item>
  );
}

function Separator(props) {
  const [, rest] = splitProps(props, ["class"]);
  return <Primitive.Separator class={cx("menu-separator", props.class)} {...rest} />;
}

export const ContextMenu = Object.assign(Root, { Trigger, Content, Item, Separator });
