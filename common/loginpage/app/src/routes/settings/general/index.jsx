import { t } from "@/services/l10n";
import styles from "./index.module.css";

export default function GeneralSettings() {
  return (
    <div class={styles.page}>
      <div class={styles.title}>
        <h1>{t("settingsGeneral")}</h1>
      </div>
      <div class={styles.form}>

      </div>
    </div>
  );
}
