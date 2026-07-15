#include "PersistenceManager.h"
#include "Lib/Json.h"

#include <Windows.h>
#include <iostream>

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    PersistenceManager persistence("SaveData");

    // 1) 저장
    json::JsonValue player = json::JsonValue::MakeObject();
    player["name"] = json::JsonValue("Hero");
    player["level"] = json::JsonValue(12);
    player["hp"] = json::JsonValue(87.5);

    json::JsonValue inventory = json::JsonValue::MakeArray();
    inventory.Push(json::JsonValue("Sword"));
    inventory.Push(json::JsonValue("Shield"));
    player["inventory"] = inventory;

    persistence.Save("player", player);
    std::cout << "저장 완료: " << persistence.DataPathFor("player").string() << "\n";

    // 2) 불러오기
    json::JsonValue loaded;
    if (persistence.TryLoad("player", loaded))
    {
        std::cout << "불러온 데이터:\n" << loaded.Dump(2) << "\n";
    }
    else
    {
        std::cout << "데이터를 찾을 수 없습니다.\n";
    }

    // 3) 존재 여부 확인
    std::cout << "player 데이터 존재 여부: " << (persistence.Exists("player") ? "true" : "false") << "\n";

    return 0;
}
