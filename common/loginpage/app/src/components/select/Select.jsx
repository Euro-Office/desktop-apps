import { splitProps } from "solid-js";
import { Select as SelectPrimitive } from "@kobalte/core/select";
import { mergeClass } from "@/utils/classes";
import styles from "./Select.module.css";

const getItemValue = (o) => o?.value ?? o;
const getItemLabel = (o) => o?.label ?? String(o ?? "");

function Root(props) {
  const [local, rest] = splitProps(props, ["onChange", "options", "value"]);

  return (
    <SelectPrimitive
      options={local.options ?? []}
      optionValue={getItemValue}
      optionTextValue={getItemLabel}
      value={local.value ?? null}
      itemComponent={(itemProps) => (
        <SelectPrimitive.Item item={itemProps.item} class={styles.item}>
          <SelectPrimitive.ItemLabel>{getItemLabel(itemProps.item.rawValue)}</SelectPrimitive.ItemLabel>
          <SelectPrimitive.ItemIndicator class={styles.itemIndicator}>
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
          </SelectPrimitive.ItemIndicator>
        </SelectPrimitive.Item>
      )}
      onChange={(v) => local.onChange?.(getItemValue(v))}
      {...rest}
    />
  );
}

function Trigger(props) {
  const [local, rest] = splitProps(props, ["class", "placeholder"]);
  return (
    <SelectPrimitive.Trigger classList={mergeClass(styles.trigger, local.class)} {...rest}>
      <SelectPrimitive.Value placeholder={local.placeholder} class={styles.value}>
        {(state) => getItemLabel(state.selectedOption())}
      </SelectPrimitive.Value>
      <SelectPrimitive.Icon class={styles.icon}>
        <svg viewBox="0 0 16 16" aria-hidden="true">
          <path d="M4 6L8 10L12 6" stroke="currentColor" stroke-width="1.5" fill="none" stroke-linecap="round" stroke-linejoin="round" />
        </svg>
      </SelectPrimitive.Icon>
    </SelectPrimitive.Trigger>
  );
}

function Content(props) {
  const [local, rest] = splitProps(props, ["class"]);
  return (
    <SelectPrimitive.Portal>
      <SelectPrimitive.Content classList={mergeClass(styles.content, local.class)} {...rest}>
        <SelectPrimitive.Listbox class={styles.listbox} />
      </SelectPrimitive.Content>
    </SelectPrimitive.Portal>
  );
}

function Item(props) {
  const [local, rest] = splitProps(props, ["class", "children"]);
  return (
    <SelectPrimitive.Item classList={mergeClass(styles.item, local.class)} {...rest}>
      <SelectPrimitive.ItemLabel>{local.children}</SelectPrimitive.ItemLabel>
      <SelectPrimitive.ItemIndicator class={styles.itemIndicator}>
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
      </SelectPrimitive.ItemIndicator>
    </SelectPrimitive.Item>
  );
}

export const Select = Object.assign(Root, { Trigger, Content, Item });
