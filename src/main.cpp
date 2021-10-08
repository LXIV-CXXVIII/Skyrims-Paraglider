#include "C:/dev/ExamplePlugin-CommonLibSSE/build/simpleini-master/SimpleIni.h"
#include "C:/dev/ExamplePlugin-CommonLibSSE/build/SOS.h"

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface * a_skse, SKSE::PluginInfo * a_info)
{
#ifndef NDEBUG
    auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
    auto path = logger::log_directory();
    if (!path) {
        return false;
    }

    *path /= "loki_POISE.log"sv;
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

    logger::info("loki_POISE v1.0.0");

    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "loki_POISE";
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

class SkyrimOnlineService {

public:
    void func(ISteamMatchmaking* a_mm, ISteamNetworking* a_net) {
    
        //const void* ptr = nullptr;
        //a_net->SendP2PPacket(CSteamID::BAnonAccount(), ptr, 20, );
        //a_net->CreateListenSocket();
        a_mm->CreateLobby(ELobbyType::k_ELobbyTypePrivate, 8);
        
    
    }

};

class MagicEffectApplyEventHandler : public RE::BSTEventSink<RE::TESMagicEffectApplyEvent> {

public:
    static MagicEffectApplyEventHandler* GetSingleton() {
        static MagicEffectApplyEventHandler singleton;
        return &singleton;
    }

    auto ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>* a_eventSource) -> RE::BSEventNotifyControl override {

        static RE::EffectSetting* notRevalisGale = NULL;
        static RE::TESDataHandler* dataHandle = NULL;
        if (!dataHandle) {  // we only need this to run once
            dataHandle = RE::TESDataHandler::GetSingleton();
            if (dataHandle) {
                notRevalisGale = dataHandle->LookupForm<RE::EffectSetting>(0x10C68, "Paragliding.esp");
            }
        };

        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }
        //a_event->target->PlayAnimation();
        if (a_event->magicEffect == notRevalisGale->formID) {
            a_event->caster->NotifyAnimationGraph("pebos");
        }

        return RE::BSEventNotifyControl::kContinue;

    }

protected:
    MagicEffectApplyEventHandler() = default;
    MagicEffectApplyEventHandler(const MagicEffectApplyEventHandler&) = delete;
    MagicEffectApplyEventHandler(MagicEffectApplyEventHandler&&) = delete;
    virtual ~MagicEffectApplyEventHandler() = default;

    auto operator=(const MagicEffectApplyEventHandler&)->MagicEffectApplyEventHandler & = delete;
    auto operator=(MagicEffectApplyEventHandler&&)->MagicEffectApplyEventHandler & = delete;

};
/// <summary>
///  determine if UGS
/// </summary>
class TESObjectLoadedEventHandler : public RE::BSTEventSink<RE::TESObjectLoadedEvent> {

public:
    static TESObjectLoadedEventHandler* GetSingleton() {
        static TESObjectLoadedEventHandler singleton;
        return &singleton;
    }

    auto ProcessEvent(const RE::TESObjectLoadedEvent* evn, RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_eventSource) -> RE::BSEventNotifyControl override {
        if (!evn) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto weapon = RE::TESForm::LookupByID<RE::TESObjectWEAP>(evn->formID);
        auto range = weapon->GetMaxRange();
        float maxRange = 0.00f;
        float minRange = 0.00f;
        if (range >= maxRange) {

        }


        //actor->pad0EC = (actor->armorRating + actor->GetBaseActorValue(RE::ActorValue::kHeavyArmor));

        return RE::BSEventNotifyControl::kContinue;
    }

protected:
    TESObjectLoadedEventHandler() = default;
    TESObjectLoadedEventHandler(const TESObjectLoadedEventHandler&) = delete;
    TESObjectLoadedEventHandler(TESObjectLoadedEventHandler&&) = delete;
    virtual ~TESObjectLoadedEventHandler() = default;

    auto operator=(const TESObjectLoadedEventHandler&)->TESObjectLoadedEventHandler & = delete;
    auto operator=(TESObjectLoadedEventHandler&&)->TESObjectLoadedEventHandler & = delete;
};

namespace PoiseMod {

    inline auto DamagePoise(RE::StaticFunctionTag* a_tag, RE::Actor* a_actor, float a_amount) -> void {

        if (!a_actor) {
            return;
        } else {
            float poise = a_actor->pad0EC;
            poise -= a_amount;
            a_actor->pad0EC = poise;
        }

    }

