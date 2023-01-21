#include "AutoStrafer.h"
#include "../Prediction/EnginePrediction.h"

#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)

void airstrafe::create_move(CUserCmd* m_pcmd) //-V2008
{
    if (g_ctx.local()->get_move_type() == MOVETYPE_LADDER) //-V807
        return;

    if (g_ctx.local()->m_fFlags() & FL_ONGROUND || engineprediction::get().backup_data.flags & FL_ONGROUND)
        return;

    static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));
    auto side_speed = cl_sidespeed->GetFloat();

    if (cfg::g_cfg.misc.airstrafe == 1)
    {
        Vector engine_angles;
        m_engine()->GetViewAngles(engine_angles);

        auto velocity = g_ctx.local()->m_vecVelocity();

        m_pcmd->m_forwardmove = min(5850.0f / velocity.Length2D(), side_speed);
        m_pcmd->m_sidemove = m_pcmd->m_command_number % 2 ? side_speed : -side_speed;

        auto yaw_velocity = math::calculate_angle(Vector(0.0f, 0.0f, 0.0f), velocity).y;
        auto ideal_rotation = math::clamp(RAD2DEG(atan2(15.0f, velocity.Length2D())), 0.0f, 45.0f);

        auto yaw_rotation = fabs(yaw_velocity - engine_angles.y) + (m_pcmd->m_command_number % 2 ? ideal_rotation : -ideal_rotation);
        auto ideal_yaw_rotation = yaw_rotation < 5.0f ? yaw_velocity : engine_angles.y;

        util::RotateMovement(m_pcmd, ideal_yaw_rotation);
    }
    else if (cfg::g_cfg.misc.airstrafe == 2)
    {
		
		static auto old_yaw = 0.0f;

		auto get_velocity_degree = [](float velocity)
		{
			auto tmp = RAD2DEG(atan(30.0f / velocity));

			if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
				return 90.0f;

			else if (tmp < 0.0f)
				return 0.0f;
			else
				return tmp;
		};

		if (g_ctx.local()->get_move_type() != MOVETYPE_WALK)
			return;

		auto velocity = g_ctx.local()->m_vecVelocity();
		velocity.z = 0.0f;

		auto forwardmove = m_pcmd->m_forwardmove;
		auto sidemove = m_pcmd->m_sidemove;

		if (velocity.Length2D() < 5.0f && !forwardmove && !sidemove)
			return;

		static auto flip = false;
		flip = !flip;

		auto turn_direction_modifier = flip ? 1.0f : -1.0f;
		auto viewangles = m_pcmd->m_viewangles;

		if (forwardmove || sidemove)
		{
			m_pcmd->m_forwardmove = 0.0f;
			m_pcmd->m_sidemove = 0.0f;

			auto turn_angle = atan2(-sidemove, forwardmove);
			viewangles.y += turn_angle * M_RADPI;
		}
		else if (forwardmove) //-V550
			m_pcmd->m_forwardmove = 0.0f;

		auto strafe_angle = RAD2DEG(atan(15.0f / velocity.Length2D()));

		if (strafe_angle > 90.0f)
			strafe_angle = 90.0f;
		else if (strafe_angle < 0.0f)
			strafe_angle = 0.0f;

		auto temp = Vector(0.0f, viewangles.y - old_yaw, 0.0f);
		temp.y = math::normalize_yaw(temp.y);

		auto yaw_delta = temp.y;
		old_yaw = viewangles.y;

		auto abs_yaw_delta = fabs(yaw_delta);

		if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 15.0f) // whatsapp turbo ativado
		{
			Vector velocity_angles;
			math::vector_angles(velocity, velocity_angles);

			temp = Vector(0.0f, viewangles.y - velocity_angles.y, 0.0f);
			temp.y = math::normalize_yaw(temp.y);

			auto velocityangle_yawdelta = temp.y;
			auto velocity_degree = get_velocity_degree(velocity.Length2D());

			if (velocityangle_yawdelta <= velocity_degree || velocity.Length2D() <= 15.0f)
			{
				if (-velocity_degree <= velocityangle_yawdelta || velocity.Length2D() <= 15.0f)
				{
					viewangles.y += strafe_angle * turn_direction_modifier;
					m_pcmd->m_sidemove = side_speed * turn_direction_modifier;
				}
				else
				{
					viewangles.y = velocity_angles.y - velocity_degree;
					m_pcmd->m_sidemove = side_speed;
				}
			}
			else
			{
				viewangles.y = velocity_angles.y + velocity_degree;
				m_pcmd->m_sidemove = -side_speed;
			}
		}
		else if (yaw_delta > 0.0f)
			m_pcmd->m_sidemove = -side_speed;
		else if (yaw_delta < 0.0f)
			m_pcmd->m_sidemove = side_speed;

		auto move = Vector(m_pcmd->m_forwardmove, m_pcmd->m_sidemove, 0.0f);
		auto speed = move.Length();

		Vector angles_move;
		math::vector_angles(move, angles_move);

		auto normalized_x = fmod(m_pcmd->m_viewangles.x + 180.0f, 360.0f) - 180.0f;
		auto normalized_y = fmod(m_pcmd->m_viewangles.y + 180.0f, 360.0f) - 180.0f;

		auto yaw = DEG2RAD(normalized_y - viewangles.y + angles_move.y);

		if (normalized_x >= 90.0f || normalized_x <= -90.0f || m_pcmd->m_viewangles.x >= 90.0f && m_pcmd->m_viewangles.x <= 200.0f || m_pcmd->m_viewangles.x <= -90.0f && m_pcmd->m_viewangles.x <= 200.0f) //-V648
			m_pcmd->m_forwardmove = -cos(yaw) * speed;
		else
			m_pcmd->m_forwardmove = cos(yaw) * speed;

		m_pcmd->m_sidemove = sin(yaw) * speed;
		

	//static float m_circle_yaw;
	//static float m_previous_yaw;

	//auto get_angle_from_speed = [](float speed) {
	//	auto ideal_angle = RAD2DEG(std::atan2(30.f, speed));
	//	std::clamp(ideal_angle, 0.f, 90.f);
	//	return ideal_angle;
	//};

	//auto get_velocity_step = [](Vector velocity, float speed, float circle_yaw) {
	//	auto velocity_degree = RAD2DEG(std::atan2(velocity.x, velocity.y));
	//	auto step = 1.5f;

	//	Vector start = G::LocalPlayer->GetOrigin(), end = start;

	//	Ray_t ray;
	//	CGameTrace trace;
	//	CTraceFilter filter;

	//	while (true) {
	//		end.x += (std::cos(DEG2RAD(velocity_degree + circle_yaw)) * speed);
	//		end.y += (std::sin(DEG2RAD(velocity_degree + circle_yaw)) * speed);
	//		end *= Interfaces->Globals->frametime;

	//		ray.Init(start, end, Vector(-20.f, -20.f, 0.f), Vector(20.f, 20.f, 32.f));
	//		Interfaces->EngineTrace->TraceRay(ray, CONTENTS_SOLID, &filter, &trace);

	//		if (trace.fraction < 1.f || trace.allsolid || trace.startsolid)
	//			break;

	//		step -= Interfaces->Globals->frametime;

	//		if (step == 0.f)
	//			break;

	//		start = end;
	//		velocity_degree += (velocity_degree + circle_yaw);
	//	}

	//	return step;
	//};

	//auto set_button_state = []() {
	//	G::UserCmd->buttons &= ~(IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK);

	//	if (G::UserCmd->sidemove < 0.f)
	//		G::UserCmd->buttons |= IN_MOVELEFT;
	//	else
	//		G::UserCmd->buttons |= IN_MOVERIGHT;

	//	if (G::UserCmd->forwardmove < 0.f)
	//		G::UserCmd->buttons |= IN_BACK;
	//	else
	//		G::UserCmd->buttons |= IN_FORWARD;
	//};

	//if (G::LocalPlayer->GetMoveType() != MOVETYPE_WALK || G::LocalPlayer->GetFlags() & FL_ONGROUND)
	//	return;

	//auto velocity = G::LocalPlayer->GetVelocity();
	//velocity.z = 0;

	//static bool flip = false;
	//auto turn_direction_modifier = flip ? 1.f : -1.f;
	//flip = !flip;

	//if (G::UserCmd->forwardmove > 0.f)
	//	G::UserCmd->forwardmove = 0.f;

	//auto speed = velocity.Length2D();


	//auto ideal_move_angle = RAD2DEG(std::atan2(15.f, speed));
	//std::clamp(ideal_move_angle, 0.f, 90.f);

	//auto yaw_delta = Math::NormalizeFloat(G::UserCmd->viewangles.y - m_previous_yaw);
	//auto abs_yaw_delta = abs(yaw_delta);
	//m_circle_yaw = m_previous_yaw = G::UserCmd->viewangles.y;

	//if (yaw_delta > 0.f)
	//	G::UserCmd->sidemove = -450.f;

	//else if (yaw_delta < 0.f)
	//	G::UserCmd->sidemove = 450.f;

	//if (abs_yaw_delta <= ideal_move_angle || abs_yaw_delta >= 30.f) {
	//	QAngle velocity_angles;
	//	Math::VectorAngles(velocity, velocity_angles);

	//	auto velocity_angle_yaw_delta = Math::NormalizeFloat(G::UserCmd->viewangles.y - velocity_angles.y);
	//	auto velocity_degree = get_angle_from_speed(speed) * 2/*menu->main.misc.retrack_speed.value()*/;

	//	if (velocity_angle_yaw_delta <= velocity_degree || speed <= 15.f) {
	//		if (-(velocity_degree) <= velocity_angle_yaw_delta || speed <= 15.f) {
	//			G::UserCmd->viewangles.y += (ideal_move_angle * turn_direction_modifier);
	//			G::UserCmd->sidemove = 450.f * turn_direction_modifier;
	//		}

	//		else {
	//			G::UserCmd->viewangles.y = velocity_angles.y - velocity_degree;
	//			G::UserCmd->sidemove = 450.f;
	//		}
	//	}

	//	else {
	//		G::UserCmd->viewangles.y = velocity_angles.y + velocity_degree;
	//		G::UserCmd->sidemove = -450.f;
	//	}
	//}

	//set_button_state();

   }
}