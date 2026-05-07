import { lazy } from "solid-js";
import { HashRouter } from "@solidjs/router";
import Sidebar from "./shell/sidebar";
import "./styles/layout.css";
import "./styles/components/common.css";

function Shell(props) {
  return (
    <div class="shell">
      <div class="col-sidebar">
        <Sidebar />
      </div>
      <div class="col-center">{props.children}</div>
      {/* TODO: AI Panel component */}
    </div>
  );
}

/** @type {import("@solidjs/router").RouteDefinition[]} */
const routes = [
  { path: "/", component: lazy(() => import("./routes/home/index.jsx")) },
  { path: "/open", component: lazy(() => import("./routes/open/index.jsx")) },
  { path: "/templates", component: lazy(() => import("./routes/templates/index.jsx")) },
  { path: "/clouds/*", component: lazy(() => import("./routes/clouds/index.jsx")) },
  { path: "/settings", component: lazy(() => import("./routes/settings/index.jsx")) },
  { path: "/about", component: lazy(() => import("./routes/about/index.jsx")) },
];

export default function App() {
  return (
    <HashRouter root={Shell}>
      {routes}
    </HashRouter>
  );
}