    inline auto RestorePoise(RE::StaticFunctionTag* a_tag, RE::Actor* a_actor, float a_amount) -> void {

        if (!a_actor) {
            return;
        } else {
            float poise = a_actor->pad0EC;
            poise += a_amount;
            a_actor->pad0EC = poise;
        }

    }

    inline auto GetPoise(RE::StaticFunctionTag* a_tag, RE::Actor* a_actor) -> float {

        if (!a_actor) {
            return -1.00f;
        } else {
            return a_actor->pad0EC;
        }

    }

    inline auto GetMaxPoise(RE::StaticFunctionTag* a_tag, RE::Actor* a_actor) -> float {

        if (!a_actor) {
            return -1.00f;
        } else {
            float a_result = (a_actor->equippedWeight + (a_actor->GetBaseActorValue(RE::ActorValue::kHeavyArmor) * 0.30f));
            auto activeEffects = a_actor->GetActiveEffectList();
            static const RE::BSFixedString buffKeyword = "MaxPoiseBuff";
            static const RE::BSFixedString nerfKeyword = "MaxPoiseNerf";
            if (activeEffects) {
                for (auto& ae : *activeEffects) {
                    if (!ae->effect) {
                        break;
                    }
                    if (!ae->effect->baseEffect) {
                        break;
                    }
                    auto keyword = ae->effect->baseEffect->GetDefaultKeyword();
                    if (!keyword) {
                        break;
                    }
                    if (keyword->formEditorID == buffKeyword) {
                        logger::info("MaxPoiseBuff keyword recognized");
                        auto buffPercent = (a_result * ae->effect->effectItem.magnitude);
                        a_result += buffPercent;
                    }
                    if (keyword->formEditorID == nerfKeyword) {
                        logger::info("MaxPoiseNerf keyword recognized");
                        auto buffPercent = (a_result * ae->effect->effectItem.magnitude);
                        a_result -= buffPercent;
                    }
                }

            }
            return a_result;
        }

    }

    inline auto SetPoise(RE::StaticFunctionTag* a_tag, RE::Actor* a_actor, float a_amount) -> void {

        if (!a_actor) {
            return;
        } else {
            a_actor->pad0EC = a_amount;
        }

    }

    bool RegisterFuncsForSKSE(RE::BSScript::IVirtualMachine* a_vm) {

        if (!a_vm) {
            return false;
        }

        a_vm->RegisterFunction("DamagePoise", "Loki_PoiseMod", DamagePoise, false);
        a_vm->RegisterFunction("RestorePoise", "Loki_PoiseMod", RestorePoise, false);
        a_vm->RegisterFunction("GetPoise", "Loki_PoiseMod", GetPoise, false);
        a_vm->RegisterFunction("SetPoise", "Loki_PoiseMod", SetPoise, false);

        return true;

    }

}

class Loki_PluginTools {

public:
    static void* CodeAllocation(Xbyak::CodeGenerator& a_code, SKSE::Trampoline* t_ptr)
    {
        auto result = t_ptr->allocate(a_code.getSize());
        std::memcpy(result, a_code.getCode(), a_code.getSize());
        return result;

    }
    static float lerp(float a, float b, float f) {
        return a + f * (b - a);
    }
    static bool HasWeaponWithKeyword(RE::TESObjectWEAP* a_weap, RE::BSFixedString a_editorID) {
        if (a_weap->keywords) {
            for (std::uint32_t idx = 0; idx < a_weap->numKeywords; ++idx) {
                if (a_weap->keywords[idx] && a_weap->keywords[idx]->formEditorID == a_editorID) {

                }
            }
        }
    }
    static RE::Effect* HasEffectWithKeyword(RE::Actor* a_actor, RE::BSFixedString a_editorID) {
        auto activeEffect = a_actor->GetActiveEffectList();
        if (activeEffect) {
            for (auto& ae : *activeEffect) {
                if (!ae->effect) {
                    return NULL;
                }
                if (!ae->effect->baseEffect) {
                    return NULL;
                }
                auto keywords = ae->effect->baseEffect->keywords;
                if (!keywords) {
                    return NULL;
                }
                //keywords->
                for (std::uint32_t idx = 0; idx < ae->effect->baseEffect->numKeywords; ++idx) {
                    if (keywords[idx] && keywords[idx]->formEditorID == a_editorID) {
                        return ae->effect;
                    }
                }
            }
        }
        return NULL;
    }

private:

protected:

};

