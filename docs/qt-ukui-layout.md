# Qt/UKUI Application Layout

## Scope

- Kylin Space Guard UI development on KylinOS Desktop V11 / UKUI.
- Rows, cards, or list entries are clipped by fixed heights or compressed layouts.
- Language combo boxes or popup lists become unreadable under UKUI theme colors.

## Diagnostics

- Check whether a list/table is compressed by the parent layout until only the header or first row is visible.
- Avoid placing too much content directly in one vertical layout when the window height is constrained.
- Check for overly small fixed row heights.
- Be careful with `ResizeToContents`, word wrap, and automatic row resizing under UKUI themes and high DPI; Qt can miscalculate row heights and create huge gaps or clipped rows.
- Long app IDs, versions, paths, and Chinese labels need either enough width, elision, tooltips, or an in-card detail view.
- Do not rely on UKUI theme defaults for combo-box foreground/background colors.

## Implementation Rules

- Prefer card/list views over classic tables for user-facing scan results.
- If a table is still necessary:
  - Use `setWordWrap(false)` for long paths and app IDs.
  - Use fixed, readable row heights such as 48 or 50 px.
  - Use column-specific sizing instead of making every column `Stretch`.
  - Add tooltips for long values.
- Put dense content inside the relevant card's scroll area, not a confusing whole-window scroll.
- Implement hover and selection as one coherent row highlight. Avoid per-cell hover blocks.
- Explicitly style combo boxes and popup views:
  - current field background, text color, and border
  - popup background and text color
  - selected row background and selected text color
- Set adequate minimum widths and use `QComboBox::AdjustToContents` where language labels differ in length.

## Verification

```bash
cmake -S . -B build -G Ninja
cmake --build build
timeout 8s env QT_QPA_PLATFORM=offscreen ./build/<app>
```

Offscreen launch only catches crashes. Real clipping, hover feedback, language visibility, and popup consistency must be checked on the UKUI desktop with screenshots and mouse interaction.
