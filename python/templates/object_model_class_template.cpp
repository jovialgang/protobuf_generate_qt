${type_name}::${type_name}(google::protobuf::RepeatedPtrField<${data_type}>* data, QObject* parent) : om::ObjectModel(${object_type}::staticMetaObject, parent)
{
    SetProtoMessage(data);

    connect(this, &${type_name}::rowsInserted, this, &${type_name}::OnInserted);
    connect(this, &${type_name}::rowsRemoved, this, &${type_name}::OnItemRemoved);
    connect(this, &${type_name}::rowsMoved, this, &${type_name}::OnItemMoved);
    connect(this, &${type_name}::modelReset, this, &${type_name}::OnReset);
    connect(this, &${type_name}::dataChanged, this, &${type_name}::OnDataChanged);
}

${type_name}::~${type_name}()
{
    UninstallAllItems();
}

${object_type}* ${type_name}::At(int row) const
{
    return static_cast<${object_type}*>(om::ObjectModel::At(row));
}

${object_type}* ${type_name}::First() const
{
    return static_cast<${object_type}*>(om::ObjectModel::First());
}

${object_type}* ${type_name}::Last() const
{
    return static_cast<${object_type}*>(om::ObjectModel::Last());
}

${object_type}* ${type_name}::Take(int row)
{
    return static_cast<${object_type}*>(om::ObjectModel::Take(row));
}

${object_type}* ${type_name}::TakeFirst()
{
    return static_cast<${object_type}*>(om::ObjectModel::TakeFirst());
}

${object_type}* ${type_name}::TakeLast()
{
    return static_cast<${object_type}*>(om::ObjectModel::TakeLast());
}

${object_type}* ${type_name}::AppendCopy(${object_type} * item)
{
    auto new_item = new ${object_type}(item->Get(), this);
    Append(new_item);
    return new_item;
}

google::protobuf::RepeatedPtrField<${data_type}>* ${type_name}::GetProtoMessage() const
{
    return data_;
}

const google::protobuf::RepeatedPtrField<${data_type}>& ${type_name}::Get() const
{
    return *data_;
}

void ${type_name}::SetProtoMessage(google::protobuf::RepeatedPtrField<${data_type}>* data, bool emit_all_signals)
{
    data_ = data;
    PrivateSyncData(emit_all_signals);
}

void ${type_name}::CheckForChangedAndSetProtoMessage(google::protobuf::RepeatedPtrField<${data_type}>* data)
{
    if (data)
        CheckForChangedProperties(*data);
    SetProtoMessage(data, !data);
}

void ${type_name}::CheckForChangedProperties(const google::protobuf::RepeatedPtrField<${data_type}>& new_data)
{
    for (int i = 0; i < std::min(items_.size(), new_data.size()); ++i)
        static_cast<${object_type}*>(items_[i])->CheckForChangedProperties(new_data[i]);
}

void ${type_name}::Set(const google::protobuf::RepeatedPtrField<${data_type}>& data)
{
    CheckForChangedProperties(data);
    data_->CopyFrom(data);
    PrivateSyncData();
}

void ${type_name}::Set(google::protobuf::RepeatedPtrField<${data_type}>&& data)
{
    CheckForChangedProperties(data);
    data_->Swap(&data);
    PrivateSyncData();
}

void ${type_name}::SyncData()
{
    PrivateSyncData(true);
}

void ${type_name}::PrivateSyncData(bool emit_all_signals)
{
    synchronizing_ = true;
    auto sync_size = std::min(data_->size(), items_.size());
    for (int i = 0; i < sync_size; ++i)
        static_cast<${object_type}*>(items_[i])->SetProtoMessage(data_->Mutable(i), emit_all_signals);
    Resize(data_->size());
    synchronizing_ = false;
    EmitChanged();
}

void ${type_name}::InstallItem(int row) 
{
    om::ObjectModel::InstallItem(row);
    connect(static_cast<${object_type}*>(items_[row]), &${object_type}::changed, this, &${type_name}::EmitChanged);
}

QObject* ${type_name}::CreateNewInstance(int row)
{
    return new ${object_type}(row >= 0 && row < data_->size() ? data_->Mutable(row) : nullptr, this);
}

void ${type_name}::OnInserted(const QModelIndex& parent, int first, int last)
{
    if (synchronizing_)
        return;
    int count = last - first + 1;
    if (items_.size() - data_->size() != count)
        throw std::logic_error("Model is not synced");

    auto pos = data_->size();

    for (int i = first; i <= last; ++i)
    {
        auto item = static_cast<${object_type}*>(items_[i]);
        item->RevokeProtoMessageOwnership();
        data_->AddAllocated(item->GetProtoMessage());
    }

    if (pos != first)
        for (int i = 0; i < count; ++i) data_->SwapElements(pos + i, first + i);

    EmitChanged();
}

void ${type_name}::OnItemRemoved(const QModelIndex& parent, int first, int last)
{
    if (synchronizing_)
        return;
    if (data_->size() - items_.size() != last - first + 1)
        throw std::logic_error("Model is not synced");
    data_->DeleteSubrange(first, last - first + 1);
    EmitChanged();
}

void ${type_name}::OnItemMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
{
    throw std::runtime_error("Not supported yet");
    EmitChanged();
}

void ${type_name}::OnReset()
{
    data_->Clear();
    EmitChanged();
}

void ${type_name}::OnDataChanged(const QModelIndex &top_left, const QModelIndex &bottom_right, const QVector<int> &roles)
{
    if (!roles.isEmpty() || !top_left.isValid() || !bottom_right.isValid())
        return;
    for (int row = top_left.row(); row <= bottom_right.row(); ++row)
    {
        auto item = static_cast<${object_type}*>(items_[row]);
        item->RevokeProtoMessageOwnership();
        data_->AddAllocated(item->GetProtoMessage());
        data_->SwapElements(row, data_->size() - 1);
        data_->RemoveLast();
        EmitChanged();
    }
}

void ${type_name}::EmitChanged()
{
    if (changed_signal_emitted_)
        return;
    changed_signal_emitted_ = true;
    QMetaObject::invokeMethod(this, [this] {
        emit changed();
        changed_signal_emitted_ = false;
    }, Qt::QueuedConnection);
}

${type_name}::const_iterator ${type_name}::begin() const
{
    return reinterpret_cast<const_iterator>(om::ObjectModel::begin());
}

${type_name}::const_iterator ${type_name}::end() const
{
    return reinterpret_cast<const_iterator>(om::ObjectModel::end());
}

${type_name}::const_iterator ${type_name}::cbegin() const
{
    return reinterpret_cast<const_iterator>(om::ObjectModel::begin());
}

${type_name}::const_iterator ${type_name}::cend() const
{
    return reinterpret_cast<const_iterator>(om::ObjectModel::end());
}