# How to build Candy Crisis from source

## macOS

Prerequisites: Xcode, git, cmake (install via homebrew)

Build recipe:

```bash
git clone https://github.com/jorio/CandyCrisis
cd CandyCrisis

curl -LO https://github.com/libsdl-org/SDL/releases/download/release-3.2.0/SDL3-3.2.0.dmg
hdiutil attach SDL3-*.dmg
cp -a /Volumes/SDL3/SDL3.xcframework/macos-arm64_x86_64/SDL3.framework SDL2.framework
hdiutil detach /Volumes/SDL2

cmake -S . -B build -G Xcode
cmake --build build --config Release
```

## Windows

Prerequisites: Visual Studio 2022, git, cmake

Build recipe (to run in PowerShell):
```bash
git clone https://github.com/jorio/CandyCrisis
cd CandyCrisis

Invoke-WebRequest -OutFile SDL3-VC.zip -Uri https://github.com/libsdl-org/SDL/releases/download/release-3.2.0/SDL3-devel-3.2.0-VC.zip
Expand-Archive SDL3-VC.zip
move SDL3-VC/SDL3-* SDL3

cmake -S . -B build -G 'Visual Studio 17 2022'
cmake --build build --config Release
```

## Linux

Prerequisites: a decent C compiler, git, cmake, and your platform's SDL3 development package

```bash
git clone https://github.com/jorio/CandyCrisis
cd CandyCrisis
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Or, if you want to build SDL3 from source and link it statically:

```bash
git clone https://github.com/jorio/CandyCrisis
cd CandyCrisis

git clone --depth 1 --branch release-3.2.0 https://github.com/libsdl-org/SDL

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SDL_FROM_SOURCE=1 -DSDL_STATIC=1
cmake --build build
```