class Loki_PoiseMod {
public:
    float BowMult, CrossbowMult, Hand2Hand, OneHandAxe, OneHandDagger, OneHandMace, OneHandSword, TwoHandAxe, TwoHandSword;
    float RapierMult, PikeMult, SpearMult, HalberdMult, QtrStaffMult, CaestusMult, ClawMult, WhipMult;
    float PowerAttackMult, BlockedMult;

    Loki_PoiseMod() {
        CSimpleIniA ini;
        ini.SetUnicode();
        auto filename = L"Data/SKSE/Plugins/loki_POISE.ini";
        SI_Error rc = ini.LoadFile(filename);

        this->PowerAttackMult = ini.GetDoubleValue("BASE", "fPowerAttackMult", -1.00f);
        this->BlockedMult = ini.GetDoubleValue("BASE", "fBlockedMult");
        this->BowMult = ini.GetDoubleValue("BASE", "fBowMult", -1.00f);
        this->CrossbowMult = ini.GetDoubleValue("BASE", "fCrossbowMult", -1.00f);
        this->Hand2Hand = ini.GetDoubleValue("BASE", "fHand2HandMult", -1.00f);
        this->OneHandAxe = ini.GetDoubleValue("BASE", "fOneHandAxeMult", -1.00f);
        this->OneHandDagger = ini.GetDoubleValue("BASE", "fOneHandDaggerMult", -1.00f);
        this->OneHandMace = ini.GetDoubleValue("BASE", "fOneHandMaceMult", -1.00f);
        this->OneHandSword = ini.GetDoubleValue("BASE", "fOneHandSwordMult", -1.00f);
        this->TwoHandAxe = ini.GetDoubleValue("BASE", "fTwoHandAxeMult", -1.00f);
        this->TwoHandSword = ini.GetDoubleValue("BASE", "fTwoHandSwordMult", -1.00f);

        this->RapierMult = ini.GetDoubleValue("ANIMATED_ARMOURY", "fRapierMult", -1.00f);
        this->PikeMult = ini.GetDoubleValue("ANIMATED_ARMOURY", "fPikeMult", -1.00f);
        this->SpearMult = ini.GetDoubleValue("ANIMATED_ARMOURY", "fSpearMult", -1.00f);
        this->HalberdMult = ini.GetDoubleValue("ANIMATED_ARMOURY", "fHalberdMult", -1.00f);
        this->QtrStaffMult = ini.GetDoubleValue("ANIMATED_ARMOURY", "fQtrStaffMult", -1.00f);
        this->CaestusMult = ini.GetDoubleValue("ANIMATED_ARMOURY", "fCaestusMult", -1.00f);
        this->ClawMult = ini.GetDoubleValue("ANIMATED_ARMOURY", "fClawMult", -1.00f);
        this->WhipMult = ini.GetDoubleValue("ANIMATED_ARMOURY", "fWhipMult", -1.00f);
        //auto section1 = ini.GetSection("SETTINGS");
        ///for (auto it = section1->begin(); it != section1->end(); ++it) {
        //    auto first = it->first;
        //    
        //    first.pItem;
        //}
        //section1->
        /**
        std::map<const char*, const char*> iniMap;
        for (auto m : iniMap) {
            if (iniMap["SETTINGS"] == "fRapierMult") {
            
            }
        }
        auto section1 = ini.GetSection("SETTINGS");
        for (auto& sec : section1) {
        
        }
        struct pebis {
            int x;
            int y;
            int z;
        };
        pebis p = {2,3,4};
        for (auto it : p) {
        
        }
        */

    }

