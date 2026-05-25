import { mkdirSync, readFileSync, readdirSync, writeFileSync } from "node:fs";
import { join } from "node:path";
import { fileURLToPath } from "node:url";
import { defineConfig } from "vite";
import solid from "vite-plugin-solid";
import { viteSingleFile } from "vite-plugin-singlefile";

const DEV_PORT = 3000;
const _dirname = fileURLToPath(new URL(".", import.meta.url));

function injectSprites() {
  return {
    name: "inject-sprites",
    transformIndexHtml(html) {
      const spritesDir = join(_dirname, "../res/img/generated");
      let svgContent = "";
      try {
        const files = readdirSync(spritesDir).filter((f) => f.endsWith(".svg"));
        for (const file of files) {
          svgContent += readFileSync(join(spritesDir, file), "utf-8");
        }
      } catch {
        // sprites not available
      }
      if (!svgContent) return html;
      return html.replace('<div id="root"></div>', `${svgContent}\n<div id="root"></div>`);
    },
  };
}

function writeDevHtml(port) {
  return {
    name: "write-dev-html",
    configureServer(server) {
      server.httpServer?.once("listening", () => {
        const outDir = join(_dirname, "../build/dist-solid");
        const html = readFileSync(join(_dirname, "index.html"), "utf-8");

        const spritesDir = join(_dirname, "../res/img/generated");
        let svgContent = "";
        try {
          const files = readdirSync(spritesDir).filter((f) => f.endsWith(".svg"));
          for (const file of files) svgContent += readFileSync(join(spritesDir, file), "utf-8");
        } catch {
          /* ok */
        }

        const withSprites = svgContent ? html.replace('<div id="root"></div>', `${svgContent}\n<div id="root"></div>`) : html;

        const devHtml = withSprites.replace(
          '<script type="module" src="./src/main.jsx"></script>',
          `<script type="module" src="http://localhost:${port}/@vite/client"></script>\n` +
            `<script type="module" src="http://localhost:${port}/src/main.jsx"></script>`,
        );

        mkdirSync(outDir, { recursive: true });
        writeFileSync(join(outDir, "index.html"), devHtml);
      });
    },
  };
}

export default defineConfig(({ mode }) => ({
  base: "./",
  server: {
    port: DEV_PORT,
    cors: { origin: "*" },
  },
  plugins: [
    injectSprites(),
    solid(),
    mode === "development" && writeDevHtml(DEV_PORT),
    mode === "production" && viteSingleFile({ removeViteModuleLoader: true }),
  ].filter(Boolean),
  css: {
    transformer: "lightningcss",
  },
  build: {
    outDir: "../build/dist-solid",
    target: "es2022",
    cssMinify: "lightningcss",
    assetsInlineLimit: 100_000_000,
  },
  resolve: {
    alias: {
      "@": "/src",
    },
  },
}));
