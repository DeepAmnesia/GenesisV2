#include "..\..\includes.hpp"
#include "HitMarker.h"
#include "..\..\utils\render.h"

player_t* sget_entity(const int index) { return reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(index)); }

void hitmarker::listener(IGameEvent* game_event)
{
	const auto attacker = m_engine()->GetPlayerForUserID(game_event->GetInt("attacker"));

	const auto victim = m_engine()->GetPlayerForUserID(game_event->GetInt("userid"));

	if (attacker != m_engine()->GetLocalPlayer())
		return;

	if (victim == m_engine()->GetLocalPlayer())
		return;

	const auto player = sget_entity(victim);
	if (!player || (player->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && !m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1))
		return;

}
int bones(int event_bone)
{
	switch (event_bone)
	{
	case 1:
		return CSGOHitboxID::Head;
	case 2:
		return CSGOHitboxID::Chest;
	case 3:
		return CSGOHitboxID::Stomach;
	case 4:
		return CSGOHitboxID::LeftHand;
	case 5:
		return CSGOHitboxID::RightHand;
	case 6:
		return CSGOHitboxID::RightCalf;
	case 7:
		return CSGOHitboxID::LeftCalf;
	default:
		return CSGOHitboxID::Pelvis;
	}
}
void hitmarker::DrawDamageIndicator()////////
{
	for (size_t id = 0; id < hits.size(); id++) 
	{

		if (hits.at(id).pos == ZERO || hits.at(id).time <= m_globals()->m_curtime) 
		{
			hits.erase(hits.begin() + id);
			continue;
		}

		if (hits.at(id).pos != ZERO)
		{
			hits.at(id).pos.z -= (cfg::g_cfg.visuals.damage_marker_speed * (m_globals()->m_curtime - hits.at(id).time));


			if (hits.at(id).time - m_globals()->m_curtime < 1.5f)
				hits.at(id).alpha /= 1.2f;
			Color clr = Color(cfg::g_cfg.visuals.damage_marker_color.r()/255.f, cfg::g_cfg.visuals.damage_marker_color.g() / 255.f, cfg::g_cfg.visuals.damage_marker_color.b() / 255.f, hits.at(id).alpha / 255.f);

			Vector out;
			if (math::world_to_screen(hits.at(id).pos, out) && hits.at(id).time > m_globals()->m_curtime)
				renderer::get().text(renderer::get().damage, static_cast<int>(out.x), static_cast<int>(out.y), clr, true, false, false, true, ("- " + std::to_string(hits.at(id).damage)).c_str());
		}
	}
}
void hitmarker::draw_hits()
{
	DrawDamageIndicator();

}

void hitmarker::add_hit(const hitmarker_t hit)
{
	hits.push_back(hit);
}


