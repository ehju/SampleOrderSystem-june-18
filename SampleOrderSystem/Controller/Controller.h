#pragma once

#include "../Model/Model.h"
#include "../View/View.h"

// Controller: 사용자 입력을 해석해 Model을 조작하고, Model의 변경 결과를 View에 반영한다.
class Controller
{
public:
    Controller(Model& model, View& view);

    void Run();

private:
    void HandleAdd();
    void HandleRemove();
    void HandleShow();

    Model& m_model;
    View& m_view;
    bool m_running = true;
};
