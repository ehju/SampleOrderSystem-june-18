#pragma once

#include <functional>
#include <string>
#include <vector>

// Model: 데이터와 비즈니스 로직만 담당한다.
// View나 Controller의 존재를 알지 못하며, 상태 변경은 콜백으로만 외부에 알린다.
class Model
{
public:
    using ChangeListener = std::function<void()>;

    void AddItem(const std::string& item);
    bool RemoveItem(size_t index);

    const std::vector<std::string>& GetItems() const;

    void SetOnChanged(ChangeListener listener);

private:
    void NotifyChanged() const;

    std::vector<std::string> m_items;
    ChangeListener m_onChanged;
};
