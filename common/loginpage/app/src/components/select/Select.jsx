import { splitProps } from "solid-js";
import { Select as Primitive } from "@kobalte/core/select";
import { mergeClass } from "@/utils/classes";
import styles from "./Select.module.css";

function Root(props) {
  return <Primitive {...props} />;
}

function Trigger(props) {
  const [local, rest] = splitProps(props, ["class", "placeholder"]);
  return (
    <Primitive.Trigger classList={mergeClass(styles.trigger, local.class)} {...rest}>
      <Primitive.Value placeholder={local.placeholder} class={styles.value} />
      <Primitive.Icon class={styles.icon}>
        <svg viewBox="0 0 16 16" aria-hidden="true">
          <path
            d="M4 6L8 10L12 6"
            stroke="currentColor"
            stroke-width="1.5"
            fill="none"
            stroke-linecap="round"
            stroke-linejoin="round"
          />
        </svg>
      </Primitive.Icon>
    </Primitive.Trigger>
  );
}

function Content(props) {
  const [local, rest] = splitProps(props, ["class", "children"]);
  return (
    <Primitive.Portal>
      <Primitive.Content classList={mergeClass(styles.content, local.class)} {...rest}>
        <Primitive.Listbox class={styles.listbox}>{local.children}</Primitive.Listbox>
      </Primitive.Content>
    </Primitive.Portal>
  );
}

function Item(props) {
  const [local, rest] = splitProps(props, ["class", "children"]);
  return (
    <Primitive.Item classList={mergeClass(styles.item, local.class)} {...rest}>
      <Primitive.ItemLabel>{local.children}</Primitive.ItemLabel>
      <Primitive.ItemIndicator class={styles.itemIndicator}>
        <svg viewBox="0 0 16 16" aria-hidden="true">
          <path
            d="M3 8L6.5 11.5L13 5"
            stroke="currentColor"
            stroke-width="1.5"
            fill="none"
            stroke-linecap="round"
            stroke-linejoin="round"
          />
        </svg>
      </Primitive.ItemIndicator>
    </Primitive.Item>
  );
}

export const Select = Object.assign(Root, { Trigger, Content, Item });
