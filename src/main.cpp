#include "C:/dev/simpleini-master/SimpleIni.h"
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface * a_skse, SKSE::PluginInfo * a_info)
{
#ifndef NDEBUG
    auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
    auto path = logger::log_directory();
    if (!path) {
        return false;
    }

    *path /= "Paraglider.log"sv;
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

    logger::info("Paraglider v1.0.0");

    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "Paraglider";
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

static bool isActivate = FALSE;
static bool isParagliding = FALSE;
static float progression = 0.0f;
static float start = 0.0f;

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
        if (a_event->magicEffect == notRevalisGale->formID) {
            start = 0.00f;
            progression = 0.00f;
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

class Loki_Paraglider
{
public:
    float FallSpeed, GaleSpeed;
    Loki_Paraglider() {
        CSimpleIniA ini;
        ini.SetUnicode();
        auto filename = L"Data/SKSE/Plugins/Paraglider.ini";
        SI_Error rc = ini.LoadFile(filename);

        this->FallSpeed = (float)ini.GetDoubleValue("SETTINGS", "fFallSpeed", 0.00f);
        this->GaleSpeed = (float)ini.GetDoubleValue("SETTINGS", "fGaleSpeed", 0.00f);

    }

    static void* CodeAllocation(Xbyak::CodeGenerator& a_code, SKSE::Trampoline* t_ptr)
    {
        auto result = t_ptr->allocate(a_code.getSize());
        std::memcpy(result, a_code.getCode(), a_code.getSize());
        return result;

    }
    static float lerp(float a, float b, float f) {
        return a + f * (b - a);
    }

    static void InstallActivateTrue() {

        REL::Relocation<std::uintptr_t> target{ REL::ID(41346), 0x3C };
        REL::Relocation<std::uintptr_t> addr{ REL::ID(41346), 0x140 };
        //Loki_Paraglider LPG;

        struct Patch : Xbyak::CodeGenerator {

            Patch(std::uintptr_t a_var, std::uintptr_t a_target) {

                Xbyak::Label ourJmp;
                Xbyak::Label ActivateIsTrue;

                mov(byte[rcx + 0x18], 0x1);
                push(rax);
                mov(rax, (uintptr_t)&isActivate);
                cmp(byte[rax], 0x1);
                je(ActivateIsTrue);
                mov(byte[rax], 0x1);
                pop(rax);
                jmp(ptr[rip + ourJmp]);

                L(ActivateIsTrue);
                mov(byte[rax], 0x0);
                pop(rax);
                jmp(ptr[rip + ourJmp]);

                L(ourJmp);
                dq(a_var);

            };

        };

        Patch patch(addr.address(), target.address());
        patch.ready();

        auto& trampoline = SKSE::GetTrampoline();
        trampoline.write_branch<6>(target.address(), Loki_Paraglider::CodeAllocation(patch, &trampoline));

    };

    static void InstallParagliderWatcher() {

        REL::Relocation<std::uintptr_t> ActorUpdate{ REL::ID(39375) };  // +69e580

        auto& trampoline = SKSE::GetTrampoline();
        _Paraglider = trampoline.write_call<5>(ActorUpdate.address() + 0x8AC + 0x08, Paraglider);

    };

    static void AddMGEFApplyEventSink() {

        auto sourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
        if (sourceHolder) { sourceHolder->AddEventSink(MagicEffectApplyEventHandler::GetSingleton()); }

    }

private:
    static void Paraglider(RE::Actor* a_this) {

        _Paraglider(a_this);

        static Loki_Paraglider* lp = NULL;
        if (!lp) {
            lp = new Loki_Paraglider();
        }

        static RE::EffectSetting* notRevalisGale = NULL;
        static RE::TESDataHandler* dataHandle = NULL;
        if (!dataHandle) {  // we only need this to run once
            dataHandle = RE::TESDataHandler::GetSingleton();
            if (dataHandle) {
                notRevalisGale = dataHandle->LookupForm<RE::EffectSetting>(0x10C68, "Paragliding.esp");
            }
        };

        const RE::BSFixedString startPara = "StartPara";
        const RE::BSFixedString endPara = "EndPara";

        if (a_this->IsPlayerTeammate()) {
            
            if (a_this->GetCharController()->context.currentState == RE::hkpCharacterStateType::kInAir) {
                a_this->NotifyAnimationGraph(startPara);
            } 
            else {
                a_this->NotifyAnimationGraph(endPara);
            }

        }

        if (!isActivate) {
            isParagliding = FALSE;
            //const RE::BSFixedString endPara = "EndPara";
            if (a_this->NotifyAnimationGraph(endPara)) {
                RE::hkVector4 hkv;
                a_this->GetCharController()->GetPositionImpl(hkv, false);
                hkv.quad.m128_f32[2] /= 0.0142875;
                a_this->GetCharController()->fallStartHeight = hkv.quad.m128_f32[2];
                a_this->GetCharController()->fallTime = 0.00f;
            }
            progression = 0.00f;
            start = 0.00f;
            return;
        } else {
            //const RE::BSFixedString startPara = "StartPara";
            int hasIt;
            a_this->GetGraphVariableInt("hasparaglider", hasIt);
            if (hasIt) {
                if (a_this->NotifyAnimationGraph(startPara)) { isParagliding = TRUE; }
                if (isParagliding) {
                    RE::hkVector4 hkv;
                    a_this->GetCharController()->GetLinearVelocityImpl(hkv);
                    if (start == 0.0f) {
                        start = hkv.quad.m128_f32[2];
                    }
                    float dest = lp->FallSpeed;
                    if (a_this->HasMagicEffect(notRevalisGale)) {
                        dest = lp->GaleSpeed;
                    }
                    auto a_result = Loki_Paraglider::lerp(start, dest, progression);
                    if (progression < 1.00f) {
                        (a_this->HasMagicEffect(notRevalisGale)) ? progression += 0.01f : progression += 0.025f;
                    }
                    hkv.quad.m128_f32[2] = a_result;
                    a_this->GetCharController()->SetLinearVelocityImpl(hkv);
                }
                if (a_this->GetCharController()->context.currentState == RE::hkpCharacterStateType::kOnGround) {
                    isParagliding = FALSE;
                    isActivate = FALSE;
                }
            }
        }

        return;

    };

    static inline REL::Relocation<decltype(Paraglider)> _Paraglider;

};


extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface * a_skse)
{
    logger::info("Paraglider loaded");

    SKSE::Init(a_skse);
    SKSE::AllocTrampoline(128);

    Loki_Paraglider::InstallActivateTrue();
    Loki_Paraglider::InstallParagliderWatcher();
    Loki_Paraglider::AddMGEFApplyEventSink();

    return true;
}