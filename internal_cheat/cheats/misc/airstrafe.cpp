// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "airstrafe.h"
#include "..\misc\prediction_system.h"

#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)

void airstrafe::create_move(CUserCmd* m_pcmd)
{
	if (g_ctx.local()->get_move_type() == MOVETYPE_LADDER)
		return;

	if (g_ctx.local()->m_fFlags() & FL_ONGROUND || engineprediction::get().backup_data.flags & FL_ONGROUND)
		return;

	static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));
	auto side_speed = cl_sidespeed->GetFloat();

	if (g_cfg.misc.airstrafe == 1)
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
	else if (g_cfg.misc.airstrafe == 2)
	{
		auto get_move_angle = [](float speed) -> const float
		{
			auto move_angle = (asin(15.f / speed)) * (180.f / M_PI);

			if (!isfinite(move_angle) || move_angle > 90.f)
				move_angle = 90.f;
			else if (move_angle < 0.f)
				move_angle = 0.f;

			return move_angle;
		};

		static auto switchKey = 1.f;
		static auto circleYaw = 0.f;
		static auto oldYaw = 0.f;

		auto velocity = g_ctx.local()->m_vecVelocity();
		velocity.z = 0.f;

		auto speed = velocity.Length2D();
		auto idealStrafe = get_move_angle(speed);
		auto sidemove = m_pcmd->m_sidemove;
		auto forwardmove = m_pcmd->m_forwardmove;

		switchKey *= -1.f;

		if (g_ctx.local()->get_move_type() == 2 && !(g_ctx.local()->m_fFlags() & FL_ONGROUND && !(m_pcmd->m_buttons & IN_JUMP)))
		{
			if (g_cfg.misc.wsadkeys && (sidemove || forwardmove))
			{
				m_pcmd->m_forwardmove = 0.f;
				m_pcmd->m_sidemove = 0.f;

				auto angle = atan2(-sidemove, forwardmove);
				g_ctx.globals.wish_angle.y += angle * M_RADPI;
			}
			else if (m_pcmd->m_forwardmove > 0.f)
				m_pcmd->m_forwardmove = 0.f;

			auto yawDelta = math::normalize_yaw(g_ctx.globals.wish_angle.y - oldYaw);
			auto absYawDelta = abs(yawDelta);

			circleYaw = oldYaw = g_ctx.globals.wish_angle.y;

			if (yawDelta > 0.f)
				m_pcmd->m_sidemove = -450.f;
			else if (yawDelta < 0.f)
				m_pcmd->m_sidemove = 450.f;

			if (absYawDelta <= idealStrafe || absYawDelta >= 30.f)
			{
				Vector velocity_angles;
				math::vector_angles(velocity, velocity_angles);

				auto velocityDelta = math::normalize_yaw(g_ctx.globals.wish_angle.y - velocity_angles.y);
				auto retrack = idealStrafe * (15.f - g_cfg.misc.retrack_speed);

				if (velocityDelta <= retrack || speed <= 15.f)
				{
					if (-retrack <= velocityDelta || speed <= 15.f)
					{
						g_ctx.globals.wish_angle.y += idealStrafe * switchKey;
						m_pcmd->m_sidemove = switchKey * 450.f;
					}
					else
					{
						g_ctx.globals.wish_angle.y = velocity_angles.y - retrack;
						m_pcmd->m_sidemove = 450.f;
					}
				}
				else
				{
					g_ctx.globals.wish_angle.y = velocity_angles.y + retrack;
					m_pcmd->m_sidemove = -450.f;
				}
			}
		}
	}
}