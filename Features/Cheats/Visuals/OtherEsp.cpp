// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "OtherEsp.h"
#include "..\ragebot\penetration.h"
#include "..\ragebot\antiaim.h"
#include "..\misc\logs.h"
#include "..\misc\misc.h"
#include "..\lagcompensation\LocalAnimations.h"
#include "..\exploits\TickBase.h"
#include "..\..\utils\render.h"

void otheresp::draw_spread_circle(player_t* local_player) 
{
	if (!cfg::g_cfg.visuals.spread_crosshair)
		return;

	if (!m_engine()->IsInGame() || !m_engine()->IsConnected())
		return;

	if (!local_player || !local_player->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	const auto weapon_info = weapon->get_csweapon_info();
	if (!weapon_info)
		return;

	if (weapon->is_knife() || weapon->is_grenade())
		return;

	auto is_special_weapon = g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_AWP || g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_G3SG1 || g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SCAR20 || g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SSG08;
	auto inaccuracy = weapon_info->flInaccuracyStand;

	if (g_ctx.local()->m_fFlags() & FL_DUCKING)
	{
		if (is_special_weapon)
			inaccuracy = weapon_info->flInaccuracyCrouchAlt;
		else
			inaccuracy = weapon_info->flInaccuracyCrouch;
	}
	else if (is_special_weapon)
		inaccuracy = weapon_info->flInaccuracyStandAlt;

	auto accuracy_raduis = inaccuracy * 550.f;

	if (cfg::g_cfg.visuals.spread_crosshair_type == 0)
	{
		//render::render_circle(static_cast<int>(ImGui::GetIO().DisplaySize.x / 2.0f), static_cast<int>(ImGui::GetIO().DisplaySize.y / 2.0f), accuracy_raduis, static_cast<int>(accuracy_raduis * 3.f), Color::Black(), 2.5f);
		//render::render_circle(static_cast<int>(ImGui::GetIO().DisplaySize.x / 2.0f), static_cast<int>(ImGui::GetIO().DisplaySize.y / 2.0f), accuracy_raduis, static_cast<int>(accuracy_raduis * 3.f), Color(config.color_visuals_spread_crosshair), 2.f);
	}
	else if (cfg::g_cfg.visuals.spread_crosshair_type == 1)
	{
		//render::render_circle_filled(static_cast<float>(ImGui::GetIO().DisplaySize.x / 2.0f), static_cast<float>(ImGui::GetIO().DisplaySize.y / 2.0f), accuracy_raduis, static_cast<float>(accuracy_raduis * 3.f), Color(config.color_visuals_spread_crosshair[0], config.color_visuals_spread_crosshair[1], config.color_visuals_spread_crosshair[2], 0.5f));
		//render::render_circle(static_cast<int>(ImGui::GetIO().DisplaySize.x / 2.0f), static_cast<int>(ImGui::GetIO().DisplaySize.y / 2.0f), accuracy_raduis, static_cast<int>(accuracy_raduis * 3.f), Color(config.color_visuals_spread_crosshair), 2.f);
	}
	else if (cfg::g_cfg.visuals.spread_crosshair_type == 1)
	{
		//render::render_circle_filled(static_cast<float>(ImGui::GetIO().DisplaySize.x / 2.0f), static_cast<float>(ImGui::GetIO().DisplaySize.y / 2.0f), accuracy_raduis, static_cast<float>(accuracy_raduis * 3.f), Color(config.color_visuals_spread_crosshair[0], config.color_visuals_spread_crosshair[1], config.color_visuals_spread_crosshair[2], 0.5f));
		//render::render_circle(static_cast<int>(ImGui::GetIO().DisplaySize.x / 2.0f), static_cast<int>(ImGui::GetIO().DisplaySize.y / 2.0f), accuracy_raduis, static_cast<int>(accuracy_raduis * 3.f), Color(config.color_visuals_spread_crosshair), 2.f);
	}
}

bool otheresp::CanPenetrate(weapon_t* weapon)
{
	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return false;

	Vector view_angles;
	m_engine()->GetViewAngles(view_angles);

	Vector direction;
	math::angle_vectors(view_angles, direction);

	CTraceFilter filter;
	filter.pSkip = g_ctx.local();

	trace_t trace;
	util::trace_line(g_ctx.globals.eye_pos, g_ctx.globals.eye_pos + direction * weapon_info->flRange, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace);

	if (trace.fraction == 1.0f) //-V550
		return false;

	auto eye_pos = g_ctx.globals.eye_pos;
	auto hits = 1;
	auto damage = (float)weapon_info->iDamage;
	auto penetration_power = weapon_info->flPenetration;

	static auto damageReductionBullets = m_cvar()->FindVar(crypt_str("ff_damage_reduction_bullets"));
	static auto damageBulletPenetration = m_cvar()->FindVar(crypt_str("ff_damage_bullet_penetration"));

	return autowall::get().handle_bullet_penetration(weapon_info, trace, eye_pos, direction, hits, damage, penetration_power, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat());
}

void otheresp::PenetrationCrosshair()
{
	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	const auto weapon_info = weapon->get_csweapon_info();
	if (!weapon_info)
		return;

	if (weapon->is_knife() || weapon->is_grenade())
		return;

	CTraceFilter filter;
	filter.pSkip = g_ctx.local();

	Vector view_angles;
	m_engine()->GetViewAngles(view_angles);

	Vector direction;
	math::angle_vectors(view_angles, direction);

	trace_t trace;
	util::trace_line(g_ctx.globals.eye_pos, g_ctx.globals.eye_pos + direction * weapon_info->flRange, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace);

	if (trace.fraction == 1.0f)
		return;

	auto color = Color(237, 42, 28, 59);
	auto color2 = Color(237, 42, 28, 175);

	if (!weapon->is_non_aim() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && CanPenetrate(weapon))
	{
		color = Color(34, 139, 34, 59);
		color2 = Color(34, 139, 34, 175);
	}
	static int width, height;
	m_engine()->GetScreenSize(width, height);

	float angle_z = math::dot_product(Vector(0, 0, 1), trace.plane.normal);
	float invangle_z = math::dot_product(Vector(0, 0, -1), trace.plane.normal);
	float angle_y = math::dot_product(Vector(0, 1, 0), trace.plane.normal);
	float invangle_y = math::dot_product(Vector(0, -1, 0), trace.plane.normal);
	float angle_x = math::dot_product(Vector(1, 0, 0), trace.plane.normal);
	float invangle_x = math::dot_product(Vector(-1, 0, 0), trace.plane.normal);

	//Looks worse with an outline.
	//renderer::get().rect_filled(width / 2 - 2, height / 2 - 2, 5, 5, Color(0, 0, 0, 255));
	if (cfg::g_cfg.visuals.penetration_crosshair[1])
		renderer::get().rect_filled(width / 2 - 1, height / 2 - 1, 3, 3, color2);
	if (cfg::g_cfg.visuals.penetration_crosshair[0])
	{
		if (angle_z > 0.5 || invangle_z > 0.5)
		{
			render::get().filled_rect_world(trace.endpos, Vector2D(5, 5), color, 0);
			render::get().rect_world(trace.endpos, Vector2D(5, 5), color2, 0);
		}
		else if (angle_y > 0.5 || invangle_y > 0.5)
		{
			render::get().filled_rect_world(trace.endpos, Vector2D(5, 5), color, 1);
			render::get().rect_world(trace.endpos, Vector2D(5, 5), color2, 1);
		}
		else if (angle_x > 0.5 || invangle_x > 0.5)
		{
			render::get().filled_rect_world(trace.endpos, Vector2D(5, 5), color, 2);
			render::get().rect_world(trace.endpos, Vector2D(5, 5), color2, 2);
		}
	}

}

void RenderScreenMarker(int w, int h, Color clr) {
	//bottom right
	renderer::get().line(w - 6, h - 6, w - 1, h - 1, clr);
	//top left
	renderer::get().line(w - 6, h + 6, w - 1, h + 1, clr);
	//top right
	renderer::get().line(w + 6, h + 6, w + 1, h + 1, clr);
	//bottom left
	renderer::get().line(w + 6, h - 6, w + 1, h - 1, clr);
}

void RenderWorldMarker(Vector origin, Color clr) {
	//bottom right
	renderer::get().line(origin.x - 6, origin.y - 6, origin.x - 1, origin.y - 1, clr);
	//top left
	renderer::get().line(origin.x - 6, origin.y + 6, origin.x - 1, origin.y + 1, clr);
	//top right
	renderer::get().line(origin.x + 6, origin.y + 6, origin.x + 1, origin.y + 1, clr);
	//bottom left
	renderer::get().line(origin.x + 6, origin.y - 6, origin.x + 1, origin.y - 1, clr);
}

void otheresp::HitMarker()
{
	if (!g_ctx.local()->is_alive())
	{
		hitmarker.hurt_time = FLT_MIN;
		hitmarker.point = ZERO;
		return;
	}

	int w, h = 0;
	m_engine()->GetScreenSize(w, h);
	w = w / 2;
	h = h / 2;

	if (hitmarker.hurt_time > m_globals()->m_realtime)
	{
		auto alpha = 255.f * (hitmarker.hurt_time - m_globals()->m_realtime) / 0.5f;
		//Zeze gay. -Edwin
		
		hitmarker.hurt_color.SetAlpha(alpha);
		if (cfg::g_cfg.visuals.hitmarker_screen)
		RenderScreenMarker(w, h, cfg::g_cfg.visuals.hitmarker_screen_color);
	}

	if (hitmarker.hurt_time_world > m_globals()->m_realtime)
	{
		auto alpha_world = 255.f;

		if (m_globals()->m_realtime > hitmarker.hurt_time_world - 0.5f && hitmarker.hurt_time_world > m_globals()->m_realtime)
		    alpha_world = 255.f * (hitmarker.hurt_time - m_globals()->m_realtime) / .5f;

		Vector origin;
		if (!math::world_to_screen(hitmarker.point, origin))
			return;


		hitmarker.hurt_color.SetAlpha(alpha_world);
		if (cfg::g_cfg.visuals.hitmarker_world)
		RenderWorldMarker(origin, cfg::g_cfg.visuals.hitmarker_world_color);
	}
}

void otheresp::AutoPeekIndicator()
{
	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	static auto position = ZERO;

	if (!g_ctx.globals.start_position.IsZero())
		position = g_ctx.globals.start_position;

	if (position.IsZero())
		return;

	if (!g_ctx.local()->is_alive() || !m_engine()->IsInGame() || !m_engine()->IsConnected())
		return;

	if (!key_binds::get().get_key_bind_state(18))
		return;

	current_peek_position = position;
	current_rotation = current_rotation + rotation_step;
	Vector end_pos = Vector(radius * cos(current_rotation) + current_peek_position.x, radius * sin(current_rotation) + current_peek_position.y, current_peek_position.z);

	//renderer::get().Draw3DFilledCircle(end_pos, radius, Color(0, 153, 255));
	iEffects()->EnergySplash(end_pos, Vector(0, 0, 0), true);

	if (current_rotation > pi * 2)
		current_rotation = 0.0f;
}

int GetIndicatorsInt(weapon_t* weapon) {
	int Num = 0;

	//This is bad, VERY BAD
	if ((cfg::g_cfg.ragebot.double_tap && cfg::g_cfg.ragebot.double_tap_key.key > KEY_NONE && cfg::g_cfg.ragebot.double_tap_key.key < KEY_MAX && exploit::get().double_tap_key)) Num + 1;
	if ((cfg::g_cfg.antiaim.hide_shots && cfg::g_cfg.antiaim.hide_shots_key.key > KEY_NONE && cfg::g_cfg.antiaim.hide_shots_key.key < KEY_MAX && exploit::get().hide_shots_key)) Num + 1;
	if ((cfg::g_cfg.ragebot.resolver && cfg::g_cfg.ragebot.roll_correction_bind.key > KEY_NONE && cfg::g_cfg.ragebot.roll_correction_bind.key < KEY_MAX && key_binds::get().get_key_bind_state(23))) Num + 1;
	if (key_binds::get().get_key_bind_state(3)) Num + 1;
	if (key_binds::get().get_key_bind_state(22)) Num + 1;
	if (g_ctx.globals.current_weapon != -1 && key_binds::get().get_key_bind_state(4) && !weapon->is_non_aim()) Num + 1;
	if (key_binds::get().get_key_bind_state(20)) Num + 1;
	if (cfg::g_cfg.antiaim.freestand && key_binds::get().get_key_bind_state(24)) Num + 1;
	if (key_binds::get().get_key_bind_state(18)) Num + 1;

	return Num;
}

void add_indicator(std::string ind, bool on, Color col) {
	if (!on)
		return;

	Vector2D screen_size = renderer::get().m_screen_size;
	ImVec2 text_size = renderer::get().screen_indicators->CalcTextSizeA(20.f, FLT_MAX, 0.0f, ind.c_str());

	renderer::get().text(renderer::get().screen_indicators, 10, (((screen_size.y / 2) + 15) + text_size.y * otheresp::get().indicators_on) - otheresp::get().total_indicators_on / 2, col, false, false, true, false, ind.c_str());
	
	otheresp::get().indicators_on++;
}

void otheresp::CrosshairIndicators() {
	if (!cfg::g_cfg.menu.keybind_list)
		return;

	if (!m_engine()->IsInGame())
		return;

	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	total_indicators_on = GetIndicatorsInt(weapon);

	add_indicator(XorStr("DT"), (cfg::g_cfg.ragebot.double_tap && cfg::g_cfg.ragebot.double_tap_key.key > KEY_NONE && cfg::g_cfg.ragebot.double_tap_key.key < KEY_MAX&& exploit::get().double_tap_key), (g_ctx.globals.startcharge ? Color(200, 0, 0) : Color(240, 240, 240)));
	add_indicator(XorStr("HS"), (cfg::g_cfg.antiaim.hide_shots && cfg::g_cfg.antiaim.hide_shots_key.key > KEY_NONE && cfg::g_cfg.antiaim.hide_shots_key.key < KEY_MAX&& exploit::get().hide_shots_key), Color(240, 240, 240));
	add_indicator(XorStr("DA"), (cfg::g_cfg.ragebot.dormant_aimbot), Color(61, 151, 219));
	add_indicator(XorStr("RESOLVER"), (cfg::g_cfg.ragebot.resolver && cfg::g_cfg.ragebot.roll_correction_bind.key > KEY_NONE && cfg::g_cfg.ragebot.roll_correction_bind.key < KEY_MAX&& key_binds::get().get_key_bind_state(23)), Color(191, 155, 107));
	add_indicator(XorStr("SAFEPOINT"), (key_binds::get().get_key_bind_state(3)), Color(0, 255, 179));
	add_indicator(XorStr("BAIM"), (key_binds::get().get_key_bind_state(22)), Color(224, 114, 219));
	add_indicator(XorStr("DMG"), (g_ctx.globals.current_weapon != -1 && key_binds::get().get_key_bind_state(4) && !weapon->is_non_aim()), Color(240, 240, 240));
	add_indicator(XorStr("FD"), (key_binds::get().get_key_bind_state(20)), Color(200, 0, 0));
	add_indicator(XorStr("FREESTAND"), (cfg::g_cfg.antiaim.freestand && key_binds::get().get_key_bind_state(24)), Color(114, 110, 255));
	add_indicator(XorStr("AUTO PEEK"), (key_binds::get().get_key_bind_state(18)), Color(123, 186, 35));

	indicators_on = 0;
	total_indicators_on = 0;
}