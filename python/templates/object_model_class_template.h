class ${type_name} : public om::ObjectModel
{
    Q_OBJECT
public:
    using iterator       = ${object_type} **;
    using const_iterator = ${object_type} *const *;

    ${type_name}(google::protobuf::RepeatedPtrField<${data_type}>* data, QObject* parent = nullptr);
    ~${type_name}() override;

    google::protobuf::RepeatedPtrField<${data_type}>* GetProtoMessage() const;
    void SetProtoMessage(google::protobuf::RepeatedPtrField<${data_type}>* data, bool emit_all_signals = false);

    void CheckForChangedAndSetProtoMessage(google::protobuf::RepeatedPtrField<${data_type}>* data);
    void CheckForChangedProperties(const google::protobuf::RepeatedPtrField<${data_type}>& new_data);

    const google::protobuf::RepeatedPtrField<${data_type}>& Get() const;
    void Set(const google::protobuf::RepeatedPtrField<${data_type}>& data);
    void Set(google::protobuf::RepeatedPtrField<${data_type}>&& data);
    void SyncData();

    const_iterator begin() const;
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;

signals:
    void changed();

public slots:
    ${object_type}* At(int row) const;
    ${object_type}* First() const;
    ${object_type}* Last() const;
    ${object_type}* Take(int row);
    ${object_type}* TakeFirst();
    ${object_type}* TakeLast();
    ${object_type}* AppendCopy(${object_type} * item);

private slots:
    void OnInserted(const QModelIndex& parent, int first, int last);
    void OnItemRemoved(const QModelIndex& parent, int first, int last);
    void OnItemMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);
    void OnReset();
    void OnDataChanged(const QModelIndex &top_left, const QModelIndex &bottom_right, const QVector<int> &roles = QVector<int>());
    void EmitChanged();
    
private:
    void InstallItem(int row) override;
    QObject* CreateNewInstance(int row) override;
    
    void PrivateSyncData(bool emit_all_signals = false);

    bool synchronizing_ = false;
    bool changed_signal_emitted_ = false;
    google::protobuf::RepeatedPtrField<${data_type}>* data_ = nullptr;
};