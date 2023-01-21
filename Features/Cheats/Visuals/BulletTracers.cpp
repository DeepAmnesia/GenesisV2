#include "BulletTracers.h"
#include "..\..\sdk\misc\BeamInfo_t.hpp"
#include "..\ragebot\ragebot.h"
#include "..\..\utils\ctx.hpp"
#include "..\misc\logs.h"
void bullettracers::RenderBeam(player_t* local_player, const Vector& src, const Vector& end, Color color, bool nade)
{
	if (src == ZERO)
		return;

	std::string model_name = XorStr("sprites/");
	model_name += XorStr("purplelaser1.vmt");
	
	BeamInfo_t beam_info;
	beam_info.m_vecStart = src;

	beam_info.m_vecEnd = end;
	beam_info.m_nType = TE_BEAMPOINTS;
	beam_info.m_pszModelName = model_name.c_str();
	beam_info.m_pszHaloName = model_name.c_str();
	beam_info.m_flHaloScale = 0.0;
	beam_info.m_flWidth = 2.0;
	beam_info.m_flEndWidth = 2.0;
	beam_info.m_flFadeLength = 0.0f;
	beam_info.m_flAmplitude = 0;
	beam_info.m_flBrightness = color.a();
	beam_info.m_flSpeed = 0.0f;
	beam_info.m_nStartFrame = 0.0;
	beam_info.m_flFrameRate = 0.0;
	beam_info.m_flRed = color.r();
	beam_info.m_flGreen = color.g();
	beam_info.m_flBlue = color.b();
	beam_info.m_nSegments = 2;
	beam_info.m_bRenderable = true;
	beam_info.m_flLife = 2.0f;
	beam_info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

	if (local_player == g_ctx.local())
	{
		if (cfg::g_cfg.visuals.local_bullet_tracers_type == 0)
		{
			auto beam = m_viewrenderbeams()->CreateBeamPoints(beam_info);

			if (beam)
				m_viewrenderbeams()->DrawBeam(beam);
		}
		if (cfg::g_cfg.visuals.local_bullet_tracers_type == 1)
		{
			m_debugoverlay()->AddLineOverlay(src, end, color.r(), color.g(), color.b(), true, 2.f);
		}
	}
	if (local_player != g_ctx.local() && local_player->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
	{
		if (cfg::g_cfg.visuals.team_bullet_tracers_type == 0)
		{
			auto beam = m_viewrenderbeams()->CreateBeamPoints(beam_info);

			if (beam)
				m_viewrenderbeams()->DrawBeam(beam);
		}
		if (cfg::g_cfg.visuals.team_bullet_tracers_type == 1)
		{
			m_debugoverlay()->AddLineOverlay(src, end, color.r(), color.g(), color.b(), true, 2.f);
		}
	}
	if (local_player->m_iTeamNum() != g_ctx.local()->m_iTeamNum() || m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1)
	{
		if (cfg::g_cfg.visuals.enemy_bullet_tracers_type == 0)
		{
			auto beam = m_viewrenderbeams()->CreateBeamPoints(beam_info);

			if (beam)
				m_viewrenderbeams()->DrawBeam(beam);
		}
		if (cfg::g_cfg.visuals.enemy_bullet_tracers_type == 1)
		{
			m_debugoverlay()->AddLineOverlay(src, end, color.r(), color.g(), color.b(), true, 2.f);
		}
	}
}
void bullettracers::events(IGameEvent* event)
{
	auto event_name = event->GetName();

	if (!strcmp(event_name, crypt_str("bullet_impact")))
	{
		auto user_id = event->GetInt(crypt_str("userid"));
		auto user = m_engine()->GetPlayerForUserID(user_id);

		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(user));

		if (e->valid(false))
		{
			if (e == g_ctx.local())
			{
				auto new_record = true;
				Vector position(event->GetFloat(crypt_str("x")), event->GetFloat(crypt_str("y")), event->GetFloat(crypt_str("z")));

				for (auto& current : impacts)
				{
					if (e == current.e)
					{
						new_record = false;

						current.impact_position = position;
						current.time = m_globals()->m_curtime;
					}
				}

				if (new_record)
					impacts.push_back(
						impact_data
						{
							e,
							position,
							m_globals()->m_curtime
						});
			}
			else if (e->m_iTeamNum() != g_ctx.local()->m_iTeamNum() || m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1)
			{
				auto new_record = true;
				Vector position(event->GetFloat(crypt_str("x")), event->GetFloat(crypt_str("y")), event->GetFloat(crypt_str("z")));

				for (auto& current : impacts)
				{
					if (e == current.e)
					{
						new_record = false;

						current.impact_position = position;
						current.time = m_globals()->m_curtime;
					}
				}

				if (new_record)
					impacts.push_back(
						impact_data
						{
							e,
							position,
							m_globals()->m_curtime
						});
			}
			else if (e->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() != 1)
			{
				auto new_record = true;
				Vector position(event->GetFloat(crypt_str("x")), event->GetFloat(crypt_str("y")), event->GetFloat(crypt_str("z")));

				for (auto& current : impacts)
				{
					if (e == current.e)
					{
						new_record = false;

						current.impact_position = position;
						current.time = m_globals()->m_curtime;
					}
				}

				if (new_record)
					impacts.push_back(
						impact_data
						{
							e,
							position,
							m_globals()->m_curtime
						});
			}
		}
	}
}

void bullettracers::draw_beams()
{
	if (impacts.empty())
		return;

	while (!impacts.empty())
	{
		if (impacts.begin()->impact_position.IsZero())
		{
			impacts.erase(impacts.begin());
			continue;
		}

		if (fabs(m_globals()->m_curtime - impacts.begin()->time) > 2.0f)
		{
			impacts.erase(impacts.begin());
			continue;
		}

		if (!impacts.begin()->e->valid(false))
		{
			impacts.erase(impacts.begin());
			continue;
		}

		if (TIME_TO_TICKS(m_globals()->m_curtime) > TIME_TO_TICKS(impacts.begin()->time))
		{
			auto color = cfg::g_cfg.visuals.enemy_bullet_tracer_color;

			if (impacts.begin()->e == g_ctx.local())
			{
				if (!cfg::g_cfg.visuals.local_bullet_tracer)
				{
					impacts.erase(impacts.begin());
					continue;
				}

				color = cfg::g_cfg.visuals.local_bullet_tracer_color;
			}
			else if ((impacts.begin()->e->m_iTeamNum() != g_ctx.local()->m_iTeamNum() || m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1 ) )
			{
				if (!cfg::g_cfg.visuals.enemy_bullet_tracer)
				{
					impacts.erase(impacts.begin());
					continue;
				}

				color = cfg::g_cfg.visuals.enemy_bullet_tracer_color;
			}
			else if (impacts.begin()->e->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() != 1)
			{
				if (!cfg::g_cfg.visuals.team_bullet_tracer)
				{
					impacts.erase(impacts.begin());
					continue;
				}

				color = cfg::g_cfg.visuals.team_bullet_tracer_color;
			}

			RenderBeam(impacts.begin()->e, impacts.begin()->e == g_ctx.local() ? impacts.begin()->e->get_eye_position() : impacts.begin()->e->get_shoot_position(), impacts.begin()->impact_position, color, false);
			impacts.erase(impacts.begin());
			continue;
		}

		break;
	}
}