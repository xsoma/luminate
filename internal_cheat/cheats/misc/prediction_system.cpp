// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "prediction_system.h"

void engineprediction::store_netvars()
{
	auto data = &netvars_data[m_clientstate()->iCommandAck % MULTIPLAYER_BACKUP];

	data->tickbase = g_ctx.local()->m_nTickBase();
	data->m_aimPunchAngle = g_ctx.local()->m_aimPunchAngle();
	data->m_aimPunchAngleVel = g_ctx.local()->m_aimPunchAngleVel();
	data->m_viewPunchAngle = g_ctx.local()->m_viewPunchAngle();
	data->m_vecViewOffset = g_ctx.local()->m_vecViewOffset();
	data->m_duckAmount = g_ctx.local()->m_flDuckAmount();
	data->m_duckSpeed = g_ctx.local()->m_flDuckSpeed();
}

void engineprediction::restore_netvars()
{
	auto data = &netvars_data[(m_clientstate()->iCommandAck - 1) % MULTIPLAYER_BACKUP];

	if (data->tickbase != g_ctx.local()->m_nTickBase())
		return;

	auto aim_punch_angle_delta = g_ctx.local()->m_aimPunchAngle() - data->m_aimPunchAngle;
	auto aim_punch_angle_vel_delta = g_ctx.local()->m_aimPunchAngleVel() - data->m_aimPunchAngleVel;
	auto view_punch_angle_delta = g_ctx.local()->m_viewPunchAngle() - data->m_viewPunchAngle;
	auto view_offset_delta = g_ctx.local()->m_vecViewOffset() - data->m_vecViewOffset;
	auto duck_amount = g_ctx.local()->m_flDuckAmount() - data->m_duckAmount;

	if (fabs(aim_punch_angle_delta.x) < 0.03125f && fabs(aim_punch_angle_delta.y) < 0.03125f && fabs(aim_punch_angle_delta.z) < 0.03125f)
		g_ctx.local()->m_aimPunchAngle() = data->m_aimPunchAngle;

	if (fabs(aim_punch_angle_vel_delta.x) < 0.03125f && fabs(aim_punch_angle_vel_delta.y) < 0.03125f && fabs(aim_punch_angle_vel_delta.z) < 0.03125f)
		g_ctx.local()->m_aimPunchAngleVel() = data->m_aimPunchAngleVel;

	if (fabs(view_punch_angle_delta.x) < 0.03125f && fabs(view_punch_angle_delta.y) < 0.03125f && fabs(view_punch_angle_delta.z) < 0.03125f)
		g_ctx.local()->m_viewPunchAngle() = data->m_viewPunchAngle;

	if (fabs(view_offset_delta.x) < 0.03125f && fabs(view_offset_delta.y) < 0.03125f && fabs(view_offset_delta.z) < 0.03125f)
		g_ctx.local()->m_vecViewOffset() = data->m_vecViewOffset;

	if (fabs(duck_amount) < 0.03125f)
	{
		g_ctx.local()->m_flDuckAmount() = data->m_duckAmount;
		g_ctx.local()->m_flDuckSpeed() = data->m_duckSpeed;
	}
}

void engineprediction::setup()
{
	if (prediction_data.prediction_stage != SETUP)
		return;

	if (m_clientstate()->iDeltaTick > 0)
		m_prediction()->Update(m_clientstate()->iDeltaTick, true, m_clientstate()->nLastCommandAck, m_clientstate()->nLastOutgoingCommand + m_clientstate()->iChokedCommands);

	backup_data.flags = g_ctx.local()->m_fFlags();
	backup_data.velocity = g_ctx.local()->m_vecVelocity();

	prediction_data.old_curtime = m_globals()->m_curtime;
	prediction_data.old_frametime = m_globals()->m_frametime;

	m_globals()->m_curtime = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase);
	m_globals()->m_frametime = m_prediction()->EnginePaused ? 0.0f : m_globals()->m_intervalpertick;

	if (!prediction_data.prediction_random_seed)
		prediction_data.prediction_random_seed = *reinterpret_cast <int**> (util::FindSignature(crypt_str("client.dll"), crypt_str("A3 ? ? ? ? 66 0F 6E 86")) + 0x1);

	if (!prediction_data.prediction_player)
		prediction_data.prediction_player = *reinterpret_cast <int**> (util::FindSignature(crypt_str("client.dll"), crypt_str("89 35 ? ? ? ? F3 0F 10 48")) + 0x2);

	static bool unlocked_fakelag = false;

	if (!m_gamerules()->m_bIsValveDS() && !unlocked_fakelag) {
		auto cl_move_clamp = util::FindSignature(crypt_str("engine.dll"), crypt_str("B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC")) + 1;
		unsigned long protect = 0;

		VirtualProtect((void*)cl_move_clamp, 4, PAGE_EXECUTE_READWRITE, &protect);
		*(std::uint32_t*)cl_move_clamp = 62;
		VirtualProtect((void*)cl_move_clamp, 4, protect, &protect);
		unlocked_fakelag = true;
	}

	prediction_data.prediction_stage = PREDICT;
}

