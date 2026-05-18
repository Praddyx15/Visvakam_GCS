#pragma once

#include <QtCore/QHash>
#include <QtCore/QObject>

class Vehicle;
class TelemetryCSVRecorder;

class TelemetryCSVManager : public QObject
{
    Q_OBJECT
public:
    explicit TelemetryCSVManager(QObject *parent = nullptr);
    ~TelemetryCSVManager() override;

    static TelemetryCSVManager *instance();
    void init();

private slots:
    void _onVehicleAdded(Vehicle *vehicle);
    void _onVehicleRemoved(Vehicle *vehicle);
    void _onSettingChanged(QVariant enabled);

private:
    void _startRecorder(Vehicle *vehicle);
    void _stopRecorder(int vehicleId);
    void _stopAllRecorders();

    QHash<int, TelemetryCSVRecorder*> _recorders;
};
