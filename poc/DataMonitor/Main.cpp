// 저장 데이터(SaveData) 폴더를 실시간으로 감시하며 콘솔에 출력하는 관리자 도구
#include "Lib/Json.h"

#include <windows.h>
#include <conio.h>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>

namespace fs = std::filesystem;

namespace
{
    constexpr auto kPollInterval = std::chrono::milliseconds(500);

    struct WatchedFile
    {
        fs::file_time_type lastWriteTime{};
        std::string pretty;
        std::string error;
    };

    std::string CurrentTimeString()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_s(&tm, &t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    void LoadFile(const fs::path& path, WatchedFile& watched)
    {
        try
        {
            json::JsonValue value = json::JsonValue::LoadFromFile(path);
            watched.pretty = value.Dump(2);
            watched.error.clear();
        }
        catch (const std::exception& e)
        {
            watched.pretty.clear();
            watched.error = e.what();
        }
    }

    // 감시 폴더를 재스캔하여 새로 생기거나 변경된 .json 파일을 다시 읽어들인다.
    void RefreshFiles(const fs::path& watchDir, std::map<std::string, WatchedFile>& files)
    {
        std::error_code ec;
        if (!fs::exists(watchDir, ec) || !fs::is_directory(watchDir, ec))
        {
            files.clear();
            return;
        }

        std::map<std::string, fs::file_time_type> currentEntries;
        for (const auto& entry : fs::directory_iterator(watchDir, ec))
        {
            if (!entry.is_regular_file(ec) || entry.path().extension() != ".json")
            {
                continue;
            }

            std::string name = entry.path().filename().string();
            fs::file_time_type writeTime = entry.last_write_time(ec);
            currentEntries[name] = writeTime;

            auto it = files.find(name);
            if (it == files.end() || it->second.lastWriteTime != writeTime)
            {
                WatchedFile watched;
                watched.lastWriteTime = writeTime;
                LoadFile(entry.path(), watched);
                files[name] = std::move(watched);
            }
        }

        for (auto it = files.begin(); it != files.end();)
        {
            it = currentEntries.count(it->first) ? std::next(it) : files.erase(it);
        }
    }

    void Render(const fs::path& watchDir, const std::map<std::string, WatchedFile>& files)
    {
        system("cls");
        std::cout << "===== DataMonitor : 저장 데이터 실시간 조회 =====\n";
        std::cout << "감시 경로 : " << fs::absolute(watchDir).string() << "\n";
        std::cout << "갱신 시각 : " << CurrentTimeString() << "\n";
        std::cout << "종료하려면 'q' 키를 누르세요.\n";
        std::cout << "------------------------------------------------\n";

        if (files.empty())
        {
            std::cout << "(감시 경로에 .json 파일이 없습니다)\n";
            return;
        }

        for (const auto& [name, watched] : files)
        {
            std::cout << "\n[" << name << "]\n";
            if (!watched.error.empty())
            {
                std::cout << "  파싱 오류: " << watched.error << "\n";
            }
            else
            {
                std::cout << watched.pretty << "\n";
            }
        }
    }
}

int main(int argc, char** argv)
{
    SetConsoleOutputCP(CP_UTF8);

    fs::path watchDir = argc > 1 ? fs::path(argv[1]) : fs::path("SaveData");

    std::map<std::string, WatchedFile> files;

    for (;;)
    {
        RefreshFiles(watchDir, files);
        Render(watchDir, files);

        auto nextTick = std::chrono::steady_clock::now() + kPollInterval;
        while (std::chrono::steady_clock::now() < nextTick)
        {
            if (_kbhit())
            {
                int ch = _getch();
                if (ch == 'q' || ch == 'Q')
                {
                    return 0;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}