    static void InstallStaggerHook() {

        REL::Relocation<std::uintptr_t> StaggerHook{ REL::ID(37673/*628c20*/) };

        auto& trampoline = SKSE::GetTrampoline();
        _ProcessHitEvent = trampoline.write_call<5>(StaggerHook.address() + 0x3C0, ProcessHitEvent);

        logger::info("ProcessHitEvent hook injected");

    }
    static void InstallPoiseRegenHook() {

        REL::Relocation<std::uintptr_t> ActorUpdate{ REL::ID(39375) };

        auto& trampoline = SKSE::GetTrampoline();
        _PoiseRegen = trampoline.write_call<5>(ActorUpdate.address() + 0x8AC, PoiseRegen);

        logger::info("Actor::Update hook injected");

    }
    static void InstallIsActorKnockdownHook() {

        REL::Relocation<std::uintptr_t> isActorKnockdown{ REL::ID(38858) };

        auto& trampoline = SKSE::GetTrampoline();
        _IsActorKnockdown = trampoline.write_call<5>(isActorKnockdown.address() + 0x7E, IsActorKnockdown);

        logger::info("isActorKnockdown hook injected");

    }

private:
    static float CalculatePoiseDamage(RE::HitData& a_hitData, RE::Actor* a_actor) {

        static Loki_PoiseMod* ptr = new Loki_PoiseMod();

        auto weap = a_hitData.weapon;
        bool weapFault = false;
        float a_result = 0.00f;
        if (!weap) {
            return a_hitData.aggressor.get()->GetAttackingWeapon()->GetWeight();
            //
        }
        a_result = weap->weight;

        switch (weap->weaponData.animationType.get()) {
        case RE::WEAPON_TYPE::kBow:
            a_result *= ptr->BowMult;
            break;

        case RE::WEAPON_TYPE::kCrossbow:
            a_result *= ptr->CrossbowMult;
            break;

        case RE::WEAPON_TYPE::kHandToHandMelee:
            if (weap->HasKeyword(0x19AAB3)) {
                a_result *= ptr->CaestusMult;
                break;
            }
            if (weap->HasKeyword(0x19AAB4)) {
                a_result *= ptr->ClawMult;
                break;
            }
            a_result *= ptr->Hand2Hand;
            break;

        case RE::WEAPON_TYPE::kOneHandAxe:
            a_result *= ptr->OneHandAxe;
            break;

        case RE::WEAPON_TYPE::kOneHandDagger:
            a_result *= ptr->OneHandDagger;
            break;

        case RE::WEAPON_TYPE::kOneHandMace:
            a_result *= ptr->OneHandMace;
            break;

        case RE::WEAPON_TYPE::kOneHandSword:
            if (weap->HasKeyword(0x801)) {
                a_result *= ptr->RapierMult;
                break;
            }
            a_result *= ptr->OneHandSword;
            break;

        case RE::WEAPON_TYPE::kTwoHandAxe:
            if (weap->HasKeyword(0xE4580)) {
                a_result *= ptr->HalberdMult;
                break;
            }
            if (weap->HasKeyword(0xE4581)) {
                a_result *= ptr->QtrStaffMult;
                break;
            }
            a_result *= ptr->TwoHandAxe;
            break;

        case RE::WEAPON_TYPE::kTwoHandSword:
            if (weap->HasKeyword(0xE457E)) {
                a_result *= ptr->PikeMult;
                break;
            }
            if (weap->HasKeyword(0xE457F)) {
                a_result *= ptr->SpearMult;
                break;
            }
            a_result *= ptr->TwoHandSword;
            break;

        }

        //auto effect = Loki_PluginTools::HasEffectWithKeyword(a_actor, "PoiseDmgBuff");
        //if (effect) {
        //    auto buffPercent = effect->effectItem.magnitude;
        //}

        auto activeEffects = a_actor->GetActiveEffectList();
        static const RE::BSFixedString buffKeyword = "PoiseDmgBuff";
        static const RE::BSFixedString nerfKeyword = "PoiseDmgNerf";
        static RE::BGSKeyword* poiseDmg = NULL;
        static RE::TESDataHandler* dataHandle = NULL;
        if (!dataHandle) {
            dataHandle = RE::TESDataHandler::GetSingleton();
            if (dataHandle) {
                poiseDmg = dataHandle->LookupForm<RE::BGSKeyword>(0xD63, "loki_POISE.esp");
            }
        }
        if (activeEffects) {
            for (auto& ae : *activeEffects) {
                if (!ae->effect) {
                    break;
                }
                if (!ae->effect->baseEffect) {
                    break;
                }
                //if (ae->effect->baseEffect->HasKeyword(poiseDmg)) {
                //    poiseDmg->formEditorID;
                //}
                auto keyword = ae->effect->baseEffect->GetDefaultKeyword();
                if (!keyword) {
                    break;
                }
                if (keyword->formEditorID == buffKeyword) {
                    logger::info("DamageNerf Keyword Recognized");
                    auto buffPercent = (a_result * ae->effect->effectItem.magnitude);
                    a_result -= buffPercent;  // the actor getting hit takes x% less poise damage
                }
                if (keyword->formEditorID == nerfKeyword) {
                    logger::info("DamageBuff Keyword Recognized");
                    auto buffPercent = (a_result * ae->effect->effectItem.magnitude);
                    a_result += buffPercent;  // the actor getting hit takes x% more poise damage
                }
            }

        }

        if (a_hitData.flags == RE::HitData::Flag::kPowerAttack) {
            a_result *= ptr->PowerAttackMult;
        }

        bool blk;
        a_actor->GetGraphVariableBool("IsBlocking", blk);
        if (blk) {
            a_result *= ptr->BlockedMult;
        }

        return a_result;

    }
    static float CalculateMaxPoise(RE::Actor* a_actor) {

        //static Loki_PoiseMod* ptr = new Loki_PoiseMod();

        //float a_result = ((a_actor->armorRating / 10) + (a_actor->GetActorValue(RE::ActorValue::kStamina) / 2));

        //const RE::BGSKeyword* poiseBuffKeyword = "";
        //if (a_actor->Is)

        float a_result = (a_actor->equippedWeight + (a_actor->GetBaseActorValue(RE::ActorValue::kHeavyArmor) * 0.30f));

        auto activeEffects = a_actor->GetActiveEffectList();
        static const RE::BSFixedString buffKeyword = "MaxPoiseBuff";
        static const RE::BSFixedString nerfKeyword = "MaxPoiseNerf";
        if (activeEffects) {
            for (auto& ae : *activeEffects) {
                if (!ae->effect) {
                    break;
                }
                if (!ae->effect->baseEffect) {
                    break;
                }
                auto keyword = ae->effect->baseEffect->GetDefaultKeyword();
                if (!keyword) {
                    break;
                }
                if (keyword->formEditorID == buffKeyword) {
                    logger::info("MaxPoiseBuff keyword recognized");
                    auto buffPercent = (a_result * ae->effect->effectItem.magnitude);
                    a_result += buffPercent;
                }
                if (keyword->formEditorID == nerfKeyword) {
                    logger::info("MaxPoiseNerf keyword recognized");
                    auto buffPercent = (a_result * ae->effect->effectItem.magnitude);
                    a_result -= buffPercent;
                }
            }

        }


        return a_result;

    }

