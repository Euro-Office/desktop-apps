import { Button as ButtonPrimitive } from "@kobalte/core/button";
import { mergeProps, splitProps } from "solid-js";
import styles from "./Button.module.css";

const variantClass = {
  default: styles.btnDefault,
  primary: styles.btnPrimary,
  accent: styles.btnAccent,
};

const defaultProps = {
  variant: "default",
}

export function Button(props) {
  const merged = mergeProps(defaultProps, props);
  const [local, rest] = splitProps(merged, ["variant", "class"]);

  return (
    <ButtonPrimitive
      class={styles.btn}
      classList={{
        [variantClass[local.variant]]: true,
        [local.class]: !!local.class,
      }}
      {...rest}
    />
  );
}
