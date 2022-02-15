#include "C:/dev/ExamplePlugin-CommonLibSSE/build/simpleini-master/SimpleIni.h"
#include "C:/dev/ExamplePlugin-CommonLibSSE/build/SkyrimOnlineService.h"
#include "TrueHUDAPI.h"

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface * a_skse, SKSE::PluginInfo * a_info)
{
#ifndef NDEBUG
    auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
    auto path = logger::log_directory();
    if (!path) {
        return false;
    }

    *path /= "loki_SkyrimOnlineService.log"sv;
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
    log->set_level(spdlog::level::trace);
#else
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::info);
#endif

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

    logger::info("loki_SkyrimOnlineService v1");

    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "loki_SkyrimOnlineService";
    a_info->version = 1;

    if (a_skse->IsEditor()) {
        logger::critical("Loaded in editor, marking as incompatible"sv);
        return false;
    }

    const auto ver = a_skse->RuntimeVersion();
    if (ver < SKSE::RUNTIME_1_5_39) {
        logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
        return false;
    }

    return true;
}

/**/
using ItemMap = RE::TESObjectREFR::InventoryItemMap;
class SkyrimOnlineService_Host {

    struct Player {
    public:
        std::string      PlayerName = {};
        std::uint16_t    PlayerLevel = NULL;
        //ItemMap          PlayerInventory = {};
        RE::BGSLocation* PlayerLocation = {};
        bool IsLobbyOpen = false;

    };
    struct ConnectedPlayer {
    public:
        std::string   PlayerInstance = {};
        std::uint64_t SteamID = {};
        std::string   Token = {};
        Player        player;
    };
    struct Pool {
    public:
        std::string ID = {};
        std::list<ConnectedPlayer> Players;
    };
    struct Scope {
    public:
        std::string     App;
        std::list<Pool> Pools;
    };

public:
    virtual HSteamListenSocket CreateListenSocket(const SteamNetworkingIPAddr& localAddress, int nOptions, const SteamNetworkingConfigValue_t* pOptions) {
    
    }
    virtual SteamAPICall_t  CreateCoopLobby(ISteamMatchmaking* a_mm, ELobbyType a_lobbyType) { return _CreateCoopLobbyImpl(a_mm, a_lobbyType); };
    virtual SteamAPICall_t  CreateGroupLobby(ISteamMatchmaking* a_mm, ELobbyType a_lobbyType) { return _CreateGroupLobbyImpl(a_mm, a_lobbyType); };
    virtual SteamAPICall_t  LeaveLobby(ISteamMatchmaking* a_mm, CSteamID a_lobby) { return _LeaveLobbyImpl(a_mm, a_lobby); };
    virtual Player          GetPlayer() { return ConstructPlayerInformation(); };
    virtual RE::NiAVObject* GetPlayer3D() { return _GetPlayer3DImpl(); };

    // members
    bool isNetworkActive = false;

private:
    SteamAPICall_t _CreateCoopLobbyImpl(ISteamMatchmaking* a_mm, ELobbyType a_lobbyType) {
        isNetworkActive = player.IsLobbyOpen ? true : false;
        return a_mm->CreateLobby(a_lobbyType, 2);
    };
    SteamAPICall_t _CreateGroupLobbyImpl(ISteamMatchmaking* a_mm, ELobbyType a_lobbyType) {
        isNetworkActive = player.IsLobbyOpen ? true : false;
        return a_mm->CreateLobby(a_lobbyType, 8);
    };
    SteamAPICall_t _LeaveLobbyImpl(ISteamMatchmaking* a_mm, CSteamID a_lobby) {
        a_mm->LeaveLobby(a_lobby);
    }
    ConnectedPlayer _ConstructConnectedPlayer() {

    }
    Pool _ConstructPool() {
        pool.ID = "Skyrim DND";
        for (auto idx = pool.Players.begin(); idx != pool.Players.end(); ++idx) {
            pool.Players.push_back(_ConstructConnectedPlayer());
        }
    }
    Pool _GetPoolImpl() {
        return pool;
    }
    Player ConstructPlayerInformation() {
        return []() -> Player {
            player.PlayerName = RE::PlayerCharacter::GetSingleton()->GetName();
            player.PlayerLevel = RE::PlayerCharacter::GetSingleton()->GetLevel();
            //player.PlayerInventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
            player.PlayerLocation = RE::PlayerCharacter::GetSingleton()->currentLocation;
            player.IsLobbyOpen = false;
            return player;
        }();
    }
    RE::NiAVObject* _GetPlayer3DImpl() {

        //for (auto idx : inv) {

        //}
    }

