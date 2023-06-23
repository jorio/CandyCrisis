#! /usr/bin/env bash

set -e

EXE_PATH="$1"

[[ ! -f "${EXE_PATH}" ]] && echo "must pass valid exe path" && exit 1

APP_NAME="$(basename "${EXE_PATH}")"
APP_NAME_LOWERCASE="$(echo "$APP_NAME" | awk '{print tolower($0)}')"
APP_DIR="build/${APP_NAME}.AppDir"

mkdir -p "${APP_DIR}/usr/bin"
mkdir -p "${APP_DIR}/usr/share"

# Copy executable and assets
cp "${EXE_PATH}" "${APP_DIR}/usr/bin"
cp -r CandyCrisisResources "${APP_DIR}/usr/share/candycrisis"

# Copy XDG stuff
cp "packaging/${APP_NAME_LOWERCASE}.desktop" "${APP_DIR}"
cp "packaging/${APP_NAME_LOWERCASE}.png" "${APP_DIR}"

# Copy AppImage kicker script
cp "packaging/AppRun" "${APP_DIR}"
chmod 755 "${APP_DIR}/AppRun"
