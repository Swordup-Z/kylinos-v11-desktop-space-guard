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
- Use existing icon and chart libraries where they improve quality without adding heavyweight runtimes.
- Keep the app lightweight: avoid WebEngine, heavyweight QML scenes, or bundled third-party runtimes unless explicitly justified.
- Cleanup actions must show candidates, selection, plan, progress, and result. Do not silently delete system content.
- Kaiming old container cleanup must only move verified old, unused layers to rollback quarantine, not delete current layers.
- Root partition usage is a total. Kaiming, ostree upper, KARE upper, and other categories are child breakdowns or highlighted writable areas under that total.
- Application containers are an app-level breakdown inside Kaiming usage; they do not equal total Kaiming usage.
- If a user asks why root space changed, compare `/`, `/home`, and `/data` mount points first so DATA/HOME content is not misattributed to root.
- Startup/preheat management must support both disable and restore. It must show affected entries and intended changes before applying them.
- Do not label space cleanup as application uninstall. Uninstalling apps is application management; cleanup is about verified old or unused container/layer data.

## UI Requirements

- Default UI language is Simplified Chinese. Provide a Chinese/English switch; `zh_CN` shows Chinese names and `en_US` shows English names.
- Chinese desktop name: `麒麟V11空间清理`. English desktop name should clearly reflect KylinOS V11 desktop space cleanup.
- The first screen is a clean overview, not an instruction page. The second primary card/page is scanning.
- Scanning starts only when the user triggers it, except for one initial lightweight startup scan if needed for overview data. Manual rescan must be available.
- After clicking scan, show an inserted/covered progress view with smooth progress updates, then replace it with selectable scan results and cleanup/optimization actions.
- Prefer card/page layouts over classic table views. If lists are needed, each row should be one coherent interactive row, not separate animated column blocks.
- Keep all content visible inside the window. If content cannot fit, scroll only inside the relevant card/list area, not the whole app in a confusing way.
- Show root usage as the parent total with distinct visual treatment; child cards show category/app/container breakdowns.
- Do not add explanatory title/description text where the data visualization itself is enough.
- Animations should be smooth and continuous without blocking UI work. User-visible status should update at least once per second during scans/actions.
- Use worker threads for scanning and actions so animations and pointer interaction stay responsive.
- Use a restrained ink-style visual direction with Apple-like clarity: layered cards, polished hover/press feedback, consistent spacing, and no low-quality decorative header strip.
- Error logs should be written to a dedicated file. The main UI should show concise failure state and the log path only when an operation fails.

## UI Verification

- GUI changes must be verified with a real desktop window, screenshots, and mouse interaction via `kylinos-v11-desktop-computer-use`.
- Before checking the latest UI, close old cleaner processes, launch the installed app with `QT_QPA_PLATFORM=xcb`, bring it to the front, then capture screenshots.
- Use real interaction for page changes and buttons. Do not rely only on offscreen launch tests.
- Keep visible UI language consistent with the selected language.
- For UKUI/Qt layout clipping, language combo-box visibility, and list hover behavior, also read `docs/qt-ukui-layout.md`.

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
