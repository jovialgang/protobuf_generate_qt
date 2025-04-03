#include "model_value.h"

// ListModelValue
om::ListModelValue::ListModelValue(QAbstractListModel* model, QModelIndex index, Id role_id) : model_(model), index_(std::move(index)), role_id_(role_id)
{}

bool om::ListModelValue::IsValid() const
{
    return index_.isValid();
}

QVariant om::ListModelValue::Get() const
{
    return model_->data(index_, role_id_);
}

bool om::ListModelValue::Set(const QVariant& val)
{
    return model_->setData(index_, val, role_id_);
}

QModelIndex om::ListModelValue::GetIndex() const
{
    return index_;
}

void om::ListModelValue::SetIndex(QModelIndex val)
{
    index_ = std::move(val);
}

om::ListModelValue& om::ListModelValue::operator++()
{
    index_ = model_->index(index_.row() + 1, index_.column());
    return *this;
}

om::ListModelValue& om::ListModelValue::operator--()
{
    index_ = model_->index(index_.row() - 1, index_.column());
    return *this;
}

// ListModelAccessValue
om::ListModelAccessValue::ListModelAccessValue(ListModelAccess* model, int row, Id role_id) : model_(model), row_(row), role_id_(role_id)
{}

om::ListModelAccessValue::ListModelAccessValue(ListModelAccess* model, int row, const QByteArray& role_name) : model_(model), row_(row), role_id_(model_->GetRoleId(role_name))
{}

bool om::ListModelAccessValue::IsValid() const
{
    return row_ >= 0 && row_ < model_->GetCount();
}

QVariant om::ListModelAccessValue::Get() const
{
    return model_->GetData(row_, role_id_);
}

bool om::ListModelAccessValue::Set(const QVariant& val)
{
    return model_->SetData(row_, val, role_id_);
}

int om::ListModelAccessValue::GetRow() const
{
    return row_;
}

void om::ListModelAccessValue::SetRow(int val)
{
    row_ = val;
}

om::ListModelAccessValue& om::ListModelAccessValue::operator++()
{
    row_++;
    return *this;
}

om::ListModelAccessValue& om::ListModelAccessValue::operator--()
{
    row_--;
    return *this;
}
