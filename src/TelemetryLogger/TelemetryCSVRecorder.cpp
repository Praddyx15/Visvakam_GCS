#include "TelemetryCSVRecorder.h"

#include "Vehicle.h"
#include "VehicleGPSFactGroup.h"
#include "BatteryFactGroupListModel.h"
#include "QmlObjectListModel.h"
#include "QGCLoggingCategory.h"
#include "SettingsManager.h"
#include "AppSettings.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

QGC_LOGGING_CATEGORY(TelemetryCSVRecorderLog, "TelemetryLogger.TelemetryCSVRecorder")

static constexpr int kSampleIntervalMs  = 200;
static constexpr int kPostArmDelayMs    = 30000;

TelemetryCSVRecorder::TelemetryCSVRecorder(Vehicle *vehicle, QObject *parent)
    : QObject(parent)
    , _vehicle(vehicle)
{
    _sampleTimer.setInterval(kSampleIntervalMs);
    _sampleTimer.setSingleShot(false);
    connect(&_sampleTimer, &QTimer::timeout, this, &TelemetryCSVRecorder::_writeSampleRow);

    _postArmTimer.setSingleShot(true);
    connect(&_postArmTimer, &QTimer::timeout, this, &TelemetryCSVRecorder::_onPostArmTimeout);

    connect(vehicle, &Vehicle::armedChanged, this, &TelemetryCSVRecorder::_onArmedChanged);

    if (vehicle->armed()) {
        _openFile();
        _sampleTimer.start();
    }
}

TelemetryCSVRecorder::~TelemetryCSVRecorder()
{
    stopNow();
}

void TelemetryCSVRecorder::stopNow()
{
    _sampleTimer.stop();
    _postArmTimer.stop();
    _closeFile();
}

void TelemetryCSVRecorder::_openFile()
{
    const QString logsDir = SettingsManager::instance()->appSettings()->telemetryCSVSavePath();
    if (!QDir().mkpath(logsDir)) {
        qCWarning(TelemetryCSVRecorderLog) << "Failed to create logs directory:" << logsDir;
        return;
    }

    const QString timestamp = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMddTHHmmssZ"));
    const QString fileName  = QStringLiteral("%1_%2.csv").arg(_vehicle->id()).arg(timestamp);
    _file.setFileName(QDir(logsDir).filePath(fileName));

    if (!_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(TelemetryCSVRecorderLog) << "Failed to open CSV file:" << _file.fileName() << _file.errorString();
        return;
    }

    _stream.setDevice(&_file);
    _stream << QStringLiteral(
        "timestamp_utc,lat,lon,alt_amsl_m,alt_rel_m,roll_deg,pitch_deg,yaw_deg,"
        "ground_speed_mps,air_speed_mps,battery_voltage_v,battery_remaining_pct,"
        "flight_mode,arming_state,gps_fix_type,num_satellites\n");
    _stream.flush();
    _fileOpen = true;

    qCDebug(TelemetryCSVRecorderLog) << "Opened CSV log:" << _file.fileName();
}

void TelemetryCSVRecorder::_closeFile()
{
    if (!_fileOpen)
        return;
    _stream.flush();
    _file.close();
    _fileOpen = false;
    qCDebug(TelemetryCSVRecorderLog) << "Closed CSV log:" << _file.fileName();
}

void TelemetryCSVRecorder::_onArmedChanged(bool armed)
{
    if (armed) {
        _postArmTimer.stop();
        if (!_fileOpen) {
            _openFile();
        }
        if (!_sampleTimer.isActive())
            _sampleTimer.start();
    } else {
        _postArmTimer.start(kPostArmDelayMs);
    }
}

void TelemetryCSVRecorder::_onPostArmTimeout()
{
    _sampleTimer.stop();
    _closeFile();
}

void TelemetryCSVRecorder::_writeSampleRow()
{
    if (!_fileOpen)
        return;

    // Timestamp
    const QString ts = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);

    // Position
    const QGeoCoordinate coord = _vehicle->coordinate();
    const double lat = coord.latitude();
    const double lon = coord.longitude();
    const double altAmsl = _vehicle->altitudeAMSL()->rawValue().toDouble();
    const double altRel  = _vehicle->altitudeRelative()->rawValue().toDouble();

    // Attitude
    const double roll  = _vehicle->roll()->rawValue().toDouble();
    const double pitch = _vehicle->pitch()->rawValue().toDouble();
    const double yaw   = _vehicle->heading()->rawValue().toDouble();

    // Speed
    const double groundSpeed = _vehicle->groundSpeed()->rawValue().toDouble();
    const double airSpeed    = _vehicle->airSpeed()->rawValue().toDouble();

    // Battery (first battery, if present)
    double battVoltage   = qQNaN();
    double battRemaining = qQNaN();
    QmlObjectListModel *batteries = _vehicle->batteries();
    if (batteries && batteries->count() > 0) {
        BatteryFactGroup *bat = qobject_cast<BatteryFactGroup*>((*batteries)[0]);
        if (bat) {
            battVoltage   = bat->voltage()->rawValue().toDouble();
            battRemaining = bat->percentRemaining()->rawValue().toDouble();
        }
    }

    // Flight mode and arming state
    const QString flightMode  = _vehicle->flightMode();
    const QString armingState = _vehicle->armed() ? QStringLiteral("ARMED") : QStringLiteral("DISARMED");

    // GPS
    QString gpsFixType    = QStringLiteral("0");
    QString numSatellites = QStringLiteral("0");
    VehicleGPSFactGroup *gps = qobject_cast<VehicleGPSFactGroup*>(_vehicle->gpsFactGroup());
    if (gps) {
        const int lockIdx            = gps->lock()->rawValue().toInt();
        const QStringList lockStrs   = gps->lock()->enumStrings();
        gpsFixType = (lockIdx >= 0 && lockIdx < lockStrs.size())
                     ? lockStrs.at(lockIdx) : QString::number(lockIdx);
        numSatellites = QString::number(gps->count()->rawValue().toInt());
    }

    auto nanToStr = [](double v) -> QString {
        return qIsNaN(v) ? QStringLiteral("") : QString::number(v, 'f', 6);
    };

    _stream << ts << ','
            << nanToStr(lat) << ','
            << nanToStr(lon) << ','
            << QString::number(altAmsl, 'f', 3) << ','
            << QString::number(altRel,  'f', 3) << ','
            << QString::number(roll,    'f', 3) << ','
            << QString::number(pitch,   'f', 3) << ','
            << QString::number(yaw,     'f', 3) << ','
            << QString::number(groundSpeed, 'f', 3) << ','
            << QString::number(airSpeed,    'f', 3) << ','
            << nanToStr(battVoltage) << ','
            << nanToStr(battRemaining) << ','
            << flightMode << ','
            << armingState << ','
            << gpsFixType << ','
            << numSatellites << '\n';
    _stream.flush();
}
