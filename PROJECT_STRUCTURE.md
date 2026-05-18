# Visvakarn GCS — Project Structure

Fork of [QGroundControl](https://github.com/mavlink/qgroundcontrol) with three custom modules:
**Pre-Flight Checklist**, **Geofence Quick-Set**, and **Telemetry CSV Recorder**.

---

## Repository Layout

```
visvakarn-gcs/
├── CMakeLists.txt              # Top-level build configuration
├── qgcresources.qrc            # Embedded resources (icons, JSON, fonts)
├── src/                        # All application source code
├── resources/                  # Asset files (SVGs, audio, firmware, maps)
├── test/                       # Unit & integration tests
├── tools/                      # Dev utilities, analyzers, code generators
├── cmake/                      # CMake modules and helper scripts
├── docs/                       # Documentation
├── deploy/                     # Deployment configs (macOS, Linux, Windows)
├── translations/               # i18n .ts files (50+ languages)
├── android/                    # Android platform layer
└── custom-example/             # Template for custom plugin modules
```

---

## `src/` — Module Breakdown

### Flight & Control UI

| Module | Description |
|---|---|
| `FlyView/` | Primary flight view — HUD, toolbar, guided actions, video overlay |
| `PlanView/` | Mission planner UI — waypoints, rally points, survey corridors |
| `FlightMap/` | Shared map component used by both Fly and Plan views |
| `Viewer3D/` | 3D terrain and vehicle visualization |

### Vehicle & Autopilot

| Module | Description |
|---|---|
| `Vehicle/` | Core vehicle abstraction — state, parameters, telemetry |
| `AutoPilotPlugins/` | Firmware-specific config UIs (ArduPilot, PX4) |
| `FirmwarePlugin/` | Firmware abstraction layer (PX4 vs ArduPilot differences) |
| `MissionManager/` | Mission, geofence, and rally planning + MAVLink upload logic |

### Communication

| Module | Description |
|---|---|
| `Comms/` | Link management — serial, UDP, TCP, Bluetooth LE |
| `MAVLink/` | MAVLink protocol parsing and message handling |

### Sensors & Systems

| Module | Description |
|---|---|
| `Camera/` | Camera control and gimbal integration |
| `GPS/` | GPS positioning and NMEA parsing |
| `Gimbal/` | Gimbal stabilization QML controller |
| `ADSB/` | ADS-B traffic awareness |
| `Joystick/` | Gamepad and joystick input |
| `FollowMe/` | Follow-me mode logic |

### UI Components

| Module | Description |
|---|---|
| `QmlControls/` | Reusable QML widgets — buttons, dialogs, indicators |
| `UI/` | MainWindow, toolbars, settings views, app shell |
| `Settings/` | App settings model (Fact-based, persisted via QSettings) |

### Infrastructure

| Module | Description |
|---|---|
| `FactSystem/` | Typed parameter/property system used everywhere in QGC |
| `LogManager/` | Flight log loading and management |
| `PositionManager/` | GCS position tracking (GPS for the ground station itself) |
| `VideoManager/` | Video stream decoding and display |
| `AnalyzeView/` | Post-flight log analysis tools |
| `Terrain/` | Terrain elevation data fetching |
| `Utilities/` | Shared helpers — file I/O, math, compression |
| `QtLocationPlugin/` | Map tile provider plugin |
| `API/` | External service integrations |
| `Android/` | Android-specific platform code |

---

## Custom Additions (Visvakarn Fork)

These three modules are not present in upstream QGroundControl.

### `src/Checklist/` — Pre-Flight Checklist

```
Checklist/
├── CMakeLists.txt          # Registers ChecklistModule (QGroundControl.Checklist)
├── ChecklistModel.h/.cpp   # QAbstractListModel singleton — loads default.json
└── PreFlightView.qml       # Full-screen checklist UI with section grouping
```

- Items are loaded from `resources/checklists/default.json` (embedded via QRC)
- Accessible in QML as `ChecklistModel` (singleton under `QGC` module)
- `readyToArm` property drives the pre-flight badge in the Fly view toolbar

### `src/Geofence/` — Geofence Quick-Set

```
Geofence/
├── CMakeLists.txt                  # Registers GeofenceModule (QGroundControl.Geofence)
├── GeofenceQuickSetController.h/.cpp  # C++ singleton — uploads circular geofence
└── GeofenceQuickSet.qml            # Dialog — radius + max altitude inputs
```

- Opens from the Fly view tool strip ("Geofence" button)
- Uploads a circular inclusion geofence centered on the active vehicle's position
- Renders a translucent `MapCircle` overlay on the fly map while active

### `src/TelemetryLogger/` — Telemetry CSV Recorder

```
TelemetryLogger/
├── CMakeLists.txt
├── TelemetryCSVManager.h/.cpp   # C++ singleton — subscribes to vehicle telemetry
└── (no QML — runs silently in background)
```

- Starts recording automatically when a vehicle connects
- Saves CSV files to `~/Documents/Visvakarn GCS Daily/YYYY-MM-DD/`
- Columns: timestamp, lat, lon, altitude, heading, groundspeed, battery voltage

---

## Build System Patterns

### QML Module Pattern

Each UI subdirectory is its own QML module:

```cmake
qt_add_library(XModule STATIC)
qt_add_qml_module(XModule
    URI QGroundControl.X
    VERSION 1.0
    RESOURCE_PREFIX /qml
    QML_FILES Foo.qml Bar.qml
    NO_PLUGIN
)
```

C++ sources that expose QML types go directly on the main executable target:

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE MyClass.cpp MyClass.h)
```

The module library is then linked to the main executable in `src/CMakeLists.txt`.

### C++ QML Singleton Pattern

```cpp
class MyClass : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    static MyClass* create(QQmlEngine*, QJSEngine*);
    static MyClass* instance();
    explicit MyClass(QObject* parent = nullptr); // must be public for Q_APPLICATION_STATIC
};
```

```cpp
// .cpp
Q_APPLICATION_STATIC(MyClass, _singletonInstance)

