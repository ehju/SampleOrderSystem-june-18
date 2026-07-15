#include <filesystem>

#include "Controller/MainMenuController.h"
#include "Controller/MonitoringController.h"
#include "Controller/OrderController.h"
#include "Controller/ProductionController.h"
#include "Controller/SampleController.h"
#include "Controller/ShipmentController.h"
#include "Model/OrderRepository.h"
#include "Model/ProductionQueue.h"
#include "Model/SampleRepository.h"

using namespace order_system;

int main()
{
    std::filesystem::create_directories("data");

    SampleRepository sampleRepository("data/samples.json");
    OrderRepository orderRepository("data/orders.json");
    ProductionQueue productionQueue;

    SampleController sampleController(sampleRepository);
    OrderController orderController(sampleRepository, orderRepository);
    MonitoringController monitoringController(sampleRepository, orderRepository);
    ProductionController productionController(productionQueue, sampleRepository, orderRepository);
    ShipmentController shipmentController(sampleRepository, orderRepository);

    MainMenuController mainMenuController(sampleRepository, sampleController, orderController, monitoringController,
                                           productionController, shipmentController, productionQueue);
    mainMenuController.Run();

    return 0;
}
