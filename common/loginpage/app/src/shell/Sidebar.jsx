import { A } from "@solidjs/router";
import { t } from "@/services/l10n";
import "./Sidebar.styles.css";

export default function Sidebar() {
  return (
    <nav class="tool-menu">
      <li class="menu-item">
        <A href="/" activeClass="selected" end>
          <svg class="icon">
            <use href="#home" />
          </svg>
          <span class="text">{t("actHome")}</span>
        </A>
      </li>
      <li class="menu-item">
        <A href="/open" activeClass="selected">
          <svg class="icon">
            <use href="#folder" />
          </svg>
          <span class="text">{t("actOpenLocal")}</span>
        </A>
      </li>
      <li class="menu-item">
        <A href="/templates" activeClass="selected">
          <svg class="icon">
            <use href="#templates" />
          </svg>
          <span class="text">{t("actTemplates")}</span>
        </A>
      </li>
      <div id="idx-sidebar-portals" class="connect" />
      <li class="menu-item devider" />
      <li class="menu-item">
        <A href="/settings" activeClass="selected">
          <svg class="icon">
            <use href="#settings" />
          </svg>
          <span class="text">{t("actSettings")}</span>
        </A>
      </li>
      <li class="menu-item hidden">
        <A href="/about" activeClass="selected">
          <svg class="icon">
            <use href="#about" />
          </svg>
          <span class="text">{t("actAbout")}</span>
        </A>
      </li>
    </nav>
  );
}
