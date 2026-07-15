#pragma once

#include <string>
#include <vector>

// View: 화면 출력과 입력 수신만 담당한다.
// Model을 직접 참조하지 않고, Controller가 전달해주는 데이터만 그린다.
enum class MenuOption
{
    Add,
    Remove,
    Show,
    Quit,
    Invalid
};

class View
{
public:
    void ShowMenu() const;
    MenuOption ReadMenuOption() const;

    std::string ReadItemText() const;
    size_t ReadItemIndex() const;

    void ShowItems(const std::vector<std::string>& items) const;
    void ShowMessage(const std::string& message) const;
};
