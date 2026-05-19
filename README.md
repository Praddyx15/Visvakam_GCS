# Visvakarn GCS (Ground Control Station)

Visvakarn GCS is a customized, rebranded fork of [QGroundControl](https://github.com/mavlink/qgroundcontrol) tailored for fixed-wing UAV field operations. It introduces new capabilities for telemetry logging, pre-flight safety validation, and rapid geofencing.

---

## Key Feature Additions

### 1. Telemetry CSV Recorder (Feature A)
Saves flight telemetry to standard CSV files for post-flight analysis (e.g., in Excel or Pandas) without requiring binary log extraction.
- **Auto-Logging**: Starts logging automatically when the vehicle is armed and stops 30 seconds after the vehicle is disarmed (capturing critical post-landing telemetry).
- **Format**: Filename format follows `<vehicle_id>_<UTC_timestamp>.csv` (e.g., `1_2026-05-14T093015Z.csv`) saved in `~/Documents/Visvakarn GCS/Logs/`.
- **Downsampling**: Telemetry is written at a steady 5 Hz rate, capturing the latest available state values.
- **Robustness**: Handles absent telemetry fields (e.g., NaN battery readings) gracefully, outputting blank fields to maintain clean CSV structure.
- **UX**: A "Show in Finder" (on macOS) or "Show in Explorer" (on Windows) button is provided directly next to the toggle in the Application Settings.

### 2. Customizable Pre-Flight Checklist (Feature B)
Enforces a structured checklist walk-through for field operators before flight.
- **Section Grouping**: Grouped into three distinct stages: *Airframe*, *Avionics*, and *Mission*.
- **Configurability**: Loads a default checklist from `resources/checklists/default.json` at startup. Allows admins to load a custom JSON checklist via a file picker in the application settings.
- **Arming Status Badge**: Features a visual "Pre-Flight Incomplete" (Red) / "Ready to Arm" (Green) status badge on the main Fly View HUD.
- **Auto-Reset**: Checklist status automatically resets to unchecked upon vehicle disarming, ensuring a fresh safety checklist is completed for every single flight.

### 3. Geofence Quick-Set (Feature C)
Allows field operators to quickly configure a circular inclusion geofence centered on the vehicle's current coordinates.
- **Input Parameters**: A clean HUD-integrated dialog lets operators configure the geofence Radius (50m to 5000m) and Maximum Altitude AGL (30m to 500m).
- **HUD Map Ring**: Displays a translucent, high-visibility boundary ring on the map once active.
- **MAVLink Integration**: Uploads the geofence directly to the autopilot using the standard MAVLink protocol. Any upload failures are captured and displayed to the user via the GCS toast notification system.

---

## Architecture & Implementation Decisions

Each feature is integrated directly into QGC's Qt Quick/C++ architecture:
- **Separation of Concerns**: Custom modules are built as static modules (`ChecklistModule` and `GeofenceModule`) under `src/Checklist/` and `src/Geofence/` and linked via CMake.
- **QML Singleton Registry**: Exposes C++ controllers (`ChecklistModel` and `GeofenceQuickSetController`) as singletons to the QML engine, keeping UI layouts clean and responsive.
- **Cross-Platform Pathing**: Standardized file URL path conversions (stripping prefixes like `file:///` on Windows or keeping absolute paths intact on UNIX-based systems) to avoid platform-specific pathing bugs.

### Technical Trade-offs Made
1. **Downsampled Telemetry Logging vs. Full Stream Capture**:
   Instead of writing every single incoming MAVLink message to disk, the CSV Recorder downsamples telemetry to a steady 5 Hz. This reduces disk I/O load, avoids freezing the main thread, and results in uniform, clean time-series logs.
2. **GCS-Side Arm Gating vs. Autopilot Gating**:
   The Pre-Flight checklist acts as a visual safety gate on the GCS interface ("Ready to Arm" badge). It does not block the low-level MAVLink arm command. This prevents locking the pilot out of emergency control states if the GCS loses state mid-operation.
3. **Local Geofence Overlay Rendering**:
   The translucent geofence circle on the HUD map is rendered using local parameters immediately after a successful upload trigger. This eliminates the need to constantly poll the autopilot for fence boundaries, saving valuable telemetry bandwidth.

---

## Build and Packaging Instructions

### Environment Prerequisites
- **macOS**: macOS 14+ (ARM64 Apple Silicon)
- **Xcode**: Xcode 15+ toolchain (Clang)
- **Qt**: Qt 6.10.3 (desktop macos, `aqtinstall` is recommended)
- **Dependencies**: CMake 3.28+, Ninja, `create-dmg` (installed via Homebrew)

### Local Build Commands
To compile the application locally:
```bash
# 1. Install Qt dependencies
pip3 install aqtinstall
aqt install-qt mac desktop 6.10.3 clang_64 --outputdir ~/Qt --modules qt5compat qtgraphs qtlocation qtpositioning qtspeech qtmultimedia qtserialport qtimageformats qtshadertools qtconnectivity qtquick3d qtsensors qtscxml qtwebsockets qthttpserver

# 2. Install package build tools
brew install cmake ninja create-dmg

# 3. Compile the application
~/Qt/6.10.3/macos/bin/qt-cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DGStreamer_USE_FRAMEWORK=OFF
cmake --build build --parallel
```

### Packaging Script
The packaging process is automated in `scripts/package_macos.sh`:
```bash
bash scripts/package_macos.sh
```
This script configures the project, builds the binaries, packages all Qt frameworks and QML plug-ins using `macdeployqt` into a standalone `.app` bundle, ad-hoc signs the binaries, and creates a drag-and-drop installer `.dmg` in the build directory.
