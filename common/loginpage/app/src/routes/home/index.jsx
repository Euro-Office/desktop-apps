import { createMemo, createSignal, Show, onCleanup, onMount } from "solid-js";
import { createStore, reconcile } from "solid-js/store";
import { makePersisted } from "@solid-primitives/storage";
import { t } from "@/services/l10n.js";
import {
  listRecents,
  listRecovers,
  onRecentsChanged,
  onRecoversChanged,
  parseRecent,
  pinRecent,
  removeRecent,
  removeRecover,
  clearRecents,
  clearRecovers,
} from "@/sdk";
import DocTypeGrid from "./components/doc-type-grid.jsx";
import RecentList from "./components/recent-list.jsx";
import DnDZone from "./components/dnd-zone.jsx";
import "@/styles/components/home.css";

const toArray = (v) => (Array.isArray(v) ? v : []);

function sortByPin(arr) {
  arr.sort((a, b) => a.pinid - b.pinid);
  return arr;
}

export default function Index() {
  const [welcomeDismissed, setWelcomeDismissed] = makePersisted(createSignal(false), { name: "welcome" });
  const [recents, setRecents] = createStore([]);
  const [recovers, setRecovers] = createStore([]);
  const recentRows = createMemo(() => recents);
  const recoverRows = createMemo(() => recovers);

  const hasFiles = () => recents.length > 0 || recovers.length > 0;

  function loadRecents(raw) {
    setRecents(reconcile(sortByPin(parseRecent(toArray(raw))), { key: "uid" }));
  }

  function loadRecovers(raw) {
    setRecovers(reconcile(parseRecent(toArray(raw)), { key: "uid" }));
  }

  function togglePin(fileid) {
    const arr = recents.map((r) => {
      if (r.fileid === fileid) {
        const newPinned = !r.pinned;
        pinRecent(fileid, newPinned);
        return { ...r, pinned: newPinned, pinid: newPinned ? -r.fileid : r.fileid };
      }
      return r;
    });
    setRecents(reconcile(sortByPin(arr), { key: "uid" }));
  }

  function removeFile(fileid, isRecovery) {
    if (isRecovery) {
      removeRecover(fileid);
      setRecovers(
        reconcile(
          recovers.map((r) => (r.fileid === fileid ? { ...r, exist: false } : r)),
          { key: "uid" },
        ),
      );
    } else {
      removeRecent(fileid);
      setRecents(
        reconcile(
          recents.map((r) => (r.fileid === fileid ? { ...r, exist: false } : r)),
          { key: "uid" },
        ),
      );
    }
  }

  function clearAll(isRecovery) {
    if (isRecovery) {
      clearRecovers();
      setRecovers(reconcile([], { key: "uid" }));
    } else {
      clearRecents();
      setRecents(reconcile([], { key: "uid" }));
    }
  }

  onMount(() => {
    setWelcomeDismissed(true);

    listRecents()
      .then(loadRecents)
      .catch(() => {});
    listRecovers()
      .then(loadRecovers)
      .catch(() => {});

    const unsubRecents = onRecentsChanged((p) => loadRecents(p));
    const unsubRecovers = onRecoversChanged((p) => loadRecovers(p));

    onCleanup(() => {
      unsubRecents();
      unsubRecovers();
    });
  });

  const [helpBefore, helpAfter = ""] = t("welNeedHelp").split("$1");

  return (
    <div class="recent-panel-container">
      <div class='cnt-title'>
        {/* FIXME: i18n */}
        <h1>Create new</h1>
      </div>

      <section class="new-doc-section">
        <DocTypeGrid t={t} />
      </section>

      <Show when={!welcomeDismissed()}>
        <div id="area-welcome">
          <h2>{t("welWelcome")}</h2>
          <p class="text-normal">{t("welDescr")}</p>
          <p class="text-normal">
            {helpBefore}
            <a class="link" href="https://helpcenter.onlyoffice.com/" target="popup">
              {t("textHelpCenter")}
            </a>
            {helpAfter}
          </p>
        </div>
      </Show>

      <Show when={!hasFiles()}>
        <section id="area-dnd-file">
          <DnDZone />
        </section>
      </Show>

      <div id="box-container">
        <RecentList
          t={t}
          recents={recentRows()}
          recovers={recoverRows()}
          onTogglePin={togglePin}
          onRemove={removeFile}
          onClearAll={clearAll}
        />
      </div>
    </div>
  );
}
