
#include <memory>

class ${type_name} : public QAbstractListModel
{
    Q_OBJECT
${properties}
public:
    using iterator       = ${repeated_field_type}<${cpp_data_type}>::iterator;
    using const_iterator = ${repeated_field_type}<${cpp_data_type}>::const_iterator;

    ${type_name}(${repeated_field_type}<${cpp_data_type}>* data, QObject* parent = nullptr);

    ${repeated_field_type}<${cpp_data_type}>* GetProtoMessage() const;
    void SetProtoMessage(${repeated_field_type}<${cpp_data_type}>* data);

    const ${repeated_field_type}<${cpp_data_type}>& Get() const;
    void Set(const ${repeated_field_type}<${cpp_data_type}>& data);
    void Set(${repeated_field_type}<${cpp_data_type}>&& data);
    void SyncData();

    int                    rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant               data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool                   setData(const QModelIndex& index, const QVariant &val, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;

public slots:
    QVariant At(int row) const;
    int IndexOf(const QVariant& val) const;
    void Append(const QVariant& val = QVariant());
    void Remove(int row, int num = 1);
    void Clear();

signals:
    void changed();
    void rowCountChanged();

private:
    ${repeated_field_type}<${cpp_data_type}>* data_ = nullptr;
};