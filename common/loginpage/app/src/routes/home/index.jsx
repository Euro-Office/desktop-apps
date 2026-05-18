import { Show, onCleanup, onMount } from "solid-js";
import { createStore, reconcile } from "solid-js/store";
import { t } from "@/services/l10n.js";
import {
  listRecents,
  onRecentsChanged,
  parseRecent,
  pinRecent,
  removeRecent,
  clearRecents,
} from "@/sdk";
import { DocTypeGrid } from "./components/DocTypeGrid.jsx";
import { RecentList } from "./components/RecentList.jsx";
import { FileDropZone } from "./components/FileDropZone.jsx";
import "@/styles/components/home.css";

const toArray = (v) => (Array.isArray(v) ? v : []);

function sortByPin(arr) {
  arr.sort((a, b) => a.pinid - b.pinid);
  return arr;
}

export default function Index() {
  const [recents, setRecents] = createStore([]);

  const hasFiles = () => recents.length > 0;

  function loadRecents(raw) {
    setRecents(reconcile(sortByPin(parseRecent(toArray(raw))), { key: "uid" }));
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

  function markFileRemoved(fileid) {
    removeRecent(fileid);
    setRecents(
      reconcile(
        recents.map((r) => (r.fileid === fileid ? { ...r, exist: false } : r)),
        { key: "uid" },
      ),
    );
  }

  function clearAll() {
    clearRecents();
    setRecents(reconcile([], { key: "uid" }));
  }

  onMount(() => {
    listRecents()
      .then(loadRecents)
      .catch(() => {});

    const unsubRecents = onRecentsChanged((p) => loadRecents(p));

    onCleanup(() => {
      unsubRecents();
    });
  });

  return (
    <div class="home-page">
      <div class="cnt-title">
        <h1>{t("actCreateNew")}</h1>
      </div>

      <section class="new-doc-section">
        <DocTypeGrid />
      </section>

      <Show when={!hasFiles()}>
        <FileDropZone />
      </Show>

      <section class="file-list-section">
        <RecentList
          recents={recents}
          onTogglePin={togglePin}
          onRemove={markFileRemoved}
          onClear={clearAll}
        />
      </section>
    </div>
  );
}
