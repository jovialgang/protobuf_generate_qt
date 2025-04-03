#include "abstract_settings.h"
#include "property_binder.h"

using namespace om;

namespace
{
// constexpr int kSaveDelay = 500;
}

// ObjectSettings
SettingsListener::SettingsListener(QObject *parent /*= nullptr*/) : PropertyChangeListener(parent)
{
    SetRoles(Role::ALL_ROLES);
}

QString SettingsListener::GetCategory() const
{
    return category_;
}

void SettingsListener::SetCategory(const QString &val)
{
    if (category_ == val)
        return;
    category_ = val;
    emit categoryChanged();
}

// Impl
class AbstractSettings::Impl
{
public:
    Impl(AbstractSettings *settings) : settings_(settings), listener_(new SettingsListener(settings)) {}

    void AddListener(SettingsListener *listener)
    {
        listeners_list_.push_back(listener);
        InstallListener(listener);
    }

    void RemoveListener(SettingsListener *listener)
    {
        UninstallListener(listener);
        listeners_list_.removeOne(listener);
    }

    void ClearListeners()
    {
        for (auto listener : listeners_list_) UninstallListener(listener);
        listeners_list_.clear();
    }

    void InstallListener(SettingsListener *listener)
    {
        Load(listener);
        QObject::connect(listener, &SettingsListener::categoryChanged, settings_, [=] { Load(listener); });
        QObject::connect(listener, &SettingsListener::propertiesChanged, settings_, [=](const Ids &roles) { Save(listener, roles); });
        QObject::connect(listener, &SettingsListener::destroyed, settings_, [=] { RemoveListener(listener); });
    }

    void UninstallListener(SettingsListener *listener) { QObject::disconnect(listener, 0, settings_, 0); }

    void OnComponentCompleted()
    {
        listener_->SetTarget(settings_);
        listener_->SetRoles(Role::OWN_ROLES);
        InstallListener(listener_);
        initialized_ = true;
        Load();
    }

    void Load()
    {
        if (!initialized_)
            return;
        Load(listener_);
        for (auto l : listeners_list_) Load(l);
    }

    void Load(SettingsListener *listener)
    {
        if (!initialized_)
            return;

        auto category = listener->GetCategory();
        if (!category.isEmpty())
            settings_->beginGroup(listener->GetCategory());

        for (auto id : listener->GetRoleIds()) listener->Write(id, settings_->value(listener->GetPropertyName(id), listener->Read(id)));

        if (!category.isEmpty())
            settings_->endGroup();
    }

    void Save()
    {
        if (!initialized_)
            return;
        Save(listener_);
        for (auto l : listeners_list_) Save(l);
    }

    void Save(SettingsListener *listener)
    {
        if (!initialized_)
            return;
        Save(listener, listener->GetRoleIds());
    }

    void Save(SettingsListener *listener, const Ids &roles)
    {
        if (!initialized_)
            return;

        auto category = listener->GetCategory();
        if (!category.isEmpty())
            settings_->beginGroup(listener->GetCategory());

        for (auto id : roles) settings_->setValue(listener->GetPropertyName(id), listener->Read(id));

        if (!category.isEmpty())
            settings_->endGroup();
    }

    QString GetCategory() const { return category_; }

    void SetCategory(const QString &val)
    {
        if (category_ == val)
            return;
        category_ = val;
        settings_->ResetStorage();
        Load();
    }

    QString GetFileName() const { return file_name_; }

    void SetFileName(const QString &val)
    {
        if (file_name_ == val)
            return;
        file_name_ = val;
        settings_->ResetStorage();
        Load();
    }

    const QVector<SettingsListener *> &GetListeners() const { return listeners_list_; }

private:
//    COMPONENT_LOGGER("ui.settings");

    AbstractSettings           *settings_ = nullptr;
    SettingsListener           *listener_ = nullptr;  // self listener
    QVector<SettingsListener *> listeners_list_;
    bool                        initialized_ = false;
    QString                     file_name_;
    QString                     category_;
};

// AbstractSettings
AbstractSettings::AbstractSettings(QObject *parent /*= nullptr*/) : QObject(parent), QQmlParserStatus(), d(std::make_unique<Impl>(this))
{}

AbstractSettings::~AbstractSettings()
{}

void AbstractSettings::componentComplete()
{
    d->OnComponentCompleted();
}

void AbstractSettings::Load()
{
    d->Load();
}

void AbstractSettings::Save()
{
    d->Save();
}

QQmlListProperty<SettingsListener> AbstractSettings::GetSettingsListenerQmlListProperty()
{
    return QQmlListProperty<SettingsListener>(this, this, &AbstractSettings::ListAppend, &AbstractSettings::ListCount, &AbstractSettings::ListGet, &AbstractSettings::ListClear);
}

void AbstractSettings::ListAppend(QQmlListProperty<SettingsListener> *list, SettingsListener *val)
{
    auto settings = reinterpret_cast<AbstractSettings *>(list->data);
    settings->d->AddListener(val);
}

int AbstractSettings::ListCount(QQmlListProperty<SettingsListener> *list)
{
    return reinterpret_cast<AbstractSettings *>(list->data)->d->GetListeners().size();
}

SettingsListener *AbstractSettings::ListGet(QQmlListProperty<SettingsListener> *list, int index)
{
    return reinterpret_cast<AbstractSettings *>(list->data)->d->GetListeners().at(index);
}

void AbstractSettings::ListClear(QQmlListProperty<SettingsListener> *list)
{
    auto settings = reinterpret_cast<AbstractSettings *>(list->data);
    settings->d->ClearListeners();
}

QString AbstractSettings::GetCategory() const
{
    return d->GetCategory();
}

void AbstractSettings::SetCategory(const QString &val)
{
    d->SetCategory(val);
}

QString AbstractSettings::GetFileName() const
{
    return d->GetFileName();
}

void AbstractSettings::SetFileName(const QString &val)
{
    d->SetFileName(val);
}

const QVector<SettingsListener *> &AbstractSettings::GetListeners() const
{
    return d->GetListeners();
}

void AbstractSettings::AddListener(SettingsListener *val)
{
    d->AddListener(val);
}

void AbstractSettings::RemoveListener(SettingsListener *val)
{
    d->RemoveListener(val);
}

void AbstractSettings::ClearListeners()
{
    d->ClearListeners();
}
