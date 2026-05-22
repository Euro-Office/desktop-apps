import { spawn } from "node:child_process";
import { mkdirSync, readFileSync, readdirSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const _dirname = dirname(fileURLToPath(import.meta.url));
const root = join(_dirname, "..");
const outDir = join(root, "..", "build", "dist-solid");
const port = 3000;

function injectSprites(html) {
  const spritesDir = join(_dirname, "..", "..", "res", "img", "generated");
  let svgContent = "";
  try {
    const files = readdirSync(spritesDir).filter((f) => f.endsWith(".svg"));
    for (const file of files) {
      svgContent += readFileSync(join(spritesDir, file), "utf-8");
    }
  } catch {
    return html;
  }
  if (!svgContent) return html;
  return html.replace('<div id="root"></div>', `${svgContent}\n<div id="root"></div>`);
}

const html = readFileSync(join(root, "index.html"), "utf-8");

const devHtml = injectSprites(html).replace(
  '<script type="module" src="./src/main.jsx"></script>',
  `<script type="module" src="http://localhost:${port}/@vite/client"></script>
<script type="module" src="http://localhost:${port}/src/main.jsx"></script>`,
);

mkdirSync(outDir, { recursive: true });
writeFileSync(join(outDir, "index.html"), devHtml);
console.log(`Dev HTML written to ${outDir}/index.html`);

const vite = spawn("npx", ["vite", "--port", String(port)], {
  cwd: root,
  stdio: "inherit",
  shell: process.platform === "win32",
});

vite.on("close", (code) => {
  process.exit(code);
});
