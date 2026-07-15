#include <gtest/gtest.h>

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <vector>

#include "../Model/SampleRepository.h"

using order_system::Sample;
using order_system::SampleRepository;

namespace {

namespace fs = std::filesystem;

class SampleRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        testDir_ = fs::temp_directory_path() / "SampleOrderSystemTest" /
            ::testing::UnitTest::GetInstance()->current_test_info()->name();

        std::error_code ec;
        fs::remove_all(testDir_, ec);
        fs::create_directories(testDir_, ec);

        jsonFilePath_ = testDir_ / "samples.json";
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(testDir_, ec);
    }

    fs::path testDir_;
    fs::path jsonFilePath_;
};

}  // namespace

// 정상 등록 시 sampleId 는 자동 채번되고 stockQuantity 는 0으로 초기화된다.
TEST_F(SampleRepositoryTest, RegisterAssignsSequentialIdAndZeroStock)
{
    SampleRepository repository(jsonFilePath_);

    Sample first = repository.Register("SampleA", 30, 0.9);
    Sample second = repository.Register("SampleB", 20, 0.8);

    EXPECT_EQ(first.GetSampleId(), 1);
    EXPECT_EQ(first.GetStockQuantity(), 0);
    EXPECT_EQ(second.GetSampleId(), 2);
    EXPECT_EQ(second.GetStockQuantity(), 0);
}

// 이름이 중복이면 등록을 거부한다.
TEST_F(SampleRepositoryTest, RejectsDuplicateName)
{
    SampleRepository repository(jsonFilePath_);
    repository.Register("SampleA", 30, 0.9);

    EXPECT_THROW(repository.Register("SampleA", 10, 0.5), std::invalid_argument);
}

// 이름 부분검색: 검색어가 포함된 시료만 반환한다.
TEST_F(SampleRepositoryTest, SearchByNameReturnsMatchingSamplesOnly)
{
    SampleRepository repository(jsonFilePath_);
    repository.Register("AlphaSample", 30, 0.9);
    repository.Register("BetaSample", 20, 0.8);
    repository.Register("Gamma", 10, 0.7);

    std::vector<Sample> results = repository.SearchByName("Sample");

    ASSERT_EQ(results.size(), 2u);
    EXPECT_EQ(results[0].GetName(), "AlphaSample");
    EXPECT_EQ(results[1].GetName(), "BetaSample");
}

// 검색 결과가 없으면 빈 목록을 반환한다.
TEST_F(SampleRepositoryTest, SearchByNameReturnsEmptyWhenNoMatch)
{
    SampleRepository repository(jsonFilePath_);
    repository.Register("AlphaSample", 30, 0.9);

    std::vector<Sample> results = repository.SearchByName("NotExist");

    EXPECT_TRUE(results.empty());
}

// ID 조회: 존재하는 ID 는 해당 시료를 반환한다.
TEST_F(SampleRepositoryTest, FindByIdReturnsSampleWhenExists)
{
    SampleRepository repository(jsonFilePath_);
    Sample registered = repository.Register("SampleA", 30, 0.9);

    std::optional<Sample> found = repository.FindById(registered.GetSampleId());

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->GetName(), "SampleA");
}

// ID 조회: 존재하지 않는 ID 는 값이 없음을 반환한다.
TEST_F(SampleRepositoryTest, FindByIdReturnsNulloptWhenNotExists)
{
    SampleRepository repository(jsonFilePath_);
    repository.Register("SampleA", 30, 0.9);

    std::optional<Sample> found = repository.FindById(9999);

    EXPECT_FALSE(found.has_value());
}

// 전체 조회: 등록된 모든 시료를 반환한다.
TEST_F(SampleRepositoryTest, GetAllReturnsAllRegisteredSamples)
{
    SampleRepository repository(jsonFilePath_);
    repository.Register("SampleA", 30, 0.9);
    repository.Register("SampleB", 20, 0.8);

    std::vector<Sample> all = repository.GetAll();

    EXPECT_EQ(all.size(), 2u);
}

// JSON 파일이 없으면 빈 상태로 시작한다.
TEST_F(SampleRepositoryTest, StartsEmptyWhenJsonFileDoesNotExist)
{
    SampleRepository repository(jsonFilePath_);

    EXPECT_TRUE(repository.GetAll().empty());
}

// 등록 직후 JSON 파일에 반영된다.
TEST_F(SampleRepositoryTest, RegisterPersistsToJsonFileImmediately)
{
    SampleRepository repository(jsonFilePath_);

    repository.Register("SampleA", 30, 0.9);

    EXPECT_TRUE(fs::exists(jsonFilePath_));
}

// 재고 증가 후 Repository 를 재생성해도 반영된 값이 유지된다.
TEST_F(SampleRepositoryTest, IncreaseStockPersistsToJsonFile)
{
    int sampleId = 0;
    {
        SampleRepository repository(jsonFilePath_);
        Sample registered = repository.Register("SampleA", 30, 0.9);
        sampleId = registered.GetSampleId();

        repository.IncreaseStock(sampleId, 10);
    }

    SampleRepository reloaded(jsonFilePath_);
    std::optional<Sample> found = reloaded.FindById(sampleId);

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->GetStockQuantity(), 10);
}

// 재고 감소 후 Repository 를 재생성해도 반영된 값이 유지된다.
TEST_F(SampleRepositoryTest, DecreaseStockPersistsToJsonFile)
{
    int sampleId = 0;
    {
        SampleRepository repository(jsonFilePath_);
        Sample registered = repository.Register("SampleA", 30, 0.9);
        sampleId = registered.GetSampleId();

        repository.IncreaseStock(sampleId, 10);
        repository.DecreaseStock(sampleId, 4);
    }

    SampleRepository reloaded(jsonFilePath_);
    std::optional<Sample> found = reloaded.FindById(sampleId);

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->GetStockQuantity(), 6);
}

// 기존 JSON 파일이 있는 상태로 Repository 를 생성하면 그 데이터가 로드된다.
TEST_F(SampleRepositoryTest, LoadsExistingDataFromJsonFileOnConstruction)
{
    {
        SampleRepository repository(jsonFilePath_);
        repository.Register("SampleA", 30, 0.9);
        repository.Register("SampleB", 20, 0.8);
    }

    SampleRepository reloaded(jsonFilePath_);
    std::vector<Sample> all = reloaded.GetAll();

    EXPECT_EQ(all.size(), 2u);
}
