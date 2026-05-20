import { Show, splitProps } from "solid-js";
import { Switch as Primitive } from "@kobalte/core/switch";
import "./Switch.styles.css";
import { mergeClass } from "@/utils/classes";

export function Switch(props) {
  const [local, rest] = splitProps(props, ["class", "children"]);
  return (
    <Primitive.Root classList={mergeClass("switch-root", local.class)} {...rest}>
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
