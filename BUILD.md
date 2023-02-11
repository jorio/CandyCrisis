# How to build Candy Crisis from source

## macOS

Prerequisites: Xcode, git, cmake (install via homebrew)

Build recipe:

```bash
git clone https://github.com/jorio/CandyCrisis
cd CandyCrisis

curl -LO https://github.com/libsdl-org/SDL/releases/download/release-2.26.3/SDL2-2.26.3.dmg
hdiutil attach SDL2-*.dmg
cp -a /Volumes/SDL2/SDL2.framework SDL2.framework
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

Invoke-WebRequest -OutFile SDL2-VC.zip -Uri https://github.com/libsdl-org/SDL/releases/download/release-2.26.3/SDL2-devel-2.26.3-VC.zip
Expand-Archive SDL2-VC.zip
move SDL2-VC/SDL2-* SDL2

cmake -S . -B build -G 'Visual Studio 17 2022'
cmake --build build --config Release
```

## Linux

Prerequisites: a decent C compiler, git, cmake, and your platform's SDL2 development package

```bash
git clone https://github.com/jorio/CandyCrisis
cd CandyCrisis
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Or, if you want to build SDL from source and link it statically:

```bash
git clone https://github.com/jorio/CandyCrisis
cd CandyCrisis

git clone --depth 1 --branch release-2.26.3 https://github.com/libsdl-org/SDL

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SDL_FROM_SOURCE=1 -DSTATIC_SDL=1
cmake --build build
```
