# Kylin Space Guard

This project is the KylinOS V11 desktop space cleaner application.

## Scope

- Desktop GUI for understanding and safely optimizing KylinOS V11 root partition usage.
- Kaiming application/base/runtime container visibility and safe old-container cleanup planning.
- KARE and ostree writable-layer visibility.
- Startup/preheat management with explicit enable/disable and restore behavior.
- Clear before/after/current usage results for user-triggered actions.

## Development Rules

- Keep project-specific development work in this repository, not in `$HOME/.os-fix-skill`.
- Prefer mature existing libraries and Qt official modules for common functionality.
- Do not hand-roll charting, icon systems, automation, parsing, or platform integration when a maintained system library is available and reasonably lightweight.
- Use C++/Qt Widgets for the desktop UI unless there is a clear reason to switch.
- Use Qt Charts for chart visualization when available.
- Keep the app lightweight: avoid WebEngine, heavyweight QML scenes, or bundled third-party runtimes unless explicitly justified.
- Cleanup actions must show candidates, selection, plan, progress, and result. Do not silently delete system content.
- Kaiming old container cleanup must only move verified old, unused layers to rollback quarantine, not delete current layers.
- Root partition usage is a total. Kaiming, ostree upper, KARE upper, and other categories are child breakdowns or highlighted writable areas under that total.
- Application containers are an app-level breakdown inside Kaiming usage; they do not equal total Kaiming usage.
- If a user asks why root space changed, compare `/`, `/home`, and `/data` mount points first so DATA/HOME content is not misattributed to root.

## UI Verification

- GUI changes must be verified with a real desktop window, screenshots, and mouse interaction via `kylinos-v11-desktop-computer-use`.
- Before checking the latest UI, close old cleaner processes, launch the installed app with `QT_QPA_PLATFORM=xcb`, bring it to the front, then capture screenshots.
- Use real interaction for page changes and buttons. Do not rely only on offscreen launch tests.
- Keep visible UI language consistent with the selected language.

## Commands

```bash
git status --short --branch
make
make install
env QT_QPA_PLATFORM=xcb ~/.local/share/kylin-space-guard/bin/kylin-space-cleaner
~/.local/bin/kylinos-v11-desktop-computer-use click <x> <y>
```

## Git

- Use author `Swordup-Z <swordup.zeng@gmail.com>`.
- Commit project changes after verification.
- This repository currently has no remote; do not assume push is available until the user provides a GitHub repository URL.
