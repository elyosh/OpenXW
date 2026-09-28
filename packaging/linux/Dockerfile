# Microsoft's Linux DXC release binaries are available for x86-64 only.
ARG XW_BUILD_PLATFORM=linux/amd64

FROM --platform=${XW_BUILD_PLATFORM} debian:bookworm-slim AS build-base

RUN apt-get update && apt-get install -y --no-install-recommends \
        autoconf \
        automake \
        build-essential \
        ca-certificates \
        cmake \
        file \
        git \
        libasound2-dev \
        libdbus-1-dev \
        libdecor-0-dev \
        libdrm-dev \
        libegl1-mesa-dev \
        libgbm-dev \
        libglib2.0-dev \
        libgl-dev \
        libibus-1.0-dev \
        libncurses-dev \
        libtool \
        libpipewire-0.3-dev \
        libpulse-dev \
        libudev-dev \
        liburing-dev \
        libvulkan-dev \
        libwayland-dev \
        libx11-dev \
        libxcursor-dev \
        libxext-dev \
        libxfixes-dev \
        libxi-dev \
        libxkbcommon-dev \
        libxml2-dev \
        libxrandr-dev \
        libxss-dev \
        libxtst-dev \
        nasm \
        ninja-build \
        patchelf \
        pkg-config \
        python3 \
        wayland-protocols \
        xz-utils \
        zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

ARG SDL_VERSION=3.4.14
ARG ZSTD_VERSION=1.5.7
ARG FFMPEG_VERSION=8.0.1
ARG FLUIDSYNTH_VERSION=2.4.8
ARG SPEEXDSP_VERSION=1.2.1
ARG SHADERCROSS_COMMIT=6b06e55c7c5d7e7a09a8a14f76e866dcfad5ab99

# The dependencies build as independent stages so that a version bump only
# rebuilds the affected dependency and so that cold builds run concurrently.

FROM build-base AS sdl-build

RUN git clone --branch "release-${SDL_VERSION}" --depth 1 \
        https://github.com/libsdl-org/SDL.git /deps/SDL \
    && cmake -S /deps/SDL -B /build/sdl -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/xw-linux \
        -DSDL_INSTALL_DOCS=OFF \
        -DSDL_SHARED=ON \
        -DSDL_STATIC=OFF \
        -DSDL_TESTS=OFF \
    && cmake --build /build/sdl --parallel \
    && cmake --install /build/sdl

FROM build-base AS zstd-build

RUN git clone --branch "v${ZSTD_VERSION}" --depth 1 \
        https://github.com/facebook/zstd.git /deps/zstd \
    && cmake -S /deps/zstd/build/cmake -B /build/zstd -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/xw-linux \
        -DZSTD_BUILD_PROGRAMS=OFF \
        -DZSTD_BUILD_SHARED=ON \
        -DZSTD_BUILD_STATIC=OFF \
        -DZSTD_BUILD_TESTS=OFF \
    && cmake --build /build/zstd --parallel \
    && cmake --install /build/zstd

FROM build-base AS ffmpeg-build

RUN git clone --branch "n${FFMPEG_VERSION}" --depth 1 \
        https://github.com/FFmpeg/FFmpeg.git /deps/FFmpeg \
    && cd /deps/FFmpeg \
    && ./configure \
        --prefix=/opt/xw-linux \
        --disable-autodetect \
        --disable-avdevice \
        --disable-avfilter \
        --disable-swscale \
        --disable-debug \
        --disable-doc \
        --disable-everything \
        --disable-network \
        --disable-programs \
        --disable-static \
        --enable-decoder=pcm_u8,pcm_s16le,vorbis \
        --enable-parser=vorbis \
        --enable-demuxer=ogg,wav \
        --enable-pic \
        --enable-pthreads \
        --enable-shared \
    && for feature in VORBIS_DECODER \
        PCM_U8_DECODER PCM_S16LE_DECODER OGG_DEMUXER WAV_DEMUXER; do \
        grep -q "^#define CONFIG_${feature} 1$" config_components.h || exit 1; \
    done \
    && make -j"$(nproc)" \
    && make install

FROM build-base AS speexdsp-build

RUN git clone --branch "SpeexDSP-${SPEEXDSP_VERSION}" --depth 1 \
        https://github.com/xiph/speexdsp.git /deps/speexdsp \
    && cd /deps/speexdsp \
    && ./autogen.sh \
    && ./configure \
        --prefix=/opt/xw-linux \
        --disable-static \
        --enable-shared \
    && make -j"$(nproc)" \
    && make install

FROM build-base AS fluidsynth-build

