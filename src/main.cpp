#include "C:/dev/ExamplePlugin-CommonLibSSE/build/simpleini-master/SimpleIni.h"

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

  


extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface * a_skse)
{
    logger::info("Skyrim Online Service loaded");
    SKSE::Init(a_skse);
    SKSE::AllocTrampoline(64);

    return true;
}