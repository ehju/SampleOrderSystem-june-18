#include "Controllers/Controller.h"
#include "Models/Model.h"
#include "Views/View.h"

int main()
{
    Model model;
    View view;
    Controller controller(model, view);

    controller.Run();

    return 0;
}
