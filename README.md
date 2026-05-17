
<p align="center">
  <img src="https://raw.githubusercontent.com/Dronecode/UX-Design/35d8148a8a0559cd4bcf50bfa2c94614983cce91/QGC/Branding/Deliverables/QGC_RGB_Logo_Horizontal_Positive_PREFERRED/QGC_RGB_Logo_Horizontal_Positive_PREFERRED.svg" alt="QGroundControl Logo" width="500">
</p>

<p align="center">
  <a href="https://github.com/mavlink/QGroundControl/releases">
    <img src="https://img.shields.io/github/v/release/mavlink/QGroundControl" alt="Latest Release">
  </a>
</p>

*QGroundControl* (QGC) is a highly intuitive and powerful Ground Control Station (GCS) designed for UAVs. Whether you're a first-time pilot or an experienced professional, QGC provides a seamless user experience for flight control and mission planning, making it the go-to solution for any *MAVLink-enabled drone*.

---

### 🌟 *Why Choose QGroundControl?*

- *🚀 Ease of Use*: A beginner-friendly interface designed for smooth operation without sacrificing advanced features for pros.
- *✈️ Comprehensive Flight Control*: Full flight control and mission management for *PX4* and *ArduPilot* powered UAVs.
- *🛠️ Mission Planning*: Easily plan complex missions with a simple drag-and-drop interface.

