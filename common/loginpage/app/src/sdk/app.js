import { cmd } from "./bridge";

export function signalAppReady() {
  cmd("app:onready", "");
}
