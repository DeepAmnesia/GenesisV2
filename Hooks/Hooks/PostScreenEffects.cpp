// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\Features\cheats\visuals\OtherEsp.h"
#include "..\..\Features\cheats\misc\fakelag.h"
#include "..\..\Features\cheats\Visuals\HitChamsHandler.h"

using DoPostScreenEffects_t = void(__thiscall*)(void*, CViewSetup*);

void __fastcall hooks::hkPostScreenEffects(void* thisptr, void* edx, CViewSetup* setup)
{
	static auto original_fn = clientmode_hook->get_func_address <DoPostScreenEffects_t>(44);

	if (!cfg::g_cfg.player.enable)
		return original_fn(thisptr, setup);

	if (!g_ctx.local())
		return original_fn(thisptr, setup);

	if (cfg::g_cfg.ragebot.enable)
	    hit_chams::get().draw_hit_chams();

	for (auto i = 0; i < m_glow()->m_GlowObjectDefinitions.Count(); i++) //-V807
	{
		if (m_glow()->m_GlowObjectDefinitions[i].IsUnused())
			continue;

		auto object = &m_glow()->m_GlowObjectDefinitions[i];
		auto entity = object->GetEnt();

		if (!entity)
			continue;

		auto client_class = entity->GetClientClass();

		if (!client_class)
			continue;

		if (entity->is_player())
		{
			auto e = (player_t*)entity;
			auto should_glow = false;

			if (!e->valid(false, false))
				continue;

			auto type = ENEMY;

			if (e == g_ctx.local())
				type = LOCAL;
			else if (e->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && !m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1)
				type = TEAM;

			float color[4];

			if (cfg::g_cfg.player.type[ENEMY].glow && type == ENEMY)
			{
				should_glow = true;

				color[0] = cfg::g_cfg.player.type[ENEMY].glow_color[0] / 255.0f;
				color[1] = cfg::g_cfg.player.type[ENEMY].glow_color[1] / 255.0f;
				color[2] = cfg::g_cfg.player.type[ENEMY].glow_color[2] / 255.0f;
				color[3] = cfg::g_cfg.player.type[ENEMY].glow_color[3] / 255.0f;
			}
			else if (cfg::g_cfg.player.type[TEAM].glow && type == TEAM)
			{
				should_glow = true;

				color[0] = cfg::g_cfg.player.type[TEAM].glow_color[0] / 255.0f;
				color[1] = cfg::g_cfg.player.type[TEAM].glow_color[1] / 255.0f;
				color[2] = cfg::g_cfg.player.type[TEAM].glow_color[2] / 255.0f;
				color[3] = cfg::g_cfg.player.type[TEAM].glow_color[3] / 255.0f;
			}
			else if (cfg::g_cfg.player.type[LOCAL].glow && type == LOCAL)
			{
				should_glow = true;

				color[0] = cfg::g_cfg.player.type[LOCAL].glow_color[0] / 255.0f;
				color[1] = cfg::g_cfg.player.type[LOCAL].glow_color[1] / 255.0f;
				color[2] = cfg::g_cfg.player.type[LOCAL].glow_color[2] / 255.0f;
				color[3] = cfg::g_cfg.player.type[LOCAL].glow_color[3] / 255.0f;
			}

			if (!should_glow)
				continue;

			object->Set
			(
				color[0],
				color[1],
				color[2],
				color[3],
				1.0f,
				0
			);
		}
		else if (cfg::g_cfg.visuals.weapon[WEAPON_GLOW] && (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponZoneRepulsor)) //-V648
		{
			float color[4] =
			{
				color[0] = cfg::g_cfg.visuals.weapon_glow_color[0] / 255.0f,
				color[1] = cfg::g_cfg.visuals.weapon_glow_color[1] / 255.0f,
				color[2] = cfg::g_cfg.visuals.weapon_glow_color[2] / 255.0f,
				color[3] = cfg::g_cfg.visuals.weapon_glow_color[3] / 255.0f
			};

			object->Set
			(
				color[0],
				color[1],
				color[2],
				color[3],
				1.0f,
				0
			);
		}
		else if (cfg::g_cfg.visuals.grenade_esp[GRENADE_GLOW])
		{
			auto model = entity->GetModel();

			if (!model)
				continue;

			auto studio_model = m_modelinfo()->GetStudioModel(model);

			if (!studio_model)
				continue;

			std::string name = studio_model->szName;

			if (name.find("flashbang") == std::string::npos && name.find("smokegrenade") == std::string::npos && name.find("incendiarygrenade") == std::string::npos && name.find("molotov") == std::string::npos && name.find("fraggrenade") == std::string::npos && name.find("decoy") == std::string::npos)
				continue;

			if (name.find("thrown") != std::string::npos ||
				client_class->m_ClassID == CBaseCSGrenadeProjectile || client_class->m_ClassID == CDecoyProjectile || client_class->m_ClassID == CMolotovProjectile)
			{
				float color[4] =
				{
					color[0] = cfg::g_cfg.visuals.grenade_glow_color[0] / 255.0f,
					color[1] = cfg::g_cfg.visuals.grenade_glow_color[1] / 255.0f,
					color[2] = cfg::g_cfg.visuals.grenade_glow_color[2] / 255.0f,
					color[3] = cfg::g_cfg.visuals.grenade_glow_color[3] / 255.0f
				};

				object->Set
				(
					color[0],
					color[1],
					color[2],
					color[3],
					1.0f,
					0
				);
			}
			else if (name.find("dropped") != std::string::npos && cfg::g_cfg.visuals.weapon[WEAPON_GLOW])
			{
				float color[4] =
				{
					color[0] = cfg::g_cfg.visuals.weapon_glow_color[0] / 255.0f,
					color[1] = cfg::g_cfg.visuals.weapon_glow_color[1] / 255.0f,
					color[2] = cfg::g_cfg.visuals.weapon_glow_color[2] / 255.0f,
					color[3] = cfg::g_cfg.visuals.weapon_glow_color[3] / 255.0f
				};

				object->Set
				(
					color[0],
					color[1],
					color[2],
					color[3],
					1.0f,
					0
				);
			}

		}
	}

	original_fn(thisptr, setup);
}