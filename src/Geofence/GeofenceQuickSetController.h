#pragma once

#include <QtCore/QObject>
#include <QtPositioning/QGeoCoordinate>
#include <QtQmlIntegration/QtQmlIntegration>

class QQmlEngine;
class QJSEngine;
class GeoFenceManager;

class GeofenceQuickSetController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool   geofenceActive READ geofenceActive NOTIFY geofenceActiveChanged)
    Q_PROPERTY(double centerLat      READ centerLat      NOTIFY geofenceActiveChanged)
    Q_PROPERTY(double centerLon      READ centerLon      NOTIFY geofenceActiveChanged)
    Q_PROPERTY(double radiusM        READ radiusM        NOTIFY geofenceActiveChanged)

public:
    static GeofenceQuickSetController* create(QQmlEngine* engine, QJSEngine* scriptEngine);
    static GeofenceQuickSetController* instance();

    /// Upload a circular inclusion geofence centred on the active vehicle.
    /// @param radiusM  Fence radius in metres (clamped to [50, 5000])
    /// @param maxAltM  Breach-return altitude in metres AGL (used as breach-return point altitude)
    Q_INVOKABLE void applyCircularGeofence(double radiusM, double maxAltM);

    bool   geofenceActive() const { return _geofenceActive; }
    double centerLat()      const { return _centerLat; }
    double centerLon()      const { return _centerLon; }
    double radiusM()        const { return _radiusM; }

    // Public so Q_APPLICATION_STATIC can construct the singleton.
    // Use GeofenceQuickSetController::instance() to obtain the singleton.
    explicit GeofenceQuickSetController(QObject* parent = nullptr);

signals:
    void geofenceActiveChanged();
    void errorOccurred(QString message);

private:

    void _onSendComplete(bool error);
    void _onFenceLoaded();
    void _onActiveVehicleChanged();
    void _connectToVehicleFenceManager();

    bool             _geofenceActive    = false;
    double           _centerLat         = 0.0;
    double           _centerLon         = 0.0;
    double           _radiusM           = 0.0;
    GeoFenceManager* _connectedManager  = nullptr;
};
