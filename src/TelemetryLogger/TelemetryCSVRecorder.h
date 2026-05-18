#pragma once

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>

class Vehicle;

class TelemetryCSVRecorder : public QObject
{
    Q_OBJECT
public:
    explicit TelemetryCSVRecorder(Vehicle *vehicle, QObject *parent = nullptr);
    ~TelemetryCSVRecorder() override;

    void stopNow();

private slots:
    void _onArmedChanged(bool armed);
    void _writeSampleRow();
    void _onPostArmTimeout();

private:
    void _openFile();
    void _closeFile();

    Vehicle*        _vehicle        = nullptr;
    QFile           _file;
    QTextStream     _stream;
    QTimer          _sampleTimer;
    QTimer          _postArmTimer;
    bool            _fileOpen       = false;
};
