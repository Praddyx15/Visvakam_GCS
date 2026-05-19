#!/usr/bin/env bash
set -e

# ==============================================================================
# Visvakarn GCS — macOS Packaging and Delivery Script
# ==============================================================================
# This script automates the complete configuration, compilation, Qt bundling,
# ad-hoc code-signing, and DMG creation for Visvakarn GCS under macOS.
# ==============================================================================

echo "🚀 Starting Visvakarn GCS macOS Packaging & Build..."

# 1. Setup variables
QT_DIR="${HOME}/Qt/6.10.3/macos"
BUILD_DIR="build_release"
CMAKE_BIN="${QT_DIR}/bin/qt-cmake"

if [ ! -d "${QT_DIR}" ]; then
    echo "❌ Error: Qt 6.10.3 macOS directory not found at ${QT_DIR}"
    echo "Please ensure Qt is installed using: aqt install-qt mac desktop 6.10.3"
    exit 1
fi

# 2. Clean previous build if requested
if [ "$1" == "--clean" ]; then
    echo "🧹 Cleaning previous build directory..."
    rm -rf "${BUILD_DIR}"
fi

# 3. Generate Visvakarn icon sets
echo "🎨 Generating app icon sets..."
bash scripts/gen_icns.sh

# 4. Run CMake Configuration with brand values
echo "🛠️ Configuring CMake with Visvakarn brand identity..."
"${CMAKE_BIN}" -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DGStreamer_USE_FRAMEWORK=OFF \
  -DQGC_MACOS_SIGN_WITH_IDENTITY=ON \
  "-DQGC_APP_NAME=Visvakarn GCS" \
  "-DQGC_ORG_NAME=Visvakarn" \
  "-DQGC_ORG_DOMAIN=visvakarn.com" \
  "-DQGC_PACKAGE_NAME=com.visvakarn.gcs" \
  "-DQGC_APP_DESCRIPTION=Visvakarn Ground Control Station" \
  "-DQGC_MACOS_BUNDLE_ID=com.visvakarn.gcs" \
  "-DQGC_MACOS_ICON_PATH=$(pwd)/resources/icons/visvakarn.icns"

# 5. Compile the application
echo "⚡ Building Visvakarn GCS application..."
cmake --build "${BUILD_DIR}" --parallel

# 6. Bundle Qt/QML frameworks, ad-hoc sign, and create DMG package
echo "📦 Running Mac deployment and creating signed DMG..."
# Note: The CMake install target automatically runs:
#   - macdeployqt to bundle all Qt frameworks and QML dependencies
#   - codesign -s - --entitlements ... to ad-hoc sign the app bundle
#   - create-dmg to bundle the app in a drag-and-drop installer
cmake --install "${BUILD_DIR}" --config Release

echo "🎉 Visvakarn GCS macOS Packaging completed successfully!"
echo "📍 The signed drag-and-drop installer DMG is ready inside: ${BUILD_DIR}/"