RUN git clone --branch "v${FLUIDSYNTH_VERSION}" --depth 1 \
        https://github.com/FluidSynth/fluidsynth.git /deps/fluidsynth \
    && cmake -S /deps/fluidsynth -B /build/fluidsynth -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/xw-linux \
        -DBUILD_SHARED_LIBS=ON \
        -Denable-alsa=OFF \
        -Denable-aufile=OFF \
        -Denable-dbus=OFF \
        -Denable-ipv6=OFF \
        -Denable-jack=OFF \
        -Denable-ladspa=OFF \
        -Denable-libinstpatch=OFF \
        -Denable-libsndfile=OFF \
        -Denable-midishare=OFF \
        -Denable-network=OFF \
        -Denable-openmp=OFF \
        -Denable-oss=OFF \
        -Denable-pipewire=OFF \
        -Denable-pulseaudio=OFF \
        -Denable-readline=OFF \
        -Denable-sdl2=OFF \
        -Denable-sdl3=OFF \
        -Denable-systemd=OFF \
    && cmake --build /build/fluidsynth --parallel \
    && cmake --install /build/fluidsynth

FROM build-base AS shader-tools

COPY packaging/common/shadercross-fsr3.patch /tmp/shadercross-fsr3.patch

# DXC 1.9 requires glibc 2.38; keep the Debian 12-compatible 1.8 release.
ARG DXC_VERSION=1.8.2502
ARG DXC_LINUX_X64_ARCHIVE=linux_dxc_2025_02_20.x86_64.tar.gz
ARG DXC_LINUX_X64_SHA256=e0580d90dbf6053a783ddd8d5153285f0606e5deaad17a7a6452f03acdf88c71

ADD --checksum=sha256:${DXC_LINUX_X64_SHA256} \
    https://github.com/microsoft/DirectXShaderCompiler/releases/download/v${DXC_VERSION}/${DXC_LINUX_X64_ARCHIVE} \
    /tmp/dxc.tar.gz

# shadercross links SDL but needs neither zstd nor FFmpeg.
COPY --from=sdl-build /opt/xw-linux /opt/xw-linux

RUN git clone https://github.com/libsdl-org/SDL_shadercross.git /deps/SDL_shadercross \
    && git -C /deps/SDL_shadercross checkout "${SHADERCROSS_COMMIT}" \
    && git -C /deps/SDL_shadercross apply /tmp/shadercross-fsr3.patch \
    && git -C /deps/SDL_shadercross submodule update --init --depth 1 \
        external/SPIRV-Cross \
    && mkdir -p \
        /deps/SDL_shadercross/external/DirectXShaderCompiler-binaries/linux \
    && tar -xzf /tmp/dxc.tar.gz \
        -C /deps/SDL_shadercross/external/DirectXShaderCompiler-binaries/linux \
    && cmake -S /deps/SDL_shadercross/external/SPIRV-Cross \
        -B /build/spirv-cross -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/xw-tools \
        -DSPIRV_CROSS_CLI=OFF \
        -DSPIRV_CROSS_ENABLE_TESTS=OFF \
        -DSPIRV_CROSS_SHARED=ON \
        -DSPIRV_CROSS_STATIC=OFF \
    && cmake --build /build/spirv-cross --parallel \
    && cmake --install /build/spirv-cross \
    && cmake -S /deps/SDL_shadercross -B /build/shadercross -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/xw-tools \
        -DCMAKE_PREFIX_PATH="/opt/xw-linux;/opt/xw-tools" \
        -DDirectXShaderCompiler_ROOT=/deps/SDL_shadercross/external/DirectXShaderCompiler-binaries \
        -DSDLSHADERCROSS_CLI=ON \
        -DSDLSHADERCROSS_DXC=ON \
        -DSDLSHADERCROSS_INSTALL=ON \
        -DSDLSHADERCROSS_INSTALL_RUNTIME=OFF \
        -DSDLSHADERCROSS_SHARED=ON \
        -DSDLSHADERCROSS_STATIC=OFF \
        -DSDLSHADERCROSS_SPIRVCROSS_SHARED=ON \
        -DSDLSHADERCROSS_VENDORED=OFF \
    && cmake --build /build/shadercross --parallel \
    && cmake --install /build/shadercross \
    && cp /deps/SDL_shadercross/external/DirectXShaderCompiler-binaries/linux/lib/libdxcompiler.so \
        /deps/SDL_shadercross/external/DirectXShaderCompiler-binaries/linux/lib/libdxil.so \
        /opt/xw-tools/lib/

FROM build-base AS xw-build

ARG XW_BUILD_TYPE=Release
ARG XW_VERSION=0.0.0-dev

