Build in a controlled Docker environment to match target glibc

This folder contains `Dockerfile.build` — a simple image to build the `uavchip-auth` binary
with an environment whose glibc is close to common target distributions. Use this when the
host toolchain/libstdc++ cause GLIBC symbol mismatches on the target device.

Quick usage:

1. Build the image (run from `mvp/client`):

```bash
docker build -t xuanyu-build -f Dockerfile.build .
```

2. Run an interactive container (source mounted):

```bash
docker run --rm -it -v "$(pwd)":/src -w /src xuanyu-build
```

3. Inside the container you can run the build steps manually or the default build has already
   attempted to run CMake. To re-run explicitly:

```bash
cmake -S . -B build -DTARGET_PLATFORM=arm
cmake --build build -j$(nproc) --target uavchip-auth
```

4. Verify the produced binary's glibc dependencies (run on the build host or inside container):

```bash
readelf -V build/bin/arm/uavchip-auth | grep GLIBC_ | sort -u
```

Notes:
- If your target uses a specific distro/version, pick that as the base image (e.g. `ubuntu:22.04`).
- For embedded boards it's best to copy the board rootfs to the build container and use a
  CMake toolchain file that sets `CMAKE_SYSROOT` to that rootfs.
