#include "Controller.h"

Controller::Controller(Model& model, View& view)
    : m_model(model)
    , m_view(view)
{
    // Model의 상태가 바뀔 때마다 View를 갱신하도록 콜백을 등록한다.
    m_model.SetOnChanged([this]() { m_view.ShowItems(m_model.GetItems()); });
}

void Controller::Run()
{
    while (m_running)
    {
        m_view.ShowMenu();

        switch (m_view.ReadMenuOption())
        {
        case MenuOption::Add:
            HandleAdd();
            break;
        case MenuOption::Remove:
            HandleRemove();
            break;
        case MenuOption::Show:
            HandleShow();
            break;
        case MenuOption::Quit:
            m_running = false;
            break;
        case MenuOption::Invalid:
        default:
            m_view.ShowMessage("잘못된 입력입니다. 다시 선택해주세요.");
            break;
        }
    }
}

void Controller::HandleAdd()
{
    const std::string text = m_view.ReadItemText();
    m_model.AddItem(text);
}

void Controller::HandleRemove()
{
    const size_t index = m_view.ReadItemIndex();
    if (!m_model.RemoveItem(index))
    {
        m_view.ShowMessage("해당 번호의 항목이 존재하지 않습니다.");
    }
}

void Controller::HandleShow()
{
    m_view.ShowItems(m_model.GetItems());
}
