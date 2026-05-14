import { readFileSync, readdirSync } from "node:fs";
import { join } from "node:path";
import { fileURLToPath } from "node:url";
import { defineConfig } from "vite";
import solid from "vite-plugin-solid";
import { viteSingleFile } from "vite-plugin-singlefile";

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
      } catch (e) {
        // sprites not available
      }
      if (!svgContent) return html;
      return html.replace('<div id="root"></div>', `${svgContent}\n<div id="root"></div>`);
    },
  };
}

export default defineConfig({
  base: "./",
  plugins: [injectSprites(), solid(), viteSingleFile({ removeViteModuleLoader: true })],
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
});
