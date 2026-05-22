import styles from "./layout.module.css";
import { t } from "@/services/l10n.js";
import {A} from "@solidjs/router";

export default function SettingsLayout(props) {
  return (
    <div class={styles.layout}>
      <nav class={styles.sidebar}>
        <div class={styles.heading}>
          <span>{t("actSettings")}</span>
        </div>
        <ul class={styles.list}>
          <li class={styles.item}>
            <A href="/settings/general" activeClass={styles.selected} end>
              {t("settingsGeneral")}
            </A>
          </li>
        </ul>
      </nav>
      <main class={styles.content}>{props.children}</main>
    </div>
  );
}
