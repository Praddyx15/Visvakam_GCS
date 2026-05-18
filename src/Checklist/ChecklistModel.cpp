#include "ChecklistModel.h"

#include "MultiVehicleManager.h"
#include "SettingsManager.h"
#include "AppSettings.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtQml/QQmlEngine>

QGC_LOGGING_CATEGORY(ChecklistModelLog, "Checklist.ChecklistModel")

Q_APPLICATION_STATIC(ChecklistModel, _singletonInstance)

static constexpr const char* kDefaultResourcePath = ":/resources/checklists/default.json";

// ---------------------------------------------------------------------------

ChecklistModel* ChecklistModel::create(QQmlEngine* engine, QJSEngine*)
{
    ChecklistModel* model = instance();
    // Tell the QML engine not to manage the lifetime — C++ owns this object.
    engine->setObjectOwnership(model, QQmlEngine::CppOwnership);
    return model;
}

ChecklistModel* ChecklistModel::instance()
{
    return _singletonInstance();
}

ChecklistModel::ChecklistModel(QObject* parent)
    : QAbstractListModel(parent)
{
    qCDebug(ChecklistModelLog) << this;
}

// ---------------------------------------------------------------------------
// QAbstractListModel

int ChecklistModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return _items.size();
}

QVariant ChecklistModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= _items.size())
        return {};

    const Item& item = _items.at(index.row());
    switch (role) {
    case IdRole:          return item.id;
    case SectionRole:     return item.section;
    case LabelRole:       return item.label;
    case DescriptionRole: return item.description;
    case SeverityRole:    return item.severity;
    case CheckedRole:     return item.checked;
    default:              return {};
    }
}

QHash<int, QByteArray> ChecklistModel::roleNames() const
{
    return {
        { IdRole,          "id"          },
        { SectionRole,     "section"     },
        { LabelRole,       "label"       },
        { DescriptionRole, "description" },
        { SeverityRole,    "severity"    },
        { CheckedRole,     "checked"     },
    };
}

// ---------------------------------------------------------------------------
// Public invokables

bool ChecklistModel::allBlockingChecked() const
{
    for (const Item& item : _items) {
        if (item.severity == QLatin1String("blocking") && !item.checked)
            return false;
    }
    return true;
}

void ChecklistModel::setChecked(const QString& id, bool checked)
{
    const bool wasReady = allBlockingChecked();
    for (int i = 0; i < _items.size(); ++i) {
        if (_items[i].id == id) {
            if (_items[i].checked == checked) return;
            _items[i].checked = checked;
            const QModelIndex idx = createIndex(i, 0);
            emit dataChanged(idx, idx, { CheckedRole });
            break;
        }
    }
    if (allBlockingChecked() != wasReady)
        emit readyToArmChanged();
}

void ChecklistModel::reset()
{
    const bool wasReady = allBlockingChecked();
    for (Item& item : _items)
        item.checked = false;
    if (!_items.isEmpty()) {
        emit dataChanged(createIndex(0, 0), createIndex(_items.size() - 1, 0), { CheckedRole });
    }
    if (allBlockingChecked() != wasReady)
        emit readyToArmChanged();
}

void ChecklistModel::loadFromFile(const QString& path)
{
    if (path.isEmpty()) {
        _loadDefault();
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(ChecklistModelLog) << "Cannot open checklist file:" << path;
        if (path != QLatin1String(kDefaultResourcePath)) {
            qCWarning(ChecklistModelLog) << "Falling back to default checklist";
            _loadDefault();
        }
        return;
    }
    _parseJson(file.readAll(), path);
}

// ---------------------------------------------------------------------------
// Initialisation

void ChecklistModel::init()
{
    // Load custom path from settings if set, otherwise load embedded default.
    const QString custom = SettingsManager::instance()->appSettings()->checklistCustomPath()->rawValue().toString();
    if (!custom.isEmpty() && QFile::exists(custom)) {
        loadFromFile(custom);
    } else {
        _loadDefault();
    }

    // React to changes in the custom path setting at runtime.
    connect(SettingsManager::instance()->appSettings()->checklistCustomPath(), &Fact::rawValueChanged,
            this, [this](QVariant value) {
                const QString path = value.toString();
                if (path.isEmpty()) {
                    _loadDefault();
                } else {
                    loadFromFile(path);
                }
            });

    // Reset checklist on each disarm.
    connect(MultiVehicleManager::instance(), &MultiVehicleManager::vehicleAdded,
            this, &ChecklistModel::_onVehicleAdded);

    const QList<Vehicle*>& vehicles = MultiVehicleManager::instance()->vehicles();
    for (Vehicle* vehicle : vehicles) {
        _onVehicleAdded(vehicle);
    }
}

void ChecklistModel::_onVehicleAdded(Vehicle* vehicle)
{
    connect(vehicle, &Vehicle::armedChanged, this, &ChecklistModel::_onArmedChanged);
}

void ChecklistModel::_onArmedChanged(bool armed)
{
    if (!armed)
        reset();
}

// ---------------------------------------------------------------------------
// JSON parsing

void ChecklistModel::_loadDefault()
{
    QFile file{QLatin1String{kDefaultResourcePath}};
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(ChecklistModelLog) << "Cannot open embedded default checklist";
        return;
    }
    _parseJson(file.readAll(), QLatin1String{kDefaultResourcePath});
}

void ChecklistModel::_parseJson(const QByteArray& bytes, const QString& sourcePath)
{
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError) {
        qCWarning(ChecklistModelLog) << "JSON parse error in" << sourcePath << ":" << err.errorString();
        if (sourcePath != QLatin1String(kDefaultResourcePath)) {
            qCWarning(ChecklistModelLog) << "Falling back to default checklist";
            _loadDefault();
        }
        return;
    }

    QList<Item> items;
    const QJsonArray sections = doc.object().value(QLatin1String("sections")).toArray();
    for (const QJsonValue& sv : sections) {
        const QJsonObject sec   = sv.toObject();
        const QString sectionId = sec.value(QLatin1String("id")).toString();
        const QString title     = sec.value(QLatin1String("title")).toString();
        const QJsonArray secItems = sec.value(QLatin1String("items")).toArray();
        for (const QJsonValue& iv : secItems) {
            const QJsonObject obj = iv.toObject();
            Item item;
            item.id          = obj.value(QLatin1String("id")).toString();
            item.section     = title.isEmpty() ? sectionId : title;
            item.label       = obj.value(QLatin1String("label")).toString();
            item.description = obj.value(QLatin1String("description")).toString();
            item.severity    = obj.value(QLatin1String("severity")).toString();
            item.checked     = false;
            items.append(item);
        }
    }

    if (items.isEmpty() && sourcePath != QLatin1String(kDefaultResourcePath)) {
        qCWarning(ChecklistModelLog) << "No checklist items parsed from custom JSON in" << sourcePath;
        qCWarning(ChecklistModelLog) << "Falling back to default checklist";
        _loadDefault();
        return;
    }

    beginResetModel();
    _items = std::move(items);
    endResetModel();
    emit readyToArmChanged();

    qCDebug(ChecklistModelLog) << "Loaded" << _items.size() << "items from" << sourcePath;
}
