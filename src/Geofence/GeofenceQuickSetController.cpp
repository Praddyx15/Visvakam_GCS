#include "GeofenceQuickSetController.h"

#include "GeoFenceManager.h"
#include "MultiVehicleManager.h"
#include "QGCFenceCircle.h"
#include "QmlObjectListModel.h"
#include "Vehicle.h"

#include <QtCore/QCoreApplication>
#include <QtQml/QQmlEngine>

Q_APPLICATION_STATIC(GeofenceQuickSetController, _singletonInstance)

GeofenceQuickSetController* GeofenceQuickSetController::create(QQmlEngine* engine, QJSEngine*)
{
    GeofenceQuickSetController* controller = instance();
    engine->setObjectOwnership(controller, QQmlEngine::CppOwnership);
    return controller;
}

GeofenceQuickSetController* GeofenceQuickSetController::instance()
{
    return _singletonInstance();
}

GeofenceQuickSetController::GeofenceQuickSetController(QObject* parent)
    : QObject(parent)
{
    connect(MultiVehicleManager::instance(), &MultiVehicleManager::activeVehicleChanged,
            this, &GeofenceQuickSetController::_onActiveVehicleChanged);
    _onActiveVehicleChanged();
}

void GeofenceQuickSetController::_onActiveVehicleChanged()
{
    if (_connectedManager) {
        _connectedManager->disconnect(this);
        _connectedManager = nullptr;
    }

    Vehicle* vehicle = MultiVehicleManager::instance()->activeVehicle();
    if (!vehicle) {
        if (_geofenceActive) {
            _geofenceActive = false;
            emit geofenceActiveChanged();
        }
        return;
    }

    _connectedManager = vehicle->geoFenceManager();
    connect(_connectedManager, &GeoFenceManager::loadComplete,
            this, &GeofenceQuickSetController::_onFenceLoaded);
}

void GeofenceQuickSetController::_onFenceLoaded()
{
    if (!_geofenceActive || !_connectedManager) {
        return;
    }
    // If the vehicle's fence is now empty, our quick-set ring is gone
    if (_connectedManager->circles().isEmpty() && _connectedManager->polygons().isEmpty()) {
        _geofenceActive = false;
        emit geofenceActiveChanged();
    }
}

void GeofenceQuickSetController::applyCircularGeofence(double radiusM, double maxAltM)
{
    Vehicle* vehicle = MultiVehicleManager::instance()->activeVehicle();
    if (!vehicle) {
        emit errorOccurred(tr("No active vehicle connected"));
        return;
    }

    GeoFenceManager* manager = vehicle->geoFenceManager();
    if (!manager->supported()) {
        emit errorOccurred(tr("Vehicle does not support geofencing"));
        return;
    }

    if (manager->inProgress()) {
        emit errorOccurred(tr("A geofence operation is already in progress"));
        return;
    }

    const QGeoCoordinate center = vehicle->coordinate();
    if (!center.isValid()) {
        emit errorOccurred(tr("Vehicle position is not available"));
        return;
    }

    // Clamp inputs
    radiusM = qBound(50.0, radiusM, 5000.0);
    maxAltM = qBound(30.0, maxAltM, 500.0);

    // Store pending overlay state immediately for visual feedback
    _centerLat = center.latitude();
    _centerLon = center.longitude();
    _radiusM   = radiusM;

    // Build temporary models — sendToVehicle copies data before returning
    QmlObjectListModel circles;
    QmlObjectListModel polygons;
    QGCFenceCircle circle(center, radiusM, true /* inclusion */);
    circles.append(&circle);

    // Breach return point carries the max altitude
    QGeoCoordinate breachReturn(center.latitude(), center.longitude(), maxAltM);

    // Qt::SingleShotConnection ensures we disconnect automatically after one fire
    connect(manager, &GeoFenceManager::sendComplete,
            this, &GeofenceQuickSetController::_onSendComplete,
            Qt::SingleShotConnection);

    manager->sendToVehicle(breachReturn, polygons, circles);
}

void GeofenceQuickSetController::_onSendComplete(bool error)
{
    if (error) {
        emit errorOccurred(tr("Geofence upload failed — check vehicle connection"));
        return;
    }

    _geofenceActive = true;
    emit geofenceActiveChanged();
}
