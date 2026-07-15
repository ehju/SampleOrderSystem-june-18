#include "Controller/Controller.h"
#include "Model/Model.h"
#include "View/View.h"

int main()
{
    Model model;
    View view;
    Controller controller(model, view);

    controller.Run();

    return 0;
}
