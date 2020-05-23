#include "humanbhop.h"

void HumanBhop::Human_Bhop(CUserCmd* cmd) 
{
	static int hops_restricted = 0;
	static int hops_hit = 0;

	if (!(cmd->buttons & IN_JUMP)
		|| (g_LocalPlayer->m_nMoveType() & MOVETYPE_LADDER))
		return;
	if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
	{
		cmd->buttons &= ~IN_JUMP;
		hops_restricted = 0;
	}
	else if ((rand() % 100 > g_Options.misc_bhop_hit_chance	
		&& hops_restricted < g_Options.misc_bhops_restricted_limit)
		|| (g_Options.misc_max_bhops_hit > 0							
			&& hops_hit > g_Options.misc_max_bhops_hit))				
	{
		cmd->buttons &= ~IN_JUMP;
		hops_restricted++;
		hops_hit = 0;
	}
	else
		hops_hit++;
}