MyClass* MyClass::create(QQmlEngine* engine, QJSEngine*) {
    engine->setObjectOwnership(instance(), QQmlEngine::CppOwnership);
    return instance();
}
MyClass* MyClass::instance() { return _singletonInstance(); }
```

### Key Conventions

| Convention | Rule |
|---|---|
| Sizing | Always use `ScreenTools.defaultFontPixelHeight/Width`, never hardcoded px |
| Colors | Always use `QGCPalette`, never hardcoded color values |
| Parameters | All vehicle parameters go through the Fact System (`vehicle->parameterManager()`) |
| Vehicle null-check | Always guard: `if (!vehicle) return;` before accessing active vehicle |
| Module URI | `QGroundControl.<ModuleName>` for all custom modules |

---

## Module URI Map

| Import | Module library | Contents |
|---|---|---|
| `import QGC` | main executable | Core C++ singletons (QGroundControl, SettingsManager, ChecklistModel, …) |
| `import QGroundControl` | `QGroundControlModule` | MainWindow.qml |
| `import QGroundControl.Controls` | `QGroundControlControlsModule` | Reusable QML widgets |
| `import QGroundControl.FlyView` | `FlyViewModule` | All Fly view QML |
| `import QGroundControl.FlightMap` | `FlightMapModule` | Map QML components |
| `import QGroundControl.PlanView` | `PlanViewModule` | All Plan view QML |
| `import QGroundControl.Checklist` | `ChecklistModule` | PreFlightView.qml |
| `import QGroundControl.Geofence` | `GeofenceModule` | GeofenceQuickSet.qml |
| `import QGroundControl.Toolbar` | `ToolbarModule` | Toolbar QML components |
