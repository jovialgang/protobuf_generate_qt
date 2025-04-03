${type_name}::${type_name}(QObject* parent) : QObject(parent)
{
    owns_message_ = true;
    message_ = new ${message_type}();
    Setup();
}

${type_name}::${type_name}(${message_type}* message, QObject* parent) : QObject(parent)
{
    if (!message)
    {
        owns_message_ = true;
        message_ = new ${message_type}();
    }
    else
    {
        owns_message_ = false;
        message_ = message;
    }
    Setup();
}

${type_name}::${type_name}(const ${message_type}& val, QObject* parent) : QObject(parent)
{
    owns_message_ = true;
    message_ = new ${message_type}(val);
    Setup();
}

${type_name}::${type_name}(${message_type}&& val, QObject* parent) : QObject(parent)
{
    owns_message_ = true;
    message_ = new ${message_type}(std::move(val));
    Setup();
}

${type_name}::~${type_name}() 
{
    if (owns_message_)
        delete message_;
}

void ${type_name}::SyncProtoMessage()
{
    SyncProtoMessagePrivate(true);
}

void ${type_name}::SyncProtoMessagePrivate(bool emit_all_signals)
{
    EmitChangedProperties(emit_all_signals);
${sync_members}
}

void ${type_name}::CheckForChangedAndSetProtoMessage(${message_type}* new_message)
{
    auto check_for_changed = new_message && !emit_all_signals_;
    if (check_for_changed)
        CheckForChangedProperties(*new_message);
    SetProtoMessage(new_message, !check_for_changed);
}

bool ${type_name}::EqualsTo(const ${message_type}& val) const
{
    return google::protobuf::util::MessageDifferencer::Equals(Get(), val);
}

bool ${type_name}::EquivalentTo(const ${message_type}& val) const
{
    return google::protobuf::util::MessageDifferencer::Equivalent(Get(), val);
}

bool ${type_name}::ApproximatelyEqualsTo(const ${message_type}& val) const
{
    return google::protobuf::util::MessageDifferencer::ApproximatelyEquals(Get(), val);
}

bool ${type_name}::ApproximatelyEquivalentTo(const ${message_type}& val) const
{
    return google::protobuf::util::MessageDifferencer::ApproximatelyEquivalent(Get(), val);
}

void ${type_name}::CheckForChangedProperties(const ${message_type}& new_message)
{
${check_members}
}

void ${type_name}::RevokeProtoMessageOwnership()
{
    owns_message_ = false;
}

${message_type}* ${type_name}::GetProtoMessage() const 
{
    return message_;
}

void ${type_name}::SetProtoMessage(${message_type}* message, bool emit_all_signals)
{
    if (!message)
    {
        if (owns_message_)
            delete message_;
        owns_message_ = true;
        message_ = new ${message_type}();
    }
    else if (message != message_)
    {
        if (owns_message_)
        {
            delete message_;
            owns_message_ = false;
        }
        message_ = message;
    }

    SyncProtoMessagePrivate(emit_all_signals);
}

const ${message_type}& ${type_name}::Get() const 
{
    return *message_;
}

void ${type_name}::Set(const ${message_type}& message)
{
    CheckForChangedProperties(message);
    *message_ = message;
    SyncProtoMessagePrivate();
}

void ${type_name}::Set(${message_type}&& message)
{
    CheckForChangedProperties(message);
    *message_ = std::move(message);
    SyncProtoMessagePrivate();
}

void ${type_name}::Setup()
{
    changed_properties_.fill(false);
${setup}
}

void ${type_name}::EmitChangedProperties(bool emit_all_signals)
{
    if (emit_all_signals)
        emit_all_signals_ = true;
    if (sync_signals_queued_)
        return;
    sync_signals_queued_ = true;
    QMetaObject::invokeMethod(this, [=] {
${sync_signals}
        changed_properties_.fill(false);
        emit_all_signals_ = false;
        sync_signals_queued_ = false;
    }, Qt::QueuedConnection);
}

void ${type_name}::EmitChanged()
{
    if (changed_signal_queued_)
        return;
    changed_signal_queued_ = true;
    QMetaObject::invokeMethod(this, [this] {
        emit changed();
        changed_signal_queued_ = false;
    }, Qt::QueuedConnection);
}

bool ${type_name}::Parse(const QByteArray& data)
{
    if (message_->ParseFromString(data.toStdString()))
    {
        SyncProtoMessagePrivate(true);
        return true;
    }
    return false;
}

QByteArray ${type_name}::Serialize() const
{
    return QByteArray::fromStdString(message_->SerializeAsString());
}

bool ${type_name}::EqualsTo(${type_name} *val) const
{
    if (!val)
        return false;
    return google::protobuf::util::MessageDifferencer::Equals(Get(), val->Get());
}

bool ${type_name}::EquivalentTo(${type_name} *val) const
{
    if (!val)
        return false;
    return google::protobuf::util::MessageDifferencer::Equivalent(Get(), val->Get());
}

bool ${type_name}::ApproximatelyEqualsTo(${type_name} *val) const
{
    if (!val)
        return false;
    return google::protobuf::util::MessageDifferencer::ApproximatelyEquals(Get(), val->Get());
}

bool ${type_name}::ApproximatelyEquivalentTo(${type_name} *val) const
{    
    if (!val)
        return false;
    return google::protobuf::util::MessageDifferencer::ApproximatelyEquivalent(Get(), val->Get());
}

void ${type_name}::Initialize(${type_name} *val)
{
    if (!val)
        return;
    message_->CopyFrom(val->Get());
    SyncProtoMessagePrivate();
}

void ${type_name}::CopyFrom(${type_name} *val)
{
    if (!val)
        return;
    auto message = val->Get();
    CheckForChangedProperties(message);
    message_->CopyFrom(message);
    SyncProtoMessagePrivate();
}

void ${type_name}::MergeFrom(${type_name} *val)
{
    if (!val)
        return;
    auto message = val->Get();
    CheckForChangedProperties(message);
    message_->MergeFrom(message);
    SyncProtoMessagePrivate();
}

void ${type_name}::Reset()
{
    message_->Clear();
    SyncProtoMessagePrivate(true);
}

bool ${type_name}::IsInitialized() const
{
    return message_->IsInitialized();
}

QString	${type_name}::GetDebugString() const
{
    return QString::fromStdString(message_->DebugString());
}

QString	${type_name}::GetShortDebugString() const
{
    return QString::fromStdString(message_->ShortDebugString());
}

QString	${type_name}::GetUtf8DebugString() const
{
    return QString::fromStdString(message_->Utf8DebugString());
}

${function_implementations}