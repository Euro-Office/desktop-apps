import { Show, splitProps } from "solid-js";
import { Switch as Primitive } from "@kobalte/core/switch";
import styles from "./Switch.module.css";
import { mergeClass } from "@/utils/classes";

export function Switch(props) {
  const [local, rest] = splitProps(props, ["class", "children"]);
  return (
    <Primitive.Root classList={mergeClass(styles.switchRoot, local.class)} {...rest}>
      <Primitive.Control class={styles.switchControl}>
        <Primitive.Thumb class={styles.switchThumb} />
      </Primitive.Control>
      <Show when={local.children}>
        <Primitive.Label class={styles.switchLabel}>{local.children}</Primitive.Label>
      </Show>
      <Primitive.Input />
    </Primitive.Root>
  );
}