void engineprediction::predict(CUserCmd* m_pcmd)
{
	if (prediction_data.prediction_stage != PREDICT)
		return;

	*reinterpret_cast <uint32_t*> (reinterpret_cast <uint32_t> (g_ctx.local()) + 0x3334) = reinterpret_cast <uint32_t> (m_pcmd);

	*prediction_data.prediction_random_seed = MD5_PseudoRandom(m_pcmd->m_command_number) & INT_MAX;
	*prediction_data.prediction_player = reinterpret_cast <int> (g_ctx.local());

	m_gamemovement()->StartTrackPredictionErrors(g_ctx.local());
	m_movehelper()->set_host(g_ctx.local());

	static auto m_nImpulse = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_nImpulse"));
	//static auto m_nButtons = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_nButtons"));
	//static auto m_afButtonLast = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_afButtonLast"));
	//static auto m_afButtonPressed = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_afButtonPressed"));
	//static auto m_afButtonReleased = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_afButtonReleased"));

	if (m_pcmd->m_impulse)
		*reinterpret_cast<uint32_t*>(uint32_t(g_ctx.local()) + m_nImpulse) = m_pcmd->m_impulse;

	//int* buttons = reinterpret_cast<int*>(uint32_t(g_ctx.local()) + m_nButtons);
	//const int buttonsChanged = m_pcmd->m_buttons ^ *buttons;

	//*reinterpret_cast<int*>(uint32_t(g_ctx.local()) + m_afButtonLast) = *buttons;
	//*reinterpret_cast<int*>(uint32_t(g_ctx.local()) + m_nButtons) = m_pcmd->m_buttons;
	//*reinterpret_cast<int*>(uint32_t(g_ctx.local()) + m_afButtonPressed) = buttonsChanged & m_pcmd->m_buttons;
	//*reinterpret_cast<int*>(uint32_t(g_ctx.local()) + m_afButtonReleased) = buttonsChanged & ~m_pcmd->m_buttons;

	m_prediction()->CheckMovingGround(g_ctx.local(), m_globals()->m_frametime);

	if (g_ctx.local()->physics_run_think(0))
		g_ctx.local()->pre_think();

	auto thinktick = reinterpret_cast <int*> (reinterpret_cast <uint32_t> (g_ctx.local()) + 0x0FC);

	if (*thinktick > 0 && *thinktick <= g_ctx.local()->m_nTickBase())
	{
		*thinktick = -1;

		static auto fn = reinterpret_cast <void(__thiscall*)(int)> (util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B")));
		fn(0);

		g_ctx.local()->think();
	}

	CMoveData move_data;
	memset(&move_data, 0, sizeof(CMoveData));

	m_prediction()->SetupMove(g_ctx.local(), m_pcmd, m_movehelper(), &move_data);
	m_gamemovement()->ProcessMovement(g_ctx.local(), &move_data);
	m_prediction()->FinishMove(g_ctx.local(), m_pcmd, &move_data);
	m_movehelper()->process_impacts();

	call_virtual <void(__thiscall*)(void*)>(m_modelcache(), 33)(m_modelcache());
	call_virtual <void(__thiscall*)(void*)>(g_ctx.local(), 339)(g_ctx.local());

	if (g_ctx.local()->m_fFlags() & FL_ONGROUND)
		*reinterpret_cast <uintptr_t*> (uintptr_t(g_ctx.local()) + 0x3014) = 0;

	if (*reinterpret_cast <uint32_t*> (reinterpret_cast <uint32_t> (g_ctx.local()) + 0x28BC) == -1)
		call_virtual <void(__thiscall*)(void*, int)>(g_ctx.local(), 218)(g_ctx.local(), 0);

	call_virtual <void(__thiscall*)(void*)>(g_ctx.local(), 219)(g_ctx.local());

	static auto post_think_v_physics = reinterpret_cast <bool(__thiscall*)(entity_t*)> (util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 81 EC ?? ?? ?? ?? 53 8B D9")));
	post_think_v_physics(g_ctx.local());

	static auto simulate_player_simulated_entities = reinterpret_cast <bool(__thiscall*)(entity_t*)> (util::FindSignature(crypt_str("client.dll"), crypt_str("56 8B F1 57 8B BE ?? ?? ?? ?? 83 EF 01")));
	simulate_player_simulated_entities(g_ctx.local());

	call_virtual <int(__thiscall*)(void*)>(m_modelcache(), 34)(m_modelcache());

	auto viewmodel = g_ctx.local()->m_hViewModel().Get();

	if (viewmodel)
	{
		viewmodel_data.weapon = viewmodel->m_hWeapon().Get();

		viewmodel_data.viewmodel_index = viewmodel->m_nViewModelIndex();
		viewmodel_data.sequence = viewmodel->m_nSequence();
		viewmodel_data.animation_parity = viewmodel->m_nAnimationParity();

		viewmodel_data.cycle = viewmodel->m_flCycle();
		viewmodel_data.animation_time = viewmodel->m_flAnimTime();
	}

	prediction_data.prediction_stage = FINISH;
}

void engineprediction::finish()
{
	if (prediction_data.prediction_stage != FINISH)
		return;

	m_gamemovement()->FinishTrackPredictionErrors(g_ctx.local());
	m_movehelper()->set_host(nullptr);

	*reinterpret_cast <uint32_t*> (reinterpret_cast <uint32_t> (g_ctx.local()) + 0x3334) = 0;
	*prediction_data.prediction_random_seed = -1;
	*prediction_data.prediction_player = 0;

	m_globals()->m_curtime = prediction_data.old_curtime;
	m_globals()->m_frametime = prediction_data.old_frametime;
}