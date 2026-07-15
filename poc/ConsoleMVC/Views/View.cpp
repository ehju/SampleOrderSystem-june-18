#include "View.h"

#include <iostream>
#include <limits>

void View::ShowMenu() const
{
    std::cout << "\n===== ConsoleMVC =====\n"
        << "1. 항목 추가 (Add)\n"
        << "2. 항목 삭제 (Remove)\n"
        << "3. 목록 보기 (Show)\n"
        << "4. 종료 (Quit)\n"
        << "번호를 선택하세요: ";
}

MenuOption View::ReadMenuOption() const
{
    int choice = 0;
    if (!(std::cin >> choice))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return MenuOption::Invalid;
    }

    switch (choice)
    {
    case 1: return MenuOption::Add;
    case 2: return MenuOption::Remove;
    case 3: return MenuOption::Show;
    case 4: return MenuOption::Quit;
    default: return MenuOption::Invalid;
    }
}

std::string View::ReadItemText() const
{
    std::cout << "추가할 항목 내용을 입력하세요: ";
    std::string text;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, text);
    return text;
}

size_t View::ReadItemIndex() const
{
    std::cout << "삭제할 항목의 번호(1부터 시작)를 입력하세요: ";
    size_t index = 0;
    std::cin >> index;
    return index == 0 ? 0 : index - 1;
}

void View::ShowItems(const std::vector<std::string>& items) const
{
    std::cout << "\n--- 목록 (" << items.size() << "개) ---\n";
    if (items.empty())
    {
        std::cout << "(비어 있음)\n";
        return;
    }

    for (size_t i = 0; i < items.size(); ++i)
    {
        std::cout << (i + 1) << ". " << items[i] << '\n';
    }
}

void View::ShowMessage(const std::string& message) const
{
    std::cout << message << '\n';
}
