// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\Features\cheats\visuals\GrenadePrediction.h"
#include "..\..\Features\cheats\misc\fakelag.h"
#include "..\..\Features\cheats\lagcompensation\LocalAnimations.h"

using OverrideView_t = void(__stdcall*)(CViewSetup*);

void thirdperson(bool fakeducking);

void __stdcall hooks::hkOverrideview(CViewSetup* viewsetup)
{
	static auto original_fn = clientmode_hook->get_func_address <OverrideView_t> (18);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	if (!viewsetup)
		return original_fn(viewsetup);

	if (g_ctx.local())
	{
		static auto fakeducking = false;
		static float maxduck = 0;
		static bool in_transition = false;

		if (!fakeducking && g_ctx.globals.fakeducking)
			fakeducking = true;
		else if (fakeducking && !g_ctx.globals.fakeducking && (!g_ctx.local()->get_animation_state()->m_fDuckAmount || g_ctx.local()->get_animation_state()->m_fDuckAmount == 1.0f)) //-V550
			fakeducking = false;

		if (!g_ctx.local()->is_alive()) //-V807
			fakeducking = false;

		auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

		if (weapon)
		{
			if (!g_ctx.local()->m_bIsScoped() && true)
				viewsetup->fov += cfg::g_cfg.visuals.fov;
			else if (cfg::g_cfg.visuals.removals[REMOVALS_ZOOM] && true)
			{
				if (weapon->m_zoomLevel() == 1)
					viewsetup->fov = 90.0f + (float)cfg::g_cfg.visuals.fov;
				else
					viewsetup->fov += (float)cfg::g_cfg.visuals.fov;
			}
		}
		else if (true)
			viewsetup->fov += cfg::g_cfg.visuals.fov;

		if (weapon)
		{
			auto viewmodel = (entity_t*)m_entitylist()->GetClientEntityFromHandle(g_ctx.local()->m_hViewModel());

			if (viewmodel)
			{
				auto eyeAng = viewsetup->angles;
				eyeAng.z -= (float)cfg::g_cfg.visuals.viewmodel_roll;

				viewmodel->set_abs_angles(eyeAng);
			}

			if (weapon->is_grenade() && cfg::g_cfg.visuals.grenade_prediction && true)
				GrenadePrediction::get().View(viewsetup, weapon);
		}

		if (true && (cfg::g_cfg.misc.thirdperson_toggle.key > KEY_NONE && cfg::g_cfg.misc.thirdperson_toggle.key < KEY_MAX || cfg::g_cfg.misc.thirdperson_when_spectating))
			thirdperson(fakeducking);
		else
		{
			g_ctx.globals.in_thirdperson = false;
			m_input()->m_fCameraInThirdPerson = false;
		}

		if (fakeducking)
		{
			in_transition = true;
			maxduck += m_globals()->m_frametime * 150;

			if (maxduck > m_gamemovement()->GetPlayerViewOffset(false).z + 0.064f)
				maxduck = m_gamemovement()->GetPlayerViewOffset(false).z + 0.064f;
		}
		else
		{
			if (maxduck > g_ctx.local()->m_vecViewOffset().z)
				maxduck -= m_globals()->m_frametime * 150;

			if (maxduck < g_ctx.local()->m_vecViewOffset().z)
			{
				maxduck = g_ctx.local()->m_vecViewOffset().z;
				in_transition = false;
			}
		}

		if (in_transition)
			viewsetup->origin = g_ctx.local()->GetAbsOrigin() + Vector(0.0f, 0.0f, maxduck);

		original_fn(viewsetup);
	}
	else
		return original_fn(viewsetup);
}

void Thirdperson_Init(bool fakeducking, float progress) 
{
	/* our current fraction. */
	static float current_fraction = 0.0f;
	auto distance = ((float)cfg::g_cfg.misc.thirdperson_distance) * progress;
	Vector angles, inverse_angles;

	// get camera angles.
	m_engine()->GetViewAngles(angles);
	m_engine()->GetViewAngles(inverse_angles);

	// cam_idealdist convar.
	inverse_angles.z = distance;

	// set camera direction.
	Vector forward, right, up;
	math::angle_vectors(inverse_angles, &forward, &right, &up);

	// various fixes to camera when fakeducking.
	auto eye_pos = fakeducking ? g_ctx.local()->GetAbsOrigin() + m_gamemovement()->GetPlayerViewOffset(false) : g_ctx.local()->GetAbsOrigin() + g_ctx.local()->m_vecViewOffset();
	auto offset = eye_pos + forward * -distance + right + up;
	
	// setup trace filter and trace.
	CTraceFilterWorldOnly filter;
	trace_t tr;
	
	// tracing to camera angles.
	m_trace()->TraceRay(Ray_t(eye_pos, offset, Vector(-16.0f, -16.0f, -16.0f), Vector(16.0f, 16.0f, 16.0f)), 131083, &filter, &tr);
	
	// interpolate camera speed if something behind our camera.
	if (current_fraction > tr.fraction)
		current_fraction = tr.fraction;

	else if (current_fraction > 0.9999f)
		current_fraction = 1.0f;

	// adapt distance to travel time.
	current_fraction = math::interpolate(current_fraction, tr.fraction, m_globals()->m_frametime * 10.0f);
	angles.z = distance * current_fraction;

	// override camera angles.
	m_input()->m_vecCameraOffset = angles;
}

void thirdperson(bool fakeducking)
{
	/* thirdperson code. */
	{
		static float progress;
		static bool in_transition;
		static auto in_thirdperson = false;

		if (!in_thirdperson && g_ctx.globals.in_thirdperson)
		{
			in_thirdperson = true;
		}
		else if (in_thirdperson && !g_ctx.globals.in_thirdperson)
			in_thirdperson = false;

		if (g_ctx.local()->is_alive() && in_thirdperson)
		{
			in_transition = false;

			if (!m_input()->m_fCameraInThirdPerson)
			{
				m_input()->m_fCameraInThirdPerson = true;
			}
		}
		else
		{
			if (!cfg::g_cfg.misc.remove_third_person_animation)
			{
				progress -= m_globals()->m_frametime * 8.f + (progress / 100);
				progress = std::clamp(progress, 0.f, 1.f);
			}
			else
			{
				progress = 0.f;
			}

			if (!progress)
				m_input()->m_fCameraInThirdPerson = false;
			else
			{
				in_transition = true;
				m_input()->m_fCameraInThirdPerson = true;
			}
		}

		if (m_input()->m_fCameraInThirdPerson && !in_transition)
		{
			if (!cfg::g_cfg.misc.remove_third_person_animation)
			{
				progress += m_globals()->m_frametime * 8.f + (progress / 100);
				progress = std::clamp(progress, 0.f, 1.f);
			}
			else
			{
				progress = 1.f;
			}
		}
		Thirdperson_Init(fakeducking, progress);
	}
	/* thirdperson on death code. */
	{
		static auto require_reset = false;

		if (g_ctx.local()->is_alive())
		{
			require_reset = false;
			return;
		}

		if (cfg::g_cfg.misc.thirdperson_when_spectating)
		{
			if (require_reset)
				g_ctx.local()->m_iObserverMode() = OBS_MODE_CHASE;

			if (g_ctx.local()->m_iObserverMode() == OBS_MODE_IN_EYE)
				require_reset = true;
		}
	}
}