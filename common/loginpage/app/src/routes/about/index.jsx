import { useLocation } from "@solidjs/router";

export default function About() {
  const loc = useLocation();
  console.log("hit splat with pathname:", loc.pathname);
  return <div class="stub-page">About — TODO</div>;
}
