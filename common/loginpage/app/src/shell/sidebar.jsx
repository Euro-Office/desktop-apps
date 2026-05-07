import { A } from "@solidjs/router";
import { t } from "@/services/l10n";
import "@/styles/components/sidebar.css";

export default function Sidebar() {
  return (
    <nav class="tool-menu">
      <li class="menu-item">
        <A href="/" activeClass="selected" end>
          <div class="icon-box">
            <svg class="icon">
              <use href="#home" />
            </svg>
          </div>
          <span class="text">{t("actHome")}</span>
        </A>
      </li>
      <li class="menu-item">
        <A href="/open" activeClass="selected">
          <div class="icon-box">
            <svg class="icon">
              <use href="#folder" />
            </svg>
          </div>
          <span class="text">{t("actOpenLocal")}</span>
        </A>
      </li>
      <li class="menu-item">
        <A href="/templates" activeClass="selected">
          <div class="icon-box">
            <svg class="icon">
              <use href="#templates" />
            </svg>
          </div>
          <span class="text">{t("actTemplates")}</span>
        </A>
      </li>
      <section id="idx-sidebar-portals" class="connect" />
      <li class="menu-item devider" />
      <li class="menu-item">
        <A href="/settings" activeClass="selected">
          <div class="icon-box">
            <svg class="icon">
              <use href="#settings" />
            </svg>
          </div>
          <span class="text">{t("actSettings")}</span>
        </A>
      </li>
      <li class="menu-item hidden">
        <A href="/about" activeClass="selected">
          <div class="icon-box">
            <svg class="icon">
              <use href="#about" />
            </svg>
          </div>
          <span class="text">{t("actAbout")}</span>
        </A>
      </li>
    </nav>
  );
}