    // members
    static Player player;
    static Pool pool;

protected:
    // members

};

class SkyrimOnlineService_DM :
    public SkyrimOnlineService_Host {

    enum class CharacterSlot : std::uint64_t {
        kCharacter0 = 0,
        kCharacter1,
        kCharacter2,
        kCharacter3,
        kCharacter4,
        kCharacter5,
        kCharacter6,
        kCharacter7,
        kCharacter8,
    };

    struct ConnectedCharacter {
    public:
        CharacterSlot charSlot;
        RE::Actor* actor;
        RE::TESFaction* faction;
        float hp;
    };

public:
    // OVERRIDE
    virtual SteamAPICall_t CreateGroupLobby(ISteamMatchmaking* a_mm, ELobbyType a_lobbyType) { return _CreateGroupLobbyImpl(a_mm, a_lobbyType); };
    virtual SteamAPICall_t LeaveLobby(ISteamMatchmaking* a_mm, CSteamID a_lobby) { };
    // ADD
    //virtual ConnectedCharacter GetCharInfo(CharacterSlot a_slot) { return GetCharacterInformation(a_slot); };

private:
    SteamAPICall_t _CreateGroupLobbyImpl(ISteamMatchmaking* a_mm, ELobbyType a_lobbyType) {
        isNetworkActive = this->GetPlayer().IsLobbyOpen ? true : false;
        return a_mm->CreateLobby(a_lobbyType, 9);
    };
    SteamAPICall_t _LeaveLobbyImpl(ISteamMatchmaking* a_mm, CSteamID a_lobby) {
        a_mm->LeaveLobby(a_lobby);
    }
    ConnectedCharacter _ConstructCharacter(CharacterSlot a_slot) {
        return [a_slot]() -> ConnectedCharacter {
            for (auto idx = characters.begin(); idx != characters.end(); ++idx) {
                idx->charSlot = a_slot;
                idx->actor = skyrim_cast<RE::Actor*>(RE::PlayerCharacter::GetSingleton());
                idx->faction = idx->actor->GetCrimeFaction();
                idx->hp = idx->actor->GetActorValue(RE::ActorValue::kHealth);
                //characters.push_back(&idx.operator==());
            };
        }();
    }
    
    ConnectedCharacter GetCharacterInformation(CharacterSlot a_slot) {
        return [a_slot]() -> ConnectedCharacter {
            switch (a_slot) {
            case CharacterSlot::kCharacter0:
                for (auto idx : characters) {
                    if (idx.charSlot == CharacterSlot::kCharacter0) {
                        return idx;
                    }
                }

            case CharacterSlot::kCharacter1:
                for (auto idx : characters) {
                    if (idx.charSlot == CharacterSlot::kCharacter1) {
                        return idx;
                    }
                }
            }

        }();
    }
    
    static std::list<ConnectedCharacter> characters;

};

void func(SkyrimOnlineService_DM* a_dm) {
    //a_dm->CreateGroupLobby();
}

struct Player {

public:
    std::string                         PlayerName = RE::PlayerCharacter::GetSingleton()->GetName();
    std::uint16_t                       PlayerLevel = RE::PlayerCharacter::GetSingleton()->GetLevel();
    RE::TESObjectREFR::InventoryItemMap PlayerInventory = RE::PlayerCharacter::GetSingleton()->GetInventory();
    RE::BGSLocation* PlayerLocation = RE::PlayerCharacter::GetSingleton()->currentLocation;
    //RE::Actor::GetCurrent3D();
    bool IsLobbyOpen = false;

};

struct ConnectedPlayer {

public:
    std::string   PlayerInstance = {};
    std::uint64_t SteamID = {};
    std::string   Token = {};
    Player* player;

};

struct Pool {

public:
    std::string                ID = {};
    //std::list<ConnectedPlayer> Players = ConnectedPlayer;

};

struct Scope {

public:
    std::string     App;
    //std::list<Pool> Pools = Pool;

};


static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {

    switch (message->type) {
    case SKSE::MessagingInterface::kNewGame:
    case SKSE::MessagingInterface::kPostLoadGame: {
        break;
    }
    case SKSE::MessagingInterface::kPostLoad: {
        break;
    }
    default:
        break;
    }

}



extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface * a_skse)
{
    logger::info("Skyrim Online Service loaded");
    SKSE::Init(a_skse);
    SKSE::AllocTrampoline(64);

    return true;
}