COPY --from=sdl-build /opt/xw-linux /opt/xw-linux
COPY --from=zstd-build /opt/xw-linux /opt/xw-linux
COPY --from=ffmpeg-build /opt/xw-linux /opt/xw-linux
COPY --from=fluidsynth-build /opt/xw-linux /opt/xw-linux
COPY --from=speexdsp-build /opt/xw-linux /opt/xw-linux
COPY --from=sdl-build /deps/SDL/LICENSE.txt /opt/xw-licenses/SDL3.txt
COPY --from=zstd-build /deps/zstd/LICENSE /opt/xw-licenses/zstd.txt
COPY --from=ffmpeg-build /deps/FFmpeg/COPYING.LGPLv2.1 /opt/xw-licenses/FFmpeg-LGPL-2.1.txt
COPY --from=fluidsynth-build /deps/fluidsynth/LICENSE /opt/xw-licenses/FluidSynth-LGPL-2.1.txt
COPY --from=speexdsp-build /deps/speexdsp/COPYING /opt/xw-licenses/SpeexDSP-BSD-3-Clause.txt
COPY --from=shader-tools /opt/xw-tools /opt/xw-tools
COPY CMakeLists.txt /src/CMakeLists.txt
COPY README.md /src/README.md
COPY aeron /src/aeron
COPY cmake /src/cmake
COPY packaging /src/packaging
COPY resources /src/resources
COPY shaders /src/shaders
COPY src /src/src

ENV LD_LIBRARY_PATH=/opt/xw-tools/lib:/opt/xw-linux/lib
ENV PKG_CONFIG_PATH=/opt/xw-linux/lib/pkgconfig

RUN case "${XW_BUILD_TYPE}" in \
        Release) xw_debug_ui=OFF ;; \
        Debug) xw_debug_ui=ON ;; \
        *) echo "XW_BUILD_TYPE must be Release or Debug" >&2; exit 2 ;; \
    esac \
    && cmake -S /src -B /build/xw -G Ninja \
        -DCMAKE_BUILD_TYPE="${XW_BUILD_TYPE}" \
        -DCMAKE_PREFIX_PATH=/opt/xw-linux \
        -DAERON_SHADERCROSS_EXECUTABLE=/opt/xw-tools/bin/shadercross \
        -DIMUSE_ENABLE_FLUIDSYNTH=ON \
        -DIMUSE_ENABLE_NUKED_SC55=ON \
        -DXW_REQUIRE_MIDI_BACKENDS=ON \
        -DXW_BUILD_TOOLS=OFF \
        -DXW_ENABLE_ASAN=OFF \
        -DXW_ENABLE_UBSAN=OFF \
        -DXW_ENABLE_DEBUG_UI="${xw_debug_ui}" \
        -DXW_VERSION="${XW_VERSION}" \
    && cmake --build /build/xw --target xw --parallel

