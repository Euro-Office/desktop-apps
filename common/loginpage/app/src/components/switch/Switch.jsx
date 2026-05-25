import { splitProps } from "solid-js";
import { Switch as SwitchPrimitive } from "@kobalte/core/switch";
import styles from "./Switch.module.css";
import { mergeClass } from "@/utils/classes";

export function Switch(props) {
  const [local, rest] = splitProps(props, ["class", "children"]);
  return (
    <SwitchPrimitive classList={mergeClass(styles.root, local.class)} {...rest}>
      <SwitchPrimitive.Control class={styles.control}>
        <SwitchPrimitive.Thumb class={styles.thumb} />
      </SwitchPrimitive.Control>
      {local.children}
      <SwitchPrimitive.Input />
    </SwitchPrimitive>
  );
}
