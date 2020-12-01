//#include "xbyak/xbyak.h"

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface * a_skse, SKSE::PluginInfo * a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= "MyFirstPlugin.log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info("MyFirstPlugin v1.0.0");

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "MyFirstPlugin";
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

static bool isHeavy = FALSE;
//auto& trampoline = SKSE::GetTrampoline();

class Loki_SinkOrSwim {

public:
	struct isSwimmingHook_Code : Xbyak::CodeGenerator {

		REL::Relocation<uint64_t> _ActorUpdate{ REL::ID(36357) };
		REL::Relocation<uint64_t> isSwimmingVariable{ REL::ID(252176) };

		isSwimmingHook_Code() {

			Xbyak::Label _OurReturn;
			Xbyak::Label retLabel;
			//Xbyak::Label _isSwimmingVariable;

			setae(r13b);
			cmp(byte[isHeavy], 0x01);
			jne(retLabel);
			mov(r13b, 0x00);

			L(retLabel);
			comiss(xmm6, dword[isSwimmingVariable.address()]);
			//comiss(xmm6, ptr[rip + _isSwimmingVariable]);
			jmp(ptr[rip + _OurReturn]);

			L(_OurReturn);
			dq(_ActorUpdate.address() + 0x6D3 + 0x25);

			//L(_isSwimmingVariable);
			//dq(isSwimmingVariable.address());

		};

	};
	void* CodeAllocation(Xbyak::CodeGenerator& a_code, SKSE::Trampoline* t_ptr) 
	{
		auto result = t_ptr->allocate(a_code.getSize());
		std::memcpy(result, a_code.getCode(), a_code.getSize());
		return result;

	}
	static void DoWaterHook(SKSE::Trampoline& t_ptr) {

		REL::Relocation<std::uintptr_t> ActorUpdate{ REL::ID(36357) };
		//auto& trampoline = SKSE::GetTrampoline();
		_GetSubmergeLevel = t_ptr.write_call<5>(ActorUpdate.address() + 0x6D3, GetSubmergeLevel);
		printf("-> %llx \n", _GetSubmergeLevel.address());
		printf("-> %llx \n", ActorUpdate.address() + 0x6D3);

	};
	static void DoIsSwimmingHook(SKSE::Trampoline& t_ptr) {
	
		//REL::Relocation<std::uintptr_t> ActorUpdate{ REL::ID(36357) };
		//auto& trampoline = SKSE::GetTrampoline();

		//printf("-> %llx \n", ActorUpdate.address() + 0x6ED);
		isSwimmingHook_Code code;
		code.ready();
		REL::Relocation<std::uintptr_t> ActorUpdate{ REL::ID(36357) };
		printf("-> %llx \n", ActorUpdate.address() + 0x6ED);

		Loki_SinkOrSwim* L_SOS = new Loki_SinkOrSwim();
		_SetActorSwimming = t_ptr.write_branch<6>(ActorUpdate.address() + 0x6ED, L_SOS->CodeAllocation(code, &t_ptr));
		printf("-> %llx \n", _SetActorSwimming.address());
		printf("-> %llx \n", ActorUpdate.address() + 0x6ED);
		// +0x1A
	
	};

private:
	static float GetSubmergeLevel(RE::Actor* a_this, float a_zPos, RE::TESObjectCELL* a_cell) {
			
		float submergedLevel = _GetSubmergeLevel(a_this, a_zPos, a_cell); // call to the OG
		static RE::SpellItem* WaterSlowdownSmol = NULL;
		static RE::SpellItem* WaterSlowdownBeeg = NULL;
		static RE::SpellItem* WaterSlowdownSwim = NULL;
		static RE::SpellItem* WaterSlowdownSink = NULL;
		//static bool isHeavy = FALSE;
		auto dataHandler = RE::TESDataHandler::GetSingleton();

		if (dataHandler) {
			WaterSlowdownSmol = dataHandler->LookupForm<RE::SpellItem>(0xD64, "SinkOrSwim.esp");
			WaterSlowdownBeeg = dataHandler->LookupForm<RE::SpellItem>(0xD65, "SinkOrSwim.esp");
			WaterSlowdownSwim = dataHandler->LookupForm<RE::SpellItem>(0xD67, "SinkOrSwim.esp");
			WaterSlowdownSink = dataHandler->LookupForm<RE::SpellItem>(0xD69, "SinkOrSwim.esp");
		}

		auto activeEffects = a_this->GetActiveEffectList();
		isHeavy = FALSE;
		if (submergedLevel >= 0.68) {
			a_this->RemoveSpell(WaterSlowdownBeeg);
			a_this->RemoveSpell(WaterSlowdownSmol);
			a_this->AddSpell(WaterSlowdownSwim);
			a_this->AddSpell(WaterSlowdownSink);

			if (activeEffects) {
				for (auto& ae : *activeEffects) {
					if (ae && ae->spell == WaterSlowdownSink) {
						if (ae->flags.none(RE::ActiveEffect::Flag::kInactive) && ae->flags.none(RE::ActiveEffect::Flag::kDispelled)) {
							isHeavy = TRUE;
						}
					}
				}
			}
		} else if (submergedLevel >= 0.43) {
			a_this->RemoveSpell(WaterSlowdownSmol);
			a_this->RemoveSpell(WaterSlowdownSwim);
			a_this->RemoveSpell(WaterSlowdownSink);
			a_this->AddSpell(WaterSlowdownBeeg);
		} else if (submergedLevel >= 0.18) {
			a_this->RemoveSpell(WaterSlowdownBeeg);
			a_this->RemoveSpell(WaterSlowdownSwim);
			a_this->RemoveSpell(WaterSlowdownSink);
			a_this->AddSpell(WaterSlowdownSmol);
		} else {
			a_this->RemoveSpell(WaterSlowdownSwim);
			a_this->RemoveSpell(WaterSlowdownBeeg);
			a_this->RemoveSpell(WaterSlowdownSmol);
			a_this->RemoveSpell(WaterSlowdownSink);
		}

		return submergedLevel;

		/*
		RE::PlayerCharacter::powerAttackTimer;

		auto isPlayerChar = a_this->IsPlayer();
		if (isPlayerChar) {

			RE::PlayerCharacter* t_this;
			t_this->powerAttackTimer
			t_this = RE::PlayerCharacter* a_this;
			t_this->powerAttackTimer = 1.00;


		}*/


	};
	/*
	struct isSwimmingHook_Code : Xbyak::CodeGenerator {

		REL::Relocation<uint64_t> _ActorUpdate{ REL::ID(36357) };
		REL::Relocation<uint64_t> isSwimmingVariable{ REL::ID(252176) };

		isSwimmingHook_Code() {

			Xbyak::Label _OurReturn;
			Xbyak::Label retLabel;
			//Xbyak::Label _isSwimmingVariable;

			setae (r13b);
			cmp (byte[isHeavy], 0x01);
			jne (retLabel);
			mov (r13b, 0x00);

			L(retLabel);
			comiss (xmm6, dword[isSwimmingVariable.address()]);
			//comiss(xmm6, ptr[rip + _isSwimmingVariable]);
			jmp (ptr[rip + _OurReturn]);

			L(_OurReturn);
			dq(_ActorUpdate.address() + 0x6D3 + 0x25);

			//L(_isSwimmingVariable);
			//dq(isSwimmingVariable.address());

		};

	};
	*/
	static inline REL::Relocation<decltype(GetSubmergeLevel)> _GetSubmergeLevel;
	static inline REL::Relocation<uintptr_t> _SetActorSwimming;

};


extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface * a_skse)
{
	logger::info("TestPlugin loaded");

	SKSE::Init(a_skse);
	SKSE::AllocTrampoline(1 << 6);
	auto& trampoline = SKSE::GetTrampoline();

	FILE* fp;
	AllocConsole();
	SetConsoleTitleA("ProxyDLL - Debug Console");
	freopen_s(&fp, "CONOUT$", "w", stdout);

	Loki_SinkOrSwim* L_SOS = new Loki_SinkOrSwim();
	L_SOS->DoWaterHook(trampoline);
	L_SOS->DoIsSwimmingHook(trampoline);

	return true;
}