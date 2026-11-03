# Desktop apps: Frontend

<p align="center"> 
  <a href="http://www.gnu.org/licenses/agpl-3.0.html"><img alt="License" src="https://img.shields.io/badge/License-AGPL%20v3.0-green?style=for-the-badge"></a>  
</p> 

This repo contains the frontend for [Desktop Editors](https://github.com/Euro-Office/DesktopEditors) which builds the program interface. [Desktop Editors](https://github.com/Euro-Office/DesktopEditors) is a free and open-source office suite that works offline on your Windows, Linux, and macOS computer. It offers maximum compatibility with Microsoft Office formats (DOCX, XLSX, PPTX) and allows you to connect to the cloud for real-time collaboration.

## **Why choose Desktop Editors? ✨**

* **✈️ Work offline, anytime:** Create and edit documents locally without needing an internet connection. Your work is always available on your machine.  
* **☁️ Connect to the cloud:** Integrate seamlessly with Seafile, ownCloud, Nextcloud and other platforms to co-author documents with your team in real time.  
* **📄 Unrivaled compatibility:** Enjoy flawless work with DOCX, XLSX, PPTX, and PDF files. We also support all other popular formats, including, ODT, ODS, ODP, CSV, etc.  
* **🛠️ All the tools you need:** Get a complete set of professional editing and formatting tools for creating stunning text documents, spreadsheets, and presentations.  
* **📝 More than just docs:** View, annotate, and convert PDF files. Create and fill out complex, interactive PDF forms. View and navigate diagrams right in the app.
* **🔒 Secure & private:** Protect your sensitive files with password encryption and digital signatures.  
* **🧩 Extendable with plugins:** Enhance your editing experience with a variety of built-in and third-party plugins like Doc2md, Draw.io, Highlight Code, and others.
* **🤖 AI integration:** Connect any AI model (local or cloud-based) for tasks like chatbot requests, translation, OCR, etc. Use AI agents to generate new files, list folder contents, preview file details without opening them in the editor, auto-fill forms with provided data, etc.

## **For developers: Building from source 👨‍💻**

This repository (`desktop-apps`) contains the frontend shell for the Desktop Editors. The core editing engine and conversion components are located in the main [DesktopEditors](https://github.com/Euro-Office/DesktopEditors) repository.

In a DesktopEditors checkout, `scripts/build.sh` (Linux, macOS) and `scripts/build.ps1` (Windows) build this repository against core. They download the prebuilt core for the `core` checkout when the build cache has it, and build core first otherwise, or when `core` has local changes. Options: `scripts/build.sh --help`, `Get-Help scripts\build.ps1`.
