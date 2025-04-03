#pragma once

#include "model_value.h"

#include <QAbstractListModel>
#include <iterator>

namespace om
{

class ListModelAccessIterator : public std::iterator<std::input_iterator_tag, ListModelAccessValue>
{
public:
private:
};
}  // namespace om