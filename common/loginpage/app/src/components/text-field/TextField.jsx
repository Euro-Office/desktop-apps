import { splitProps } from "solid-js";
import { TextField as TextFieldPrimitive } from "@kobalte/core/text-field";
import { mergeClass } from "@/utils/classes";
import styles from "./TextField.module.css";

function Root(props) {
  const [local, rest] = splitProps(props, ["class"]);
  return (
    <TextFieldPrimitive classList={mergeClass(styles.root, local.class)} {...rest} />
  );
}

function Input(props) {
  const [local, rest] = splitProps(props, ["class"]);
  return (
    <TextFieldPrimitive.Input classList={mergeClass(styles.input, local.class)} {...rest} />
  );
}

export const TextField = Object.assign(Root, { Input });
