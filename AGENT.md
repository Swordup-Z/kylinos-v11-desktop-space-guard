# Project Agent Notes

This project is the KylinOS V11 desktop space cleaner application.

## Development Rules

- Keep project-specific development work in this repository, not in the system-repair skill repository.
- Prefer mature existing libraries and Qt official modules for common functionality.
- Do not hand-roll charting, icon systems, automation, parsing, or platform integration when a maintained system library is available and reasonably lightweight.
- Use C++/Qt Widgets for the desktop UI unless there is a clear reason to switch.
- Use Qt Charts for chart visualization when available.
- Keep the app lightweight: avoid WebEngine, heavyweight QML scenes, or bundled third-party runtimes unless explicitly justified.
- GUI changes must be verified with a real desktop window, screenshots, and mouse interaction via `kylinos-v11-desktop-computer-use`.
- Start GUI verification instances with `QT_QPA_PLATFORM=xcb` so window-manager control and mouse simulation are deterministic.
- Do not use this cleaner to silently delete system content. Cleanup actions must show candidates, selection, plan, progress, and result.
- Kaiming old container cleanup must only move verified old, unused layers to rollback quarantine, not delete current layers.
- Root partition usage is a total. Kaiming, ostree upper, KARE upper, and other categories are child breakdowns or highlighted writable areas under that total.
- Application containers are an app-level breakdown inside Kaiming usage; they do not equal total Kaiming usage.

## Verification

```bash
make
make install
env QT_QPA_PLATFORM=xcb ~/.local/share/kylin-space-guard/bin/kylin-space-cleaner
~/.local/bin/kylinos-v11-desktop-computer-use click <x> <y>
```
