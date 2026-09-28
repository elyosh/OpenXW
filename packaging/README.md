# Building OpenXW packages

| Target | Graphics | Artifact | CI |
|---|---|---|---|
| macOS arm64 | Metal | `.app` in a DMG | Yes |
| macOS x86_64 | Metal | `.app` in a DMG | No |
| Windows x86_64 | D3D12 and Vulkan | ZIP | Yes |
| Linux x86_64 | Vulkan | tar.xz | Yes |

- Output directory: `build/artifacts`.
- Runtime libraries: SDL3, zstd, FFmpeg, FluidSynth and SpeexDSP.
- MIDI: FluidSynth and Nuked SC-55 are required in distributable packages.
- FFmpeg formats: Ogg/Vorbis and WAV.
- Debug: ImGui overlay enabled; macOS includes `OpenXW.dSYM`.
- Release: ImGui overlay disabled.
- Package builds disable ASAN and UBSan, including Debug packages.
- Original game data is not included.

All targets stage the application and Aeron shaders, including DOS meshes/sprites,
the shader starfield, hyperspace and FSR, alongside `resources/config.yaml`,
`resources/aeron/scene3d_defaults.yaml` and `resources/opt_alpha_overrides.yaml`.
macOS uses MSL, Linux SPIR-V, and Windows includes both DXIL and SPIR-V.
No baked skybox is required or bundled. Models and cockpit artwork are loaded
from the user's original installations.

## All platforms

On a Mac with Docker Buildx (`linux/amd64` support) and the macOS prerequisites
below, run `./packaging/build-packages.sh`. It builds Linux, Windows, and native
macOS packages sequentially into `build/artifacts`. Submodules must already be
initialized; the wrapper preserves existing checkouts and source edits.
`XW_VERSION` defaults to the short commit hash and `XW_BUILD_TYPE` to `Release`.

## macOS

Requirements: Xcode command-line tools, CMake, Ninja, pkg-config, Autoconf,
Automake, Libtool and GLib (FluidSynth's dependency).

```sh
brew install autoconf automake cmake glib libtool ninja pkg-config
```

Initialize submodules with `git submodule update --init --recursive`.
Run on the target architecture. Build variants sequentially; they share caches.

```sh
XW_VERSION=0.0.0-dev ./packaging/macos/build-package.sh
XW_BUILD_TYPE=Debug ./packaging/macos/build-package.sh
```

| Environment variable | Default |
|---|---|
| `XW_VERSION` | `0.0.0-dev` |
| `XW_BUILD_TYPE` | `Release` |
| `XW_MACOS_ARCHITECTURE` | Host architecture |
| `XW_MACOS_DEPLOYMENT_TARGET` | `13.0` |
| `XW_MACOS_BUILD_ROOT` | `build/cache/macos-<arch>` |
| `XW_MACOS_ARTIFACT_DIR` | `build/artifacts` |
| `XW_MACOS_SHADERCROSS_EXECUTABLE` | Build the pinned shader compiler |
| `XW_MACOS_BUILD_VERSION` | `1` |
| `XW_MACOS_BUNDLE_IDENTIFIER` | `org.totallyopen.openxw` |
| `XW_MACOS_SIGN_IDENTITY` | `-` (ad hoc) |

Compiler parallelism: `sysctl -n hw.logicalcpu`.
Bundled dylibs are relocated into `Contents/Frameworks`.

### Signing and notarization

Set `XW_MACOS_SIGN_IDENTITY` to a Developer ID identity and
`XW_MACOS_NOTARY_PROFILE` to a notarytool keychain profile.
Alternatively, supply all three direct-credential variables:
`XW_MACOS_NOTARY_APPLE_ID`, `XW_MACOS_NOTARY_TEAM_ID` and
`XW_MACOS_NOTARY_PASSWORD`.

```sh
./packaging/macos/sign-package.sh input.dmg output.dmg
```

`XW_MACOS_RELEASE_TAG` optionally replaces the signed DMG on a GitHub release.

## Linux and Windows

Requirements: Docker Buildx with `linux/amd64` support. Run from the repository
root. ARM64 Docker hosts require amd64 emulation.

```sh
docker buildx build --platform linux/amd64 -f packaging/linux/Dockerfile \
  --build-arg XW_BUILD_TYPE=Release \
  --target artifact --output type=local,dest=build/artifacts .
docker buildx build --platform linux/amd64 -f packaging/windows/Dockerfile \
  --build-arg XW_BUILD_TYPE=Release \
  --target artifact --output type=local,dest=build/artifacts .
```

Use `--build-arg XW_BUILD_TYPE=Debug` to build Debug packages.

| Build argument | Default |
|---|---|
| `XW_VERSION` | `0.0.0-dev` |
| `XW_BUILD_TYPE` | `Release` |

- Build environment: Debian Bookworm; Windows uses MinGW-w64.
- CMake parallelism: build-tool default; FFmpeg: `make -j"$(nproc)"`.
- BuildKit may run dependency stages concurrently.
- Linux: libraries in `lib/`, executable RPATH `$ORIGIN/lib`, library RPATH `$ORIGIN`.
- Linux host requirements: glibc 2.35+, GCC 12-compatible runtimes, graphics/audio drivers.
- Linux FluidSynth also uses the host's GLib/GThread runtime libraries.
- Windows: runtime DLLs beside the executable; Debug includes WinPixEventRuntime.
- A plain non-macOS `cmake --install --component Runtime` stages application files;
  Docker packaging adds runtime dependencies and their licenses.

The MIDI dependency recipes are adapted from OpenTIE, while Nuked SC-55 is
compiled from OpenXW's own iMUSE submodule. Linux and macOS build FluidSynth
2.4.8 and SpeexDSP 1.2.1 from source. Windows uses OpenTIE's pinned vcpkg
`2025.10.17` registry and library-only FluidSynth patches, including the
SpeexDSP runtime DLL name correction. Packages set `XW_REQUIRE_MIDI_BACKENDS=ON`
so missing dependencies fail configuration instead of producing silent stubs.

## CI

| Workflow | Trigger | Configurations | Output |
|---|---|---|---|
| `ci.yml` | Main push | Release and Debug | Workflow artifacts |
| `ci.yml` | Pull request or manual | Release | Workflow artifacts |
| `release.yml` | `v*` tag | Release and Debug | Draft GitHub release |

Package checks cover game configuration, selected shaders, required FFmpeg
codecs, Linux runtime resolution and ABI versions, Windows executable subsystem
and Debug symbols, and macOS plist syntax, dylib paths, signatures and dSYM UUIDs.
CI does not run gameplay tests or notarization.

## Icons and development installs

The [icon generator](icons/README.md) produces the committed Windows, macOS and
window icons from OpenXvT's unchanged placeholder artwork. Builds need no image
tools. Regenerate with `python3 packaging/icons/icon_tools.py` when replacing it.

A normal macOS CMake build produces `build/OpenXW.app`; installing the Runtime
component additionally copies its linked non-system libraries into the bundle.
Use the package script for distributable artifacts: it builds the pinned minimal
dependencies, checks relocatability and signs the result. Installing a local build
linked against Homebrew can pull in Homebrew's larger transitive dependency set.

The build directory's loose pre-step-8 `OpenXW` executable is obsolete. Launch
`OpenXW.app/Contents/MacOS/OpenXW` or the app bundle on macOS.