    static bool IsActorKnockdown(RE::Character* a_this, std::int64_t a_unk) {

        if (a_this->IsPlayerRef()) {
            //a_this->SetGraphVariableFloat("staggerMagnitude", 1.00f);
            a_this->NotifyAnimationGraph("poise_largest_start");
            return false;
        } else {
            return _IsActorKnockdown(a_this, a_unk);
        }

    }

    static RE::hkpCharacterProxy* GetProxy(void* a_1, void* a_2) {

        using func_t = decltype(&Loki_PoiseMod::GetProxy);
        REL::Relocation<func_t> func{ REL::ID(77242) };
        return func(a_1, a_2);

    }

    static void PoiseRegen(RE::Actor* a_actor) {

        _PoiseRegen(a_actor);

        static RE::EffectSetting* poiseDelay = NULL;
        static RE::SpellItem* climbSpell = NULL;
        static RE::TESDataHandler* dataHandle = NULL;
        if (!dataHandle) {
            dataHandle = RE::TESDataHandler::GetSingleton();
            if (dataHandle) {
                poiseDelay = dataHandle->LookupForm<RE::EffectSetting>(0xD63, "loki_POISE.esp");
                climbSpell = dataHandle->LookupForm<RE::SpellItem>(0xD63, "loki_Climbing.esp");
            }
        }

        auto avHealth = a_actor->GetActorValue(RE::ActorValue::kHealth);
        if (avHealth <= 0.00f || a_actor->IsGhost()) { return; }

        if (!a_actor->HasMagicEffect(poiseDelay)) {
            a_actor->pad0EC = Loki_PoiseMod::CalculateMaxPoise(a_actor);
        }


        /**
        auto charCont = a_actor->GetCharController();
        auto surfaceAngle = charCont->surfaceInfo.surfaceNormal.quad.m128_f32[0];
        auto surfaceVelocity = charCont->surfaceInfo.surfaceVelocity;
        if (surfaceAngle >= 0.30f || surfaceAngle <= -0.30f) {
            RE::DebugNotification("angle too high");
            a_actor->AddSpell(climbSpell);
            a_actor->NotifyAnimationGraph("SneakStart");
            if (surfaceAngle > 0.00f) {
                charCont->pitchAngle = surfaceAngle * -1.00f;
                charCont->rollAngle = surfaceAngle;
            } else if (surfaceAngle < 0.00f) {
                charCont->pitchAngle = surfaceAngle;
                charCont->rollAngle = surfaceAngle * -1.00f;
            }
            if (surfaceVelocity.quad.m128_f32[0] >= 2.00f) {
                surfaceVelocity.quad.m128_f32[0] = 2.00f;
            } else if (surfaceVelocity.quad.m128_f32[1] >= 2.00f) {
                surfaceVelocity.quad.m128_f32[1] = 2.00f;
            } else if (surfaceVelocity.quad.m128_f32[2] >= 2.00f) {
                surfaceVelocity.quad.m128_f32[2] = 2.00f;
            }
        } else {
            a_actor->RemoveSpell(climbSpell);
        }
        */
        


        //auto proxy = (RE::hkpCharacterProxy)a_actor->AsReference();
        //RE::RTTI_hkpCharacterProxyCinfo;
        //39375	69e580  Actor::Update Call

        return;

    }

