#pragma once

#include "Lib/Json.h"

#include <filesystem>
#include <string>

// json::JsonValue 기반으로 key 단위 데이터를 파일에 저장/로드하는 관리자.
// 프로그램이 저장 도중 종료되어도 기존 데이터가 손상되지 않도록
// 임시 파일 + 원자적 rename, 백업본 유지, 백업을 통한 자동 복구를 제공한다.
class PersistenceManager
{
public:
    // baseDirectory가 존재하지 않으면 생성한다.
    explicit PersistenceManager(std::filesystem::path baseDirectory);

    // key로 데이터를 저장한다.
    // 1) 기존 파일이 있으면 백업으로 보관
    // 2) 임시 파일에 쓴 뒤 원자적으로 rename하여 쓰기 도중 중단되어도 기존 파일이 보존되도록 함
    void Save(const std::string& key, const json::JsonValue& data, int prettyIndent = 2) const;

    // key에 해당하는 데이터를 불러온다. 메인 파일과 백업 모두 없거나 손상된 경우 예외를 던진다.
    json::JsonValue Load(const std::string& key) const;

    // Load와 동일하지만 실패 시 예외 대신 false를 반환한다.
    // 메인 파일이 손상되었거나 없는 경우 백업에서 복구를 시도하고,
    // 백업 복구에 성공하면 메인 파일도 백업 내용으로 되살린다.
    bool TryLoad(const std::string& key, json::JsonValue& outData) const;

    bool Exists(const std::string& key) const;

    // 메인 파일과 백업 파일을 모두 삭제한다. 하나라도 삭제되었으면 true.
    bool Remove(const std::string& key) const;

    std::filesystem::path DataPathFor(const std::string& key) const;
    std::filesystem::path BackupPathFor(const std::string& key) const;

private:
    static void AtomicWrite(const std::filesystem::path& target, const std::string& content);

    std::filesystem::path m_baseDirectory;
};
