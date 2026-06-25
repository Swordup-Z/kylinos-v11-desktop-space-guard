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
- Root partition usage labels must make clear that the number comes from the `/` filesystem and excludes separately mounted `/home` and `/data` when those are separate filesystems.
- Application containers are an app-level breakdown inside Kaiming usage; they do not equal total Kaiming usage.
- If a user asks why root space changed, compare `/`, `/home`, and `/data` mount points first so DATA/HOME content is not misattributed to root.
- Startup/preheat management must support both disable and restore. It must show affected entries and intended changes before applying them.
- Do not label space cleanup as application uninstall. Uninstalling apps is application management; cleanup is about verified old or unused container/layer data.

## UI Requirements

- Default UI language is Simplified Chinese. Provide a Chinese/English switch; `zh_CN` shows Chinese names and `en_US` shows English names.
- Chinese desktop name: `麒麟V11空间清理`. English desktop name should clearly reflect KylinOS V11 desktop space cleanup.
- The application/taskbar icon must remain recognizable at small taskbar size. Avoid gray system-tool or gear-like artwork; use a distinctive colorful cleanup/storage symbol and install matching PNG/SVG icons for the executable's desktop class.
- The user has authorized using CleanMyMac's official visual design as the primary reference for this project. Match its major interaction and visual language closely for this app's own KylinOS cleanup functions: dark purple Smart Care-style workspace, glassy selectable task cards, prominent glowing circular primary action, task review affordances, animated scanning focus, clear completed/running states, and polished left/navigation/tool surfaces.
- Treat the CleanMyMac reference as the target for first-screen colors, selected-state colors, task-card hierarchy, page/module switching, and transition animation timing/easing. Do not settle for a generic purple theme, ordinary tab-strip hierarchy, hard cuts, or unrelated white report panels on Smart Care surfaces.
- The app window should look like a single CleanMyMac-style application card: no system-menu-bar feel, no separate floating title strip, and no fragmented panels outside the main shell. Implement app-level window controls and page function buttons inside the unified card.
- Keep the default app window compact while preserving readable text size. Prefer reducing the default/minimum window bounds and using internal scroll areas over shrinking fonts.
- Right-top window controls should be a classic transparent glass control card with clear minimize, maximize/restore, and close symbols. Do not use barely visible glyphs or force macOS traffic-light dots when the current design calls for classic controls.
- The app background must be a dynamic glass surface, not a static flat gradient. Outer shell, sidebar, header, and content cards must derive their glass color from the same active page theme.
- The top fixed/header panel must stay visually tied to the active page theme. Use transparent glass tint and themed border; do not give it an independent fixed dark or purple color.
- Content cards below the header should use transparent glass surfaces with light tint and borders. Avoid heavy dark filled panels unless a specific state needs strong contrast.
- Do not leave a visible black frame outside the unified app card. If the window is frameless, the visible application should be the card itself or a transparent window background.
- Left navigation icons must be useful feature entries only. Their selected highlight box and icon drawing must be centered against the sidebar's real visual center; do not rely on font glyph metrics or system icons that create visual right/left bias.
- Left navigation should not include a separate "scan result" icon when old-container cleanup and startup/preheat optimization already enter the result/review workflow. Keep navigation entries tied to distinct user-facing functions.
- No page may ship with overlapping text, buttons, charts, cards, scrollbars, or status lines. Verify screenshots after every layout change, especially scan result cards, top status, and left navigation.
- Old-container cleanup and preheat/autostart optimization pages must keep their status value blocks wide and consistent enough to avoid wrapping normal Chinese status text. Their summary cards must not place animated visuals behind or above text where they can collide with labels or buttons.
- Do not copy CleanMyMac product copy into user-facing KylinOS functionality. Replace content, metrics, actions, safety language, and execution results with KylinOS V11 space-cleaning concepts such as old Kaiming containers, startup/preheat entries, root usage, app containers, KARE, and ostree writable-layer visibility.
- The first screen is a clean overview, not an instruction page. The second primary card/page is scanning.
- Scanning starts only when the user triggers it, except for one initial lightweight startup scan if needed for overview data. Manual rescan must be available.
- The fixed top/header panel must include a prominent manual rescan button. Users must be able to refresh scan data after startup without navigating into a result page.
- The top manual rescan button must use the active page theme's glass/accent color, not a fixed purple or unrelated primary color.
- After clicking scan, show an inserted/covered progress view with smooth progress updates, then replace it with selectable scan results and cleanup/optimization actions.
- Prefer card/page layouts over classic table views. If lists are needed, each row should be one coherent interactive row, not separate animated column blocks.
- Keep all content visible inside the window. If content cannot fit, scroll only inside the relevant card/list area, not the whole app in a confusing way.
- Show root usage as the parent total with distinct visual treatment; child cards show category/app/container breakdowns.
- Root usage pie slice labels should show only concrete size and percentage, for example `8.5 GiB · 26.3%`. Category names stay in the legend and rows to keep the chart uncluttered.
- Do not explain large root-usage sectors with a separate prose label when a visual breakdown is possible. Split broad "other root usage" into meaningful visible chart categories such as KARE base, WPS/apps, and system/cache.
- Do not add explanatory title/description text where the data visualization itself is enough.
- Animations should be smooth and continuous without blocking UI work. User-visible status should update at least once per second during scans/actions.
- Page switching must not reduce perceived sharpness. Do not apply whole-page opacity or slide effects to primary navigation transitions if they make text/cards look dim, blurry, or unstable after clicks.
- Switching between the application-container and old-container/navigation pages must not expose a black fallback background. Prefer instant theme application with local button feedback over unstable cross-page color interpolation.
- Use worker threads for scanning and actions so animations and pointer interaction stay responsive.
- Use a restrained ink-style visual direction with Apple-like clarity: layered cards, polished hover/press feedback, consistent spacing, and no low-quality decorative header strip.
- Error logs should be written to a dedicated file. The main UI should show concise failure state and the log path only when an operation fails.

