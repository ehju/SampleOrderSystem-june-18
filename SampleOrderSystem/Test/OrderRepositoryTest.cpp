#include <gtest/gtest.h>

#include <filesystem>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "../Lib/Json.h"
#include "../Model/OrderRepository.h"

using order_system::Order;
using order_system::OrderRepository;
using order_system::OrderStatus;

namespace {

namespace fs = std::filesystem;

class OrderRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        testDir_ = fs::temp_directory_path() / "SampleOrderSystemTest" /
            ::testing::UnitTest::GetInstance()->current_test_info()->name();

        std::error_code ec;
        fs::remove_all(testDir_, ec);
        fs::create_directories(testDir_, ec);

        jsonFilePath_ = testDir_ / "orders.json";
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(testDir_, ec);
    }

    // orderId/sampleId/customerName/quantity/status 조합으로 orders.json 을 직접 작성한다.
    // OrderRepository 의 공개 API(Add) 로는 RESERVED 이외의 상태를 만들 수 없으므로,
    // 상태별 조회 테스트를 위해 파일을 직접 구성한다.
    void WriteRawOrdersJson(
        const std::vector<std::tuple<int, int, std::string, int, std::string>>& entries)
    {
        json::JsonValue root = json::JsonValue::MakeArray();
        for (const auto& entry : entries) {
            json::JsonValue item = json::JsonValue::MakeObject();
            item["orderId"] = std::get<0>(entry);
            item["sampleId"] = std::get<1>(entry);
            item["customerName"] = std::get<2>(entry);
            item["quantity"] = std::get<3>(entry);
            item["status"] = std::get<4>(entry);
            item["createdAt"] = 0;
            root.Push(std::move(item));
        }
        root.SaveToFile(jsonFilePath_);
    }

    fs::path testDir_;
    fs::path jsonFilePath_;
};

}  // namespace

// 정상 생성 시 orderId 는 자동 채번되고 status 는 RESERVED 로 설정된다.
TEST_F(OrderRepositoryTest, AddAssignsSequentialIdAndReservedStatus)
{
    OrderRepository repository(jsonFilePath_);

    Order first = repository.Add(10, "CustomerA", 5);
    Order second = repository.Add(20, "CustomerB", 3);

    EXPECT_EQ(first.GetOrderId(), 1);
    EXPECT_EQ(first.GetStatus(), OrderStatus::RESERVED);
    EXPECT_EQ(second.GetOrderId(), 2);
    EXPECT_EQ(second.GetStatus(), OrderStatus::RESERVED);
}

// 생성된 주문의 sampleId/customerName/quantity 는 입력값과 동일해야 한다.
TEST_F(OrderRepositoryTest, AddStoresInputValuesCorrectly)
{
    OrderRepository repository(jsonFilePath_);

    Order order = repository.Add(10, "CustomerA", 5);

    EXPECT_EQ(order.GetSampleId(), 10);
    EXPECT_EQ(order.GetCustomerName(), "CustomerA");
    EXPECT_EQ(order.GetQuantity(), 5);
}

// 상태별 조회: RESERVED 주문만 반환하고 다른 상태의 주문은 제외한다.
TEST_F(OrderRepositoryTest, FindByStatusReturnsOnlyMatchingStatusOrders)
{
    WriteRawOrdersJson({
        {1, 10, "CustomerA", 5, "RESERVED"},
        {2, 10, "CustomerB", 3, "CONFIRMED"},
        {3, 20, "CustomerC", 2, "REJECTED"},
        {4, 20, "CustomerD", 7, "RESERVED"},
    });

    OrderRepository repository(jsonFilePath_);

    std::vector<Order> reserved = repository.FindByStatus(OrderStatus::RESERVED);

    ASSERT_EQ(reserved.size(), 2u);
    EXPECT_EQ(reserved[0].GetOrderId(), 1);
    EXPECT_EQ(reserved[1].GetOrderId(), 4);
}

// ID 조회: 존재하는 ID 는 해당 주문을 반환한다.
TEST_F(OrderRepositoryTest, FindByIdReturnsOrderWhenExists)
{
    OrderRepository repository(jsonFilePath_);
    Order created = repository.Add(10, "CustomerA", 5);

    std::optional<Order> found = repository.FindById(created.GetOrderId());

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->GetCustomerName(), "CustomerA");
}

// ID 조회: 존재하지 않는 ID 는 값이 없음을 반환한다.
TEST_F(OrderRepositoryTest, FindByIdReturnsNulloptWhenNotExists)
{
    OrderRepository repository(jsonFilePath_);
    repository.Add(10, "CustomerA", 5);

    std::optional<Order> found = repository.FindById(9999);

    EXPECT_FALSE(found.has_value());
}

// 전체 조회: 생성된 모든 주문을 반환한다.
TEST_F(OrderRepositoryTest, GetAllReturnsAllCreatedOrders)
{
    OrderRepository repository(jsonFilePath_);
    repository.Add(10, "CustomerA", 5);
    repository.Add(20, "CustomerB", 3);

    std::vector<Order> all = repository.GetAll();

    EXPECT_EQ(all.size(), 2u);
}

// JSON 파일이 없으면 빈 상태로 시작한다.
TEST_F(OrderRepositoryTest, StartsEmptyWhenJsonFileDoesNotExist)
{
    OrderRepository repository(jsonFilePath_);

    EXPECT_TRUE(repository.GetAll().empty());
}

// 생성 직후 JSON 파일에 반영된다.
TEST_F(OrderRepositoryTest, AddPersistsToJsonFileImmediately)
{
    OrderRepository repository(jsonFilePath_);

    repository.Add(10, "CustomerA", 5);

    EXPECT_TRUE(fs::exists(jsonFilePath_));
}

// 기존 JSON 파일이 있는 상태로 Repository 를 생성하면 그 데이터가 로드된다.
TEST_F(OrderRepositoryTest, LoadsExistingDataFromJsonFileOnConstruction)
{
    {
        OrderRepository repository(jsonFilePath_);
        repository.Add(10, "CustomerA", 5);
        repository.Add(20, "CustomerB", 3);
    }

    OrderRepository reloaded(jsonFilePath_);
    std::vector<Order> all = reloaded.GetAll();

    EXPECT_EQ(all.size(), 2u);
}
