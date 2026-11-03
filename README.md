# Omacap

A compact screenshot app for Omarchy and Hyprland, forked from
[Omasnap](https://github.com/omacom/omasnap) 1.24.0 (`6208946`).

![Omacap editor](assets/preview-editor.png)

Drag an area, click a window, or click empty space for the focused monitor.
Captures copy automatically; files are saved only when you choose Save. A small preview lets you edit on demand.

The editor uses a compact dock with select/crop, arrow, draw, shape, text,
redact, color, undo, and **Copy**. Extra tools and output actions live under
**…**. Buttons have 6 px gaps, wider group spacing, and padding around the dock.
Keyboard help opens with **?** or **… → Shortcuts**.
Capture notifications offer **Save** and **Edit**. Edit keeps Save under **…**.

Omacap has its own binary, icon, desktop entry, config, runtime locks, and
recent history. It runs alongside Omasnap and the unrelated window-snapping
plugin. Screen recording stays with Omarchy.

## Build and run

Requires Qt 6.8+, LayerShellQt, Wayland protocols, libdeflate, CMake, Ninja,
and a C++23 compiler. Runtime tools: `hyprctl`, `wl-copy`, `wl-paste`;
`tesseract` is needed only for OCR. See [dependencies](docs/dependencies.md).

```sh
make build
./build/omacap
./build/omacap --file /absolute/path/to/image.png
./build/omacap --capture-region --copy
```

`./install-omarchy` installs dependencies and the app under `~/.local`.
It does not change Hyprland bindings. `OMACAP_PREFIX` selects another prefix.

## Controls

| Key | Action |
| --- | --- |
| Esc | Cancel / close; dismiss an open More menu first |
| Enter | Copy only |
| V / A / F | Select / arrow / draw |
| R / E / T / D | Rectangle / ellipse / text / redact |
| Ctrl+Z / Ctrl+Shift+Z | Undo / redo |
| Ctrl+C / Ctrl+S | Copy only / save only |
| Ctrl+P | Pin |
| ? | Toggle shortcuts |

Optional INI config: `~/.config/omacap/omacap.conf`. Captures go to
`~/Pictures/Screenshots`; `OMACAP_SCREENSHOT_DIR` overrides the destination.
The upstream capture/editor flags remain available through `--help`.

## Checks and attribution

`make check` builds, runs the headless interaction suite, and runs available
static analysis. Generated screenshots stay in `build/smoke-output/`.

Original MIT copyright and bundled font/icon notices are retained. The
[changelog](CHANGELOG.md) records upstream history. The PNG metadata key
`Omasnap logical size` stays unchanged to preserve shared image-scale metadata.

The local Omarchy notification hover fix is preserved in
[`patches/omarchy-notifications-hover.patch`](patches/omarchy-notifications-hover.patch)
for the user-owned clone of `omarchy.notifications`.
