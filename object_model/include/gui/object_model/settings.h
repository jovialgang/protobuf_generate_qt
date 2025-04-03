#pragma once

#include "abstract_settings.h"

#include <QSettings>
#include <memory>

namespace om
{
class Settings : public AbstractSettings
{
    Q_OBJECT
public:
    Settings(QObject *parent = nullptr);

public slots:
    QVariant value(const QString &key, const QVariant &default_value = QVariant()) override;
    void     setValue(const QString &key, const QVariant &value) override;
    void     sync() override;
    void     beginGroup(const QString &prefix) override;
    void     endGroup() override;

protected:
    void ResetStorage() override;

private:
    const std::unique_ptr<QSettings> &Instance();

    std::unique_ptr<QSettings> settings_;
};
}  // namespace om
