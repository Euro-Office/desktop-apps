import { Show, splitProps } from "solid-js";
import { Switch as Primitive } from "@kobalte/core/switch";
import { cx } from "@knst/clsv";
import "./Switch.styles.css";

export function Switch(props) {
  const [local, rest] = splitProps(props, ["class", "children"]);
  return (
    <Primitive.Root class={cx("switch-root", local.class)} {...rest}>
      <Primitive.Control class="switch-control">
        <Primitive.Thumb class="switch-thumb" />
      </Primitive.Control>
      <Show when={local.children}>
        <Primitive.Label class="switch-label">{local.children}</Primitive.Label>
      </Show>
      <Primitive.Input />
    </Primitive.Root>
  );
}
