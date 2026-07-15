#include "Model.h"

void Model::AddItem(const std::string& item)
{
    m_items.push_back(item);
    NotifyChanged();
}

bool Model::RemoveItem(size_t index)
{
    if (index >= m_items.size())
    {
        return false;
    }

    m_items.erase(m_items.begin() + index);
    NotifyChanged();
    return true;
}

const std::vector<std::string>& Model::GetItems() const
{
    return m_items;
}

void Model::SetOnChanged(ChangeListener listener)
{
    m_onChanged = std::move(listener);
}

void Model::NotifyChanged() const
{
    if (m_onChanged)
    {
        m_onChanged();
    }
}