🔍 For a deeper dive into using QGC, check out the [User Manual](https://docs.qgroundcontrol.com/en/) – although thanks to QGC's intuitive UI, you may not even need it!

---

### 🚁 *Key Features*

- 🕹️ *Full Flight Control*: Supports all *MAVLink drones*.
- ⚙️ *Vehicle Setup*: Tailored configuration for *PX4* and *ArduPilot* platforms.
- 🔧 *Fully Open Source*: Customize and extend the software to suit your needs.

🎯 Check out the latest updates in our [New Features and Release Notes](https://github.com/mavlink/qgroundcontrol/blob/master/CHANGELOG.md).

---

### 💻 *Get Involved!*

QGroundControl is *open-source*, meaning you have the power to shape it! Whether you're fixing bugs, adding features, or customizing for your specific needs, QGC welcomes contributions from the community.

🛠️ Start building today with our [Developer Guide](https://dev.qgroundcontrol.com/en/) and [build instructions](https://dev.qgroundcontrol.com/en/getting_started/).

---

### 🔗 *Useful Links*

- 🌐 [Official Website](http://qgroundcontrol.com)
- 📘 [User Manual](https://docs.qgroundcontrol.com/en/)
- 🛠️ [Developer Guide](https://dev.qgroundcontrol.com/en/)
- 💬 [Discussion & Support](https://docs.qgroundcontrol.com/en/Support/Support.html)
- 🤝 [Contributing](.github/CONTRIBUTING.md) ([Dev Guide](https://dev.qgroundcontrol.com/en/contribute/))
- 📜 [License Information](https://github.com/mavlink/qgroundcontrol/blob/master/.github/COPYING.md)

---

With QGroundControl, you're in full command of your UAV, ready to take your missions to the next level.

---

### Stargazers over time

[![Stargazers over time](https://starchart.cc/mavlink/qgroundcontrol.svg?variant=adaptive)](https://starchart.cc/mavlink/qgroundcontrol)

---

## Build Environment

Baseline build confirmed on macOS 15.7.1 (Apple Silicon / arm64).

| Component | Version |
|-----------|---------|
| macOS | 15.7.1 (BuildVersion 24G231) |
| Xcode toolchain | Apple clang 17.0.0 (Command Line Tools, clang-1700.0.13.5) |
| Qt | 6.10.3 (installed via aqtinstall into ~/Qt/6.10.3/macos) |
| CMake | 4.0.0 |
| Ninja | 1.13.2 |
| ccache | installed via Homebrew |

### Qt modules installed

Base Qt plus: `qt5compat qtgraphs qtlocation qtpositioning qtspeech qtmultimedia qtserialport qtimageformats qtshadertools qtconnectivity qtquick3d qtsensors qtscxml qtwebsockets qthttpserver`

### Build commands

```bash
# Install Qt 6.10.3 (one-time)
pip3 install aqtinstall
aqt install-qt mac desktop 6.10.3 clang_64 \
  --outputdir ~/Qt \
  --modules qt5compat qtgraphs qtlocation qtpositioning qtspeech qtmultimedia \
            qtserialport qtimageformats qtshadertools qtconnectivity qtquick3d \
            qtsensors qtscxml qtwebsockets qthttpserver

# Install Homebrew dependencies
brew install cmake ninja ccache just pkgconf create-dmg

# Generate Visvakarn app icon (.icns)
bash scripts/gen_icns.sh

# Configure (pass brand values to override CMake cache)
~/Qt/6.10.3/macos/bin/qt-cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DGStreamer_USE_FRAMEWORK=OFF \
  "-DQGC_APP_NAME=Visvakarn GCS" \
  "-DQGC_ORG_NAME=Visvakarn" \
  "-DQGC_ORG_DOMAIN=visvakarn.com" \
  "-DQGC_PACKAGE_NAME=com.visvakarn.gcs" \
  "-DQGC_APP_DESCRIPTION=Visvakarn Ground Control Station" \
  "-DQGC_MACOS_BUNDLE_ID=com.visvakarn.gcs" \
  "-DQGC_MACOS_ICON_PATH=$(pwd)/resources/icons/visvakarn.icns"

# Build (~40 min first time on Apple M-series)
cmake --build build --parallel
```

The resulting app bundle is at `build/Release/QGroundControl.app`.

## Build Patches

### Step 2 — Visvakarn GCS rebrand

No upstream source files were patched. All changes are additive:

- `cmake/CustomOptions.cmake` — app name, org, domain, bundle ID, icon path updated to Visvakarn values
- `CMakeLists.txt` — `project()` call hardcoded to `QGroundControl` (CMake target names cannot contain spaces); display name comes from `QGC_APP_NAME`
- `cmake/platform/Apple.cmake` — `MACOSX_BUNDLE_BUNDLE_NAME` changed from `CMAKE_PROJECT_NAME` to `QGC_APP_NAME` so the bundle shows "Visvakarn GCS"
- `src/QmlControls/QGCPalette.cc` — dark-theme colours updated to Visvakarn brand palette (#2B2B2B background, #73BAF2 accent, #E9E9E9 text)
- `src/UI/toolbar/*.qml`, `src/UI/MainWindow.qml`, `src/FlightMap/FlightMap.qml`, `src/AutoPilotPlugins/APM/APMFollowComponent.qml` — logo image references replaced with Visvakarn assets
- `src/UI/AppSettings/HelpSettings.qml` — QGroundControl doc URLs replaced with visvakarn.com/gcs
- `src/AutoPilotPlugins/Common/ESP8266Component.qml`, `src/GPS/NTRIP/NTRIPHttpTransport.cc`, `src/Vehicle/MAVLinkLogManager.{h,cc}`, `src/Camera/SimulatedCameraControl.h`, `src/Utilities/Parsing/Json/JsonParsing.cc` — user-visible "QGroundControl" strings replaced
- `resources/SplashScreen.png` — replaced with Visvakarn logo on #2B2B2B background
- `resources/icons/visvakarn.icns` — generated from `assets/visvakarn/Visvakarn_app-icon.png`
- `resources/Visvakarn*.{png,svg}` — brand logo assets added
- `qgcresources.qrc` — Visvakarn logo assets registered
- `scripts/gen_icns.sh` — icon generation script added
- `-DGStreamer_USE_FRAMEWORK=OFF` required on reconfigure (CPM-downloaded GStreamer uses pkg-config, not a system framework)