    static void ProcessHitEvent(RE::Actor* a_actor, RE::HitData& a_hitData) {

        static RE::TESDataHandler* dataHandle = NULL;
        static RE::SpellItem* poiseDelay = NULL;
        static RE::BGSKeyword* kCreature = NULL;
        static RE::BGSKeyword* kDragon = NULL;
        static RE::BGSKeyword* kGiant = NULL;
        if (!dataHandle) {
            dataHandle = RE::TESDataHandler::GetSingleton();
            if (dataHandle) {
                poiseDelay = dataHandle->LookupForm<RE::SpellItem>(0xD62, "loki_POISE.esp");
                kCreature = dataHandle->LookupForm<RE::BGSKeyword>(0x13795, "Skyrim.esm");
                kDragon = dataHandle->LookupForm<RE::BGSKeyword>(0x35d59, "Skyrim.esm");
                kGiant = dataHandle->LookupForm<RE::BGSKeyword>(0x10E984, "Skyrim.esm");
            }
        }

        static const RE::BSFixedString ae_Stagger = "staggerStart";
        static const RE::BSFixedString staggerDire = "staggerDirection";
        static const RE::BSFixedString staggerMagn = "staggerMagnitude";
        static const RE::BSFixedString isBlocking = "IsBlocking";
        static const RE::BSFixedString isAttacking = "IsAttacking";

        static const RE::BSFixedString poiseSmall = "poise_small_start";
        static const RE::BSFixedString poiseMed = "poise_med_start";
        static const RE::BSFixedString poiseLarge = "poise_large_start";
        static const RE::BSFixedString poiseLargest = "poise_largest_start";

        using HitFlag = RE::HitData::Flag;

        auto avHealth = a_actor->GetActorValue(RE::ActorValue::kHealth);
        auto avParalysis = a_actor->GetActorValue(RE::ActorValue::kParalysis);
        if (avHealth <= 0.00f || a_actor->IsInKillMove() || a_actor->IsGhost() || avParalysis) { return _ProcessHitEvent(a_actor, a_hitData); }

        float dmg = Loki_PoiseMod::CalculatePoiseDamage(a_hitData, a_actor);
        if (dmg <= 0.00f) dmg = 0.00f;
        a_actor->pad0EC -= dmg;

        float maxPoise = Loki_PoiseMod::CalculateMaxPoise(a_actor);
        auto prcnt25 = maxPoise * 0.25f;
        auto prcnt40 = maxPoise * 0.40f;
        auto prcnt50 = maxPoise * 0.50f;

        auto hitPos = a_hitData.aggressor.get()->GetPosition();
        auto heading = a_actor->GetHeadingAngle(hitPos, false);
        auto stagDir = (heading >= 0.0f) ? heading / 360.0f : (360.0f + heading) / 360.0f;
        if (a_actor->GetHandle() == a_hitData.aggressor) { stagDir = 0.0f; } // 0 when self-hit

        auto a = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
        a->Cast(poiseDelay, false, a_actor, 1.0f, false, 0.0f, 0);

        bool isBlk = false;
        a_actor->GetGraphVariableBool(isBlocking, isBlk);
        if ((float)a_actor->pad0EC <= 0.00f) {
            a_actor->SetGraphVariableFloat(staggerDire, stagDir); // set direction
            a_actor->pad0EC = maxPoise; // remember earlier when we calculated max poise health?
            if (a_actor->HasKeyword(kCreature) || a_actor->HasKeyword(kGiant)) { // if creature, use normal beh
                a_actor->SetGraphVariableFloat(staggerMagn, 1.00f);
                a_actor->NotifyAnimationGraph(ae_Stagger);          // play animation
            } 
            else {
                if (a_hitData.flags == HitFlag::kExplosion || a_hitData.aggressor.get()->HasKeyword(kDragon)
                    || a_hitData.aggressor.get()->HasKeyword(kGiant)) {  // check if explosion, dragon, or giant attack
                    a_actor->NotifyAnimationGraph(poiseLargest);         // if those, play tier 4
                }
                else {
                    a_actor->NotifyAnimationGraph(poiseMed);  // if not those, play tier 2
                }
            }
            if (a_actor->IsPlayerRef()) {
                RE::DebugNotification("! POISE BROKEN !");     // just QoL
            }
        }
        else if ((float)a_actor->pad0EC < prcnt25 || (float)a_actor->pad0EC < 5.00f) {
            a_actor->SetGraphVariableFloat(staggerDire, stagDir); // set direction
            if (a_actor->HasKeyword(kCreature) || a_actor->HasKeyword(kGiant)) { // if creature, use normal beh
                a_actor->SetGraphVariableFloat(staggerMagn, 0.75f);
                a_actor->NotifyAnimationGraph(ae_Stagger);
            } 
            else {
                if (a_hitData.flags == HitFlag::kExplosion || a_hitData.aggressor.get()->HasKeyword(kDragon)
                    || a_hitData.aggressor.get()->HasKeyword(kGiant)) {  // check if explosion, dragon, or giant attack
                    a_actor->NotifyAnimationGraph(poiseLarge);           // if those, play tier 3
                } 
                else {
                    isBlk ? a_hitData.pushBack = 5.00 : a_actor->NotifyAnimationGraph(poiseMed); // if block, set pushback, ! play tier 2
                }
            }
        }
        else if ((float)a_actor->pad0EC < prcnt40 || (float)a_actor->pad0EC < 10.00f) {
            a_actor->SetGraphVariableFloat(staggerDire, stagDir); // set direction
            if (a_actor->HasKeyword(kCreature) || a_actor->HasKeyword(kGiant)) {
                a_actor->SetGraphVariableFloat(staggerMagn, 0.50f);
                a_actor->NotifyAnimationGraph(ae_Stagger);
            }
            else {
                isBlk ? a_hitData.pushBack = 3.75f : a_actor->NotifyAnimationGraph(poiseMed);
            }
            //isBlk ? a_hitData.pushBack = 3.75f : a_actor->NotifyAnimationGraph(poiseMed);
        }
        else if ((float)a_actor->pad0EC < prcnt50 || (float)a_actor->pad0EC < 15.00f) {
            a_actor->SetGraphVariableFloat(staggerDire, stagDir); // set direction
            if (a_actor->HasKeyword(kCreature) || a_actor->HasKeyword(kGiant)) {
                a_actor->SetGraphVariableFloat(staggerMagn, 0.25f);
                a_actor->NotifyAnimationGraph(ae_Stagger);
            } 
            else {
                isBlk ? a_hitData.pushBack = 2.50f : a_actor->NotifyAnimationGraph(poiseSmall);
            }
            //isBlk ? a_hitData.pushBack = 2.50f : a_actor->NotifyAnimationGraph(poiseSmall);
        }

        return _ProcessHitEvent(a_actor, a_hitData);

    };

    static inline REL::Relocation<decltype(PoiseRegen)> _PoiseRegen;
    static inline REL::Relocation<decltype(ProcessHitEvent)> _ProcessHitEvent;
    static inline REL::Relocation<decltype(IsActorKnockdown)> _IsActorKnockdown;

};  


extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface * a_skse)
{
    logger::info("POISE loaded");
    SKSE::Init(a_skse);
    SKSE::AllocTrampoline(64);

    Loki_PoiseMod::InstallStaggerHook();
    Loki_PoiseMod::InstallPoiseRegenHook();
    Loki_PoiseMod::InstallIsActorKnockdownHook();
    SKSE::GetPapyrusInterface()->Register(PoiseMod::RegisterFuncsForSKSE);

    return true;
}