## UI Verification

- GUI changes must be verified with a real desktop window, screenshots, and mouse interaction via `kylinos-v11-desktop-computer-use`.
- If `kylinos-v11-desktop-computer-use` lacks a capability required for verification, such as screenshot capture, scrolling, right-click, or keyboard input, update and verify that helper first, then return to this project.
- Before checking the latest UI, close old cleaner processes, launch the installed app with `QT_QPA_PLATFORM=xcb`, bring it to the front, then capture screenshots.
- Use real interaction for page changes, primary buttons, checkbox selection, right-click context menus, scrolling, language switching, and rescan/apply flows. Do not rely only on offscreen launch tests.
- Keep visible UI language consistent with the selected language.
- For UKUI/Qt layout clipping, language combo-box visibility, and list hover behavior, also read `docs/qt-ukui-layout.md`.

## Commands

```bash
git status --short --branch
make
make install
cmake -S . -B build-codex -G Ninja
cmake --build build-codex
cpack --config build-codex/CPackConfig.cmake -G DEB
env QT_QPA_PLATFORM=xcb ~/.local/share/kylin-space-guard/bin/kylin-space-cleaner
~/.local/bin/kylinos-v11-desktop-computer-use click <x> <y>
```

## Packaging

- The distributable application format is a Debian package. Keep CMake install rules and CPack DEB metadata current when adding binaries, helpers, desktop files, or icons.
- The `.deb` must install the Qt GUI to `/usr/bin`, the helper to `/usr/libexec`, the desktop entry to `/usr/share/applications`, and hicolor PNG/SVG icons under `/usr/share/icons/hicolor`.

## Git

- Use author `Swordup-Z <swordup.zeng@gmail.com>`.
- Commit project changes after verification.
- This repository currently has no remote; do not assume push is available until the user provides a GitHub repository URL.
