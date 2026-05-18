# ============================================================================
# Checklist Module — Pre-Flight Checklist C++ model and QML view
# ============================================================================

# C++ sources go on the main executable target (same pattern as QmlControls).
target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        ChecklistModel.cpp
        ChecklistModel.h
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# QML module — PreFlightView and any future checklist QML components.
qt_add_library(ChecklistModule STATIC)

qt_add_qml_module(ChecklistModule
    URI QGroundControl.Checklist
    VERSION 1.0
    RESOURCE_PREFIX /qml
    QML_FILES
        PreFlightView.qml
    NO_PLUGIN
)
