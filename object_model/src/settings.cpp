#include "settings.h"
#include <QDebug>
#include <QCoreApplication>

using namespace om;

Settings::Settings(QObject *parent) : AbstractSettings(parent)
{}

QVariant Settings::value(const QString &key, const QVariant &default_value /* = QVariant() */)
{
    return Instance()->value(key, default_value);
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    Instance()->setValue(key, value);
}

void Settings::sync()
{
    Instance()->sync();
}

void Settings::beginGroup(const QString &prefix)
{
    Instance()->beginGroup(prefix);
}

void Settings::endGroup()
{
    Instance()->endGroup();
}

const std::unique_ptr<QSettings> &Settings::Instance()
{
    if (!settings_)
    {
        settings_ = GetFileName().isEmpty() ? std::make_unique<QSettings>() : std::make_unique<QSettings>(GetFileName(), QSettings::Format::IniFormat);

        if (settings_->status() != QSettings::NoError)
        {
            QString a = static_cast<QChar>(settings_->status());
            qDebug() << ("Failed to initialize QSettings instance. Status code is: " + a);
            if (settings_->status() == QSettings::AccessError)
            {
                QStringList missing_identifiers;
                if (QCoreApplication::organizationName().isEmpty())
                    missing_identifiers.append(QLatin1String("organizationName"));
                if (QCoreApplication::organizationDomain().isEmpty())
                    missing_identifiers.append(QLatin1String("organizationDomain"));
                if (QCoreApplication::applicationName().isEmpty())
                    missing_identifiers.append(QLatin1String("applicationName"));
                if (!missing_identifiers.isEmpty())
                    qDebug() << ("The following application identifiers have not been set: " + missing_identifiers.join(","));
            }
        }

        auto category = GetCategory();
        if (!category.isEmpty())
            beginGroup(category);
    }

    return settings_;
}

void Settings::ResetStorage()
{
    settings_.reset();
}
