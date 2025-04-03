${type_name}::${type_name}(${repeated_field_type}<${cpp_data_type}>* data, QObject* parent) : QAbstractListModel(parent)
{
    SetProtoMessage(data);
    connect(this, &${type_name}::rowsInserted, this, &${type_name}::changed);
    connect(this, &${type_name}::rowsRemoved, this, &${type_name}::changed);
    connect(this, &${type_name}::rowsMoved, this, &${type_name}::changed);
    connect(this, &${type_name}::modelReset, this, &${type_name}::changed);
    connect(this, &${type_name}::dataChanged, this, &${type_name}::changed);

    connect(this, &${type_name}::rowsInserted, this, &${type_name}::rowCountChanged);
    connect(this, &${type_name}::rowsRemoved, this, &${type_name}::rowCountChanged);
    connect(this, &${type_name}::modelReset, this, &${type_name}::rowCountChanged);
}

${repeated_field_type}<${cpp_data_type}>* ${type_name}::GetProtoMessage() const
{
    return data_;
}

void ${type_name}::SetProtoMessage(${repeated_field_type}<${cpp_data_type}>* data)
{
    beginResetModel();
    data_ = data;
    endResetModel();
}

const ${repeated_field_type}<${cpp_data_type}>& ${type_name}::Get() const
{
    return *data_;
}

void ${type_name}::Set(const ${repeated_field_type}<${cpp_data_type}>& data)
{
    beginResetModel();
    data_->CopyFrom(data);
    endResetModel();
}

void ${type_name}::Set(${repeated_field_type}<${cpp_data_type}>&& data)
{
    beginResetModel();
    data_->Swap(&data);
    endResetModel();
}

void ${type_name}::SyncData()
{
    beginResetModel();
    endResetModel();
}

int ${type_name}::rowCount(const QModelIndex& parent) const
{
    return data_ ? data_->size() : 0;
}

QVariant ${type_name}::data(const QModelIndex& index, int role) const
{
    if (!data_ || !index.isValid() || role != Qt::UserRole)
        return QVariant();
    return ${get_val};
}

bool ${type_name}::setData(const QModelIndex& index, const QVariant &val, int role)
{
    if (!data_ || !index.isValid())
        return true;
    if (role == Qt::UserRole)
    {
        ${set_val};
        emit dataChanged(index, index, { Qt::UserRole });
    }
    return true;
}

QHash<int, QByteArray> ${type_name}::roleNames() const
{
    return { { Qt::UserRole, "item" } };
}

QVariant ${type_name}::At(int row) const
{
    return data(index(row, 0), Qt::UserRole);
}

int ${type_name}::IndexOf(const QVariant& val) const
{
    for (int i = 0; i < rowCount(); ++i)
        if (At(i) == val)
            return i;
    return -1;
}

void ${type_name}::Append(const QVariant& val)
{
    if (!data_)
        return;
    auto row = data_->size();
    beginInsertRows(QModelIndex(), row, row);
    data_->Add();
    endInsertRows();
    setData(index(row, 0), val, Qt::UserRole);
}

void ${type_name}::Remove(int row, int num)
{
    if (!data_)
        return;
    beginRemoveRows(QModelIndex(), row, row + num - 1);
    data_->erase(data_->begin() + row, data_->begin() + row + num);
    endRemoveRows();
}

void ${type_name}::Clear()
{
    if (!data_)
        return;
    beginResetModel();
    data_->Clear();
    endResetModel();
}

${type_name}::iterator ${type_name}::begin()
{
    return data_->begin();
}

${type_name}::iterator ${type_name}::end()
{
    return data_->end();
}

${type_name}::const_iterator ${type_name}::begin() const
{
    return data_->begin();
}

${type_name}::const_iterator ${type_name}::end() const
{
    return data_->end();
}

${type_name}::const_iterator ${type_name}::cbegin() const
{
    return data_->cbegin();
}

${type_name}::const_iterator ${type_name}::cend() const
{
    return data_->cend();
}