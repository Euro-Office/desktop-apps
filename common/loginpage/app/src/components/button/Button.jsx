import { cva } from '@knst/clsv'
import { Button as ButtonPrimitive } from "@kobalte/core/button";
import './Button.styles.css'
import {splitProps} from "solid-js";

export const buttonVariants = cva({
  base: 'btn',
  variants: {
    variant: {
      default: 'btn-default',
      primary: 'btn-primary',
      accent: 'btn-accent',
    }
  },
  defaultVariants: {
    variant: 'default'
  }
})

export function Button(props) {
  const [, rest] = splitProps(props, ['variant', 'class'])
  return (
    <ButtonPrimitive class={buttonVariants({ variant: props.variant, class: props.class })} {...rest}/>
  )
}
