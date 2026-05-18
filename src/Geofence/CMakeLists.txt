# ============================================================================
# Geofence Quick-Set — circular geofence dialog and upload controller
# ============================================================================

# C++ sources compile into the main executable target
target_sources(${CMAKE_PROJECT_NAME}
    PRIVATE
        GeofenceQuickSetController.cpp
        GeofenceQuickSetController.h
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# QML module
qt_add_library(GeofenceModule STATIC)

qt_add_qml_module(GeofenceModule
    URI QGroundControl.Geofence
    VERSION 1.0
    RESOURCE_PREFIX /qml
    QML_FILES
        GeofenceQuickSet.qml
    NO_PLUGIN
)
