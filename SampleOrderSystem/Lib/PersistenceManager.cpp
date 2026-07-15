#include "PersistenceManager.h"

#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

PersistenceManager::PersistenceManager(fs::path baseDirectory)
    : m_baseDirectory(std::move(baseDirectory))
{
    fs::create_directories(m_baseDirectory);
}

fs::path PersistenceManager::DataPathFor(const std::string& key) const
{
    return m_baseDirectory / (key + ".json");
}

fs::path PersistenceManager::BackupPathFor(const std::string& key) const
{
    return m_baseDirectory / (key + ".json.bak");
}

void PersistenceManager::AtomicWrite(const fs::path& target, const std::string& content)
{
    fs::path tmpPath = target;
    tmpPath += ".tmp";

    {
        std::ofstream file(tmpPath, std::ios::binary | std::ios::trunc);
        if (!file)
        {
            throw std::runtime_error("임시 파일을 열 수 없습니다: " + tmpPath.string());
        }
        file << content;
        if (!file)
        {
            throw std::runtime_error("임시 파일 쓰기에 실패했습니다: " + tmpPath.string());
        }
    } // 파일을 닫아 내용이 디스크로 flush된 뒤에 rename 진행

    fs::rename(tmpPath, target);
}

void PersistenceManager::Save(const std::string& key, const json::JsonValue& data, int prettyIndent) const
{
    fs::create_directories(m_baseDirectory);

    const fs::path dataPath = DataPathFor(key);
    const fs::path backupPath = BackupPathFor(key);

    if (fs::exists(dataPath))
    {
        std::error_code ec;
        fs::copy_file(dataPath, backupPath, fs::copy_options::overwrite_existing, ec);
    }

    AtomicWrite(dataPath, data.Dump(prettyIndent));
}

bool PersistenceManager::TryLoad(const std::string& key, json::JsonValue& outData) const
{
    const fs::path dataPath = DataPathFor(key);

    if (fs::exists(dataPath))
    {
        try
        {
            outData = json::JsonValue::LoadFromFile(dataPath);
            return true;
        }
        catch (const std::exception&)
        {
            // 메인 파일이 손상된 경우 백업으로 복구를 시도한다.
        }
    }

    const fs::path backupPath = BackupPathFor(key);
    if (fs::exists(backupPath))
    {
        try
        {
            outData = json::JsonValue::LoadFromFile(backupPath);

            std::error_code ec;
            fs::copy_file(backupPath, dataPath, fs::copy_options::overwrite_existing, ec);

            return true;
        }
        catch (const std::exception&)
        {
        }
    }

    return false;
}

json::JsonValue PersistenceManager::Load(const std::string& key) const
{
    json::JsonValue result;
    if (!TryLoad(key, result))
    {
        throw std::runtime_error("데이터를 불러올 수 없습니다 (메인/백업 모두 없거나 손상됨): " + key);
    }
    return result;
}

bool PersistenceManager::Exists(const std::string& key) const
{
    return fs::exists(DataPathFor(key));
}

bool PersistenceManager::Remove(const std::string& key) const
{
    std::error_code ec;
    bool removedData = fs::remove(DataPathFor(key), ec);
    bool removedBackup = fs::remove(BackupPathFor(key), ec);
    return removedData || removedBackup;
}
