#pragma once
#include "..\..\includes.hpp"
#include "..\lagcompensation\animation_system.h"
#include "aim.h"
//
//class knifebot : public singleton <knifebot>
//{
//	struct table_t {
//		uint8_t swing[2][2][2];
//		uint8_t stab[2][2];		 
//	};
//
//	const table_t m_knife_dmg{ { { { 25, 90 }, { 21, 76 } }, { { 40, 90 }, { 34, 76 } } }, { { 65, 180 }, { 55, 153 } } };
//
//	void scan_targets();
//	void fire(CUserCmd* cmd);
//	bool determinate_hit_type(bool stab_type, const Vector& delta);
//	bool KnifeIsBehind();
//	bool can_knife(Vector angle);
//
//	bool stab = false;
//
//	scanned_target final_target;
//public:
//	void run(CUserCmd* cmd);
//};

class knifebot : public singleton <knifebot>
{
	void scan_targets();
	void fire(CUserCmd* cmd);
	int determinate_hit_type(bool stab_type, const Vector& delta);

	scanned_target final_target;
public:
	void run(CUserCmd* cmd);
};