import { Button as ButtonPrimitive } from "@kobalte/core/button";
import { mergeProps, splitProps } from "solid-js";
import "./Button.styles.css";

const variantClass = {
  default: "btn-default",
  primary: "btn-primary",
  accent: "btn-accent",
};

const defaultProps = {
  variant: "default",
}

export function Button(props) {
  const merged = mergeProps(defaultProps, props);
  const [local, rest] = splitProps(merged, ["variant", "class"]);

  return (
    <ButtonPrimitive
      class='btn'
      classList={{
        [variantClass[local.variant]]: true,
        [local.class]: !!local.class,
      }}
      {...rest}
    />
  );
}
