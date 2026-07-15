#pragma once

#include <string>

#include "../Controller/MainMenuController.h"

namespace order_system {

// MainMenuView: 메인 메뉴 화면 출력과 메뉴 선택 입력만 담당한다.
class MainMenuView {
public:
    void ShowMainMenu(const MainMenuSummary& summary) const;

    // 메뉴 선택 값을 입력받는다. 숫자가 아니면 -1을 반환한다.
    int ReadMenuChoice() const;

    void ShowMessage(const std::string& message) const;
};

}  // namespace order_system