RUN cmake --install /build/xw --component Runtime --prefix /build/package/openxw-linux-x86_64 \
    && mkdir -p /build/package/openxw-linux-x86_64/licenses /build/package/openxw-linux-x86_64/lib \
    && cp /opt/xw-licenses/SDL3.txt \
        /build/package/openxw-linux-x86_64/licenses/SDL3.txt \
    && cp /opt/xw-licenses/zstd.txt \
        /build/package/openxw-linux-x86_64/licenses/zstd.txt \
    && cp /opt/xw-licenses/FFmpeg-LGPL-2.1.txt \
        /build/package/openxw-linux-x86_64/licenses/FFmpeg-LGPL-2.1.txt \
    && cp /opt/xw-licenses/FluidSynth-LGPL-2.1.txt \
        /build/package/openxw-linux-x86_64/licenses/FluidSynth-LGPL-2.1.txt \
    && cp /opt/xw-licenses/SpeexDSP-BSD-3-Clause.txt \
        /build/package/openxw-linux-x86_64/licenses/SpeexDSP-BSD-3-Clause.txt \
    && cp -L /opt/xw-linux/lib/libSDL3.so.0 \
        /build/package/openxw-linux-x86_64/lib/ \
    && cp -L /opt/xw-linux/lib/libavcodec.so.62 \
        /build/package/openxw-linux-x86_64/lib/ \
    && cp -L /opt/xw-linux/lib/libavformat.so.62 \
        /build/package/openxw-linux-x86_64/lib/ \
    && cp -L /opt/xw-linux/lib/libavutil.so.60 \
        /build/package/openxw-linux-x86_64/lib/ \
    && cp -L /opt/xw-linux/lib/libswresample.so.6 \
        /build/package/openxw-linux-x86_64/lib/ \
    && cp -L /opt/xw-linux/lib/libzstd.so.1 \
        /build/package/openxw-linux-x86_64/lib/ \
    && cp -L /opt/xw-linux/lib/libfluidsynth.so.3 \
        /build/package/openxw-linux-x86_64/lib/ \
    && cp -L /opt/xw-linux/lib/libspeexdsp.so.1 \
        /build/package/openxw-linux-x86_64/lib/ \
    && patchelf --set-rpath '$ORIGIN/lib' \
        /build/package/openxw-linux-x86_64/OpenXW \
    && for library in /build/package/openxw-linux-x86_64/lib/*.so*; do \
        patchelf --set-rpath '$ORIGIN' "$library"; \
    done

RUN test -f /build/package/openxw-linux-x86_64/shaders/aeron_dx5_projected_triangle.vert.spv \
    && test -f /build/package/openxw-linux-x86_64/resources/config.yaml \
    && test -f /build/package/openxw-linux-x86_64/resources/aeron/scene3d_defaults.yaml \
    && test -f /build/package/openxw-linux-x86_64/resources/opt_alpha_overrides.yaml \
    && test -f /build/package/openxw-linux-x86_64/shaders/dos_mesh.frag.spv \
    && test -f /build/package/openxw-linux-x86_64/shaders/dos_sprite.frag.spv \
    && test -f /build/package/openxw-linux-x86_64/shaders/sky_stars.frag.spv \
    && test -f /build/package/openxw-linux-x86_64/shaders/hyperspace_tunnel.frag.spv \
    && test -f /build/package/openxw-linux-x86_64/shaders/hyperspace_environment.comp.spv \
    && test -f /build/package/openxw-linux-x86_64/resources/ui-theme.yaml \
    && test -f /build/package/openxw-linux-x86_64/resources/assets/fonts/menu.fnt \
    && test -f /build/package/openxw-linux-x86_64/resources/assets/fonts/menu.png \
    && file /build/package/openxw-linux-x86_64/OpenXW \
    && env -u LD_LIBRARY_PATH ldd /build/package/openxw-linux-x86_64/OpenXW \
    && ! env -u LD_LIBRARY_PATH \
        ldd /build/package/openxw-linux-x86_64/OpenXW | grep -q "not found" \
    # The GCC runtime must resolve from the host, never from the package: the
    # host's Mesa Vulkan ICDs dlopen into the process and bind whichever
    # libstdc++ is already loaded, so a bundled (older) copy breaks Vulkan
    # device enumeration on newer distros.
    && ! env -u LD_LIBRARY_PATH \
        ldd /build/package/openxw-linux-x86_64/OpenXW \
        | grep -E 'libstdc[+][+]|libgcc_s' | grep -q '/build/package' \
    # Host baseline contract: glibc 2.35 and the GCC 12 runtime (Ubuntu 22.04
    # era). Fail packaging if any shipped object requires newer symbol
    # versions, or the glibc-2.36-only DT_RELR ABI, so a toolchain or
    # dependency bump cannot silently raise the floor.
    && for object in /build/package/openxw-linux-x86_64/OpenXW \
        /build/package/openxw-linux-x86_64/lib/*.so*; do \
        if readelf -V "$object" | grep -q 'GLIBC_ABI_'; then \
            echo "$object requires a GLIBC_ABI_* version (DT_RELR needs glibc 2.36+)" >&2; \
            exit 1; \
        fi; \
        for bound in GLIBC_2.35 GLIBCXX_3.4.30 CXXABI_1.3.13; do \
            required=$(readelf -V "$object" \
                | grep -oE "${bound%_*}_[0-9][0-9.]*" | sort -uV | tail -n 1); \
            if [ -n "$required" ] && \
                [ "$(printf '%s\n%s\n' "$required" "$bound" | sort -V | tail -n 1)" != "$bound" ]; then \
                echo "$object requires $required, above the $bound host baseline" >&2; \
                exit 1; \
            fi; \
        done; \
    done \
    && if [ "${XW_BUILD_TYPE}" = Debug ]; then \
        readelf --sections /build/package/openxw-linux-x86_64/OpenXW \
            | grep -q '\.debug_info'; \
    fi \
    && if [ "${XW_BUILD_TYPE}" = Debug ]; then \
        artifact_variant=debug; \
    else \
        artifact_variant=release; \
    fi \
    && package_name="openxw-${XW_VERSION}-linux-x86_64-${artifact_variant}" \
    && mv /build/package/openxw-linux-x86_64 "/build/package/${package_name}" \
    && mkdir -p /build/artifacts \
    && tar \
        --sort=name \
        --mtime="@0" \
        --owner=0 \
        --group=0 \
        --numeric-owner \
        -C /build/package \
        -cJf "/build/artifacts/${package_name}.tar.xz" \
        "${package_name}" \
    && tar -tJf "/build/artifacts/${package_name}.tar.xz"

FROM scratch AS artifact
COPY --from=xw-build /build/artifacts/ /
