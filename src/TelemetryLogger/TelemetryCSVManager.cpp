#include "TelemetryCSVManager.h"

#include "TelemetryCSVRecorder.h"
#include "MultiVehicleManager.h"
#include "QmlObjectListModel.h"
#include "SettingsManager.h"
#include "AppSettings.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QCoreApplication>

QGC_LOGGING_CATEGORY(TelemetryCSVManagerLog, "TelemetryLogger.TelemetryCSVManager")

Q_APPLICATION_STATIC(TelemetryCSVManager, _instance)

TelemetryCSVManager::TelemetryCSVManager(QObject *parent)
    : QObject(parent)
{
    qCDebug(TelemetryCSVManagerLog) << this;
}

TelemetryCSVManager::~TelemetryCSVManager()
{
    _stopAllRecorders();
}

TelemetryCSVManager *TelemetryCSVManager::instance()
{
    return _instance();
}

void TelemetryCSVManager::init()
{
    MultiVehicleManager *mvm = MultiVehicleManager::instance();
    connect(mvm, &MultiVehicleManager::vehicleAdded,   this, &TelemetryCSVManager::_onVehicleAdded);
    connect(mvm, &MultiVehicleManager::vehicleRemoved, this, &TelemetryCSVManager::_onVehicleRemoved);

    Fact *settingFact = SettingsManager::instance()->appSettings()->recordTelemetryCSV();
    connect(settingFact, &Fact::rawValueChanged, this, &TelemetryCSVManager::_onSettingChanged);
}

void TelemetryCSVManager::_onVehicleAdded(Vehicle *vehicle)
{
    if (SettingsManager::instance()->appSettings()->recordTelemetryCSV()->rawValue().toBool())
        _startRecorder(vehicle);
}

void TelemetryCSVManager::_onVehicleRemoved(Vehicle *vehicle)
{
    _stopRecorder(vehicle->id());
}

void TelemetryCSVManager::_onSettingChanged(QVariant enabled)
{
    if (enabled.toBool()) {
        QmlObjectListModel *vehicles = MultiVehicleManager::instance()->vehicles();
        if (!vehicles)
            return;
        for (int i = 0; i < vehicles->count(); ++i) {
            Vehicle *v = qobject_cast<Vehicle*>((*vehicles)[i]);
            if (v && !_recorders.contains(v->id()))
                _startRecorder(v);
        }
    } else {
        _stopAllRecorders();
    }
}

void TelemetryCSVManager::_startRecorder(Vehicle *vehicle)
{
    const int vid = vehicle->id();
    if (_recorders.contains(vid)) {
        qCWarning(TelemetryCSVManagerLog) << "Recorder already exists for vehicle" << vid;
        return;
    }
    auto *recorder = new TelemetryCSVRecorder(vehicle, vehicle);
    _recorders.insert(vid, recorder);
    qCDebug(TelemetryCSVManagerLog) << "Started recorder for vehicle" << vid;
}

void TelemetryCSVManager::_stopRecorder(int vehicleId)
{
    TelemetryCSVRecorder *recorder = _recorders.take(vehicleId);
    if (!recorder)
        return;
    recorder->stopNow();
    recorder->deleteLater();
    qCDebug(TelemetryCSVManagerLog) << "Stopped recorder for vehicle" << vehicleId;
}

void TelemetryCSVManager::_stopAllRecorders()
{
    const QList<int> ids = _recorders.keys();
    for (int id : ids)
        _stopRecorder(id);
}
