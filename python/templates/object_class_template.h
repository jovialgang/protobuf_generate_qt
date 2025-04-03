class ${type_name} : public QObject
{
    Q_OBJECT
${properties}
public:
${using}
${enums}
    Q_INVOKABLE ${type_name}(QObject* parent = nullptr);
    ${type_name}(${message_type}* message, QObject* parent = nullptr);
    ${type_name}(const ${message_type}& val, QObject* parent = nullptr);
    ${type_name}(${message_type}&& val, QObject* parent = nullptr);
    ~${type_name}();

    void SyncProtoMessage();
    void CheckForChangedAndSetProtoMessage(${message_type}* new_message);
    void CheckForChangedProperties(const ${message_type}& new_message);
    void RevokeProtoMessageOwnership();

    bool EqualsTo(const ${message_type}& val) const;
    bool EquivalentTo(const ${message_type}& val) const;
    bool ApproximatelyEqualsTo(const ${message_type}& val) const;
    bool ApproximatelyEquivalentTo(const ${message_type}& val) const;

    ${message_type}* GetProtoMessage() const;
    void SetProtoMessage(${message_type}* message, bool emit_all_signals = false);

    const ${message_type}& Get() const;
    void Set(const ${message_type}& val);
    void Set(${message_type}&& val);

${function_definitions}
public slots:
    void Initialize(${type_name} *val);
    void CopyFrom(${type_name} *val);
    void MergeFrom(${type_name} *val);
    void Reset();

    bool Parse(const QByteArray& data);
    QByteArray Serialize() const;

    bool IsInitialized() const;
    bool EqualsTo(${type_name} *val) const;
    bool EquivalentTo(${type_name} *val) const;
    bool ApproximatelyEqualsTo(${type_name} *val) const;
    bool ApproximatelyEquivalentTo(${type_name} *val) const;

    QString	GetDebugString() const;
    QString	GetShortDebugString() const;
    QString	GetUtf8DebugString() const;

${slot_function_definitions}
signals:
    void changed();
${notifiers}
private slots:
    void EmitChanged();

private:
    void Setup();
    void EmitChangedProperties(bool emit_all_signals = false);
    void SyncProtoMessagePrivate(bool emit_all_signals = false);

${private_function_definitions}

mutable ${message_type}* message_ = nullptr;
// Класс ли следит за удалением message_
bool owns_message_ = true;
// Выставляется в true когда вызывается EmitChanged() и сигнал changed добавляется в очередь на выполнение
// Сбрасывается в false после эмита сигнала changed()
bool changed_signal_queued_ = false;
// Выставляется в true когда вызывается EmitChangedProperties() и сигналы об изменении полей добавляются в очередь на выполнение
// Сбрасывается в false после эмита сигналов полей
bool sync_signals_queued_ = false;
// Отражает необходимость вызова всех сигналов класса, кроме наследованных
bool emit_all_signals_ = false;
// Хранит то, какие поля у message_ изменились
std::array<bool, ${fields_count}> changed_properties_;

${members}
};