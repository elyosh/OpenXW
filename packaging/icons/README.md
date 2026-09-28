# Application icons

The master image is copied unchanged from OpenXvT as a placeholder. The generator
is adapted directly from OpenXvT and uses only Python's standard library.

| File | Role |
|---|---|
| `packaging/icons/openxw-icon.png` | 1024x1024 RGBA master; OpenXvT placeholder |
| `packaging/icons/icon_tools.py` | Generates all derived assets |
| `cmake/windows/openxw.ico` | Windows executable icon |
| `cmake/macos/OpenXW.icns` | macOS bundle icon |
| `src/xw_app/window_icon.h` | Embedded 64x64 BMP window/taskbar icon |

```sh
python3 packaging/icons/icon_tools.py
```

Commit the master and regenerated outputs together. Builds consume the committed
outputs and do not require Python or image tooling. The master must be a 1024x1024,
8-bit RGB/RGBA, non-interlaced PNG.

Windows resources embed the ICO. The macOS plist references the ICNS in the bundle.
`XwHostConfig_InitAeron` passes the embedded BMP through Aeron for Windows/Linux
window icons; macOS uses the bundle icon. No original game icon is packaged.
