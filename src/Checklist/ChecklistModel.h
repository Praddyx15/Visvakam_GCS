#pragma once

#include <QtCore/QAbstractListModel>
#include <QtQmlIntegration/QtQmlIntegration>

class QQmlEngine;
class QJSEngine;
class Vehicle;

class ChecklistModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool readyToArm READ allBlockingChecked NOTIFY readyToArmChanged)

public:
    // QML singleton factory — returns the C++ singleton; QML does not own the pointer.
    static ChecklistModel* create(QQmlEngine* engine, QJSEngine* scriptEngine);
    static ChecklistModel* instance();

    void init();

    enum Role {
        IdRole          = Qt::UserRole + 1,
        SectionRole,
        LabelRole,
        DescriptionRole,
        SeverityRole,
        CheckedRole,
    };

    // QAbstractListModel
    int      rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool allBlockingChecked() const;
    Q_INVOKABLE void setChecked(const QString& id, bool checked);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void loadFromFile(const QString& path);

    // Public so Q_APPLICATION_STATIC can construct the singleton instance.
    // Use ChecklistModel::instance() to obtain the singleton.
    explicit ChecklistModel(QObject* parent = nullptr);

signals:
    void readyToArmChanged();

private:

    struct Item {
        QString id;
        QString section;
        QString label;
        QString description;
        QString severity;   // "blocking" | "warning"
        bool    checked = false;
    };

    void   _loadDefault();
    void   _parseJson(const QByteArray& bytes, const QString& sourcePath);
    void   _onVehicleAdded(Vehicle* vehicle);
    void   _onArmedChanged(bool armed);

    QList<Item> _items;
};
