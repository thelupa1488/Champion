#include <algorithm>

#include "visuals.hpp"
#include "aimbot.hpp"
#include "../hooks/hooks.hpp"
#include "../config/options.hpp"
#include "../render/render.hpp"
#include "grenade_prediction.hpp"
#include "../helpers/math.hpp"
#include "../features/backtrack.hpp"
#include "../helpers/utils.hpp"
#include <iomanip>
bomb_planted_event bomb_planted_listener;
round_end_event round_end_listener;
std::string string_format(const std::string fmt_str, ...) {
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy(&formatted[0], fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::string(formatted.get());
}

float GetArmourHealth(float flDamage, int ArmorValue)
{
	float flCurDamage = flDamage;

	if (flCurDamage == 0.0f || ArmorValue == 0)
		return flCurDamage;

	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	float flNew = flCurDamage * flArmorRatio;
	float flArmor = (flCurDamage - flNew) * flArmorBonus;

	if (flArmor > ArmorValue)
	{
		flArmor = ArmorValue * (1.0f / flArmorBonus);
		flNew = flCurDamage - flArmor;
	}

	return flNew;
}

void Visuals::Player::debug_visuals()
{
	int x_offset = 0;
	if (g_Options.debug_showposes)
	{
		char act[128];
		RECT bbox = ctx.bbox;
		auto poses = ctx.pl->m_flPoseParameter();
		for (int i = 0; i < 24; ++i){
			sprintf(act, "Pose %d %f", i, poses[i]);
			//Render::Get().RenderText(act, bbox.right + 5, bbox.top + i * 12, 15.f, Color(10 * i, 255, 255, 255));
		}
		//Visuals::DrawString(Visuals::ui_font, bbox.right + 5, bbox.top + -12, Color(255, 0, 55, 255), FONT_LEFT, "LBY %f", (Visuals::ESP_ctx.player->m_flLowerBodyYawTarget() + 180.f) / 360.f);
		x_offset += 50;
	}

	if (g_Options.debug_showactivities)
	{
		float h = fabs(ctx.feet_pos.y - ctx.head_pos.y);
		float w = h / 2.0f;

		int offsety = 0;
		for (int i = 0; i < ctx.pl->GetNumAnimOverlays(); i++)
		{
			auto layer = ctx.pl->GetAnimOverlay(i);
			int number = layer->m_nSequence,
				activity = ctx.pl->GetSequenceActivity(number);
			std::setprecision(3);
			Render::Get().RenderText(std::to_string(number).c_str(), ctx.bbox.right + 5, ctx.bbox.top + i * 12, 15.f,Color(255, 255, 0, 255));
			Render::Get().RenderText(std::to_string(activity).c_str(), ctx.bbox.right + 75, ctx.bbox.top + i * 12, 15.f, Color(255, 255, 0, 255));
			Render::Get().RenderText(std::to_string(layer->m_flCycle).c_str(), ctx.bbox.right + 145, ctx.bbox.top + i * 12, 15.f, Color(255, 255, 0, 255));
			Render::Get().RenderText(std::to_string(layer->m_flWeight).c_str(), ctx.bbox.right + 215, ctx.bbox.top + i * 12, 15.f, Color(255, 255, 0, 255));
			Render::Get().RenderText(std::to_string(layer->m_flPlaybackRate).c_str(), ctx.bbox.right + 285, ctx.bbox.top + i * 12, 15.f, Color(255, 255, 0, 255));

			/*if (activity == 979)
			{
				Visuals::DrawString(Visuals::ui_font, Visuals::ESP_ctx.head_pos.x + w + x_offset + 65, Visuals::ESP_ctx.head_pos.y + offsety, Color(255, 255, 0, 255), FONT_LEFT, std::to_string(layer.m_flWeight).c_str());
				Visuals::DrawString(Visuals::ui_font, Visuals::ESP_ctx.head_pos.x + w + x_offset + 65, Visuals::ESP_ctx.head_pos.y + offsety + 12, Color(255, 255, 0, 255), FONT_LEFT, std::to_string(layer.m_flCycle).c_str());
			}*/

			offsety += 12;
		}
		x_offset += 100;
	}
}

void draw_hitboxes(C_BasePlayer* player, matrix3x4_t* matrix)
{
	const auto model = player->GetModel();

	if (!model)
		return;

	const auto hdr = g_MdlInfo->GetStudiomodel(model);

	if (!hdr)
		return;

	const auto set = hdr->GetHitboxSet(player->m_nHitboxSet());

	if (!set)
		return;

	for (auto i = 0; i < set->numhitboxes; i++) {
		const auto hitbox = set->GetHitbox(i);

		if (!hitbox)
			continue;

		if (hitbox->m_flRadius == -1.0f)
		{

		}
		else
		{
			Vector min, max;
			Math::VectorTransform(hitbox->bbmin, matrix[hitbox->bone], min);
			Math::VectorTransform(hitbox->bbmax, matrix[hitbox->bone], max);

			g_DebugOverlay->DrawPill(min, max, hitbox->m_flRadius, 0, 255, 0, 160, g_GlobalVars->interval_per_tick * 2.f);
		}
	}
}

float flPlayerAlpha[65];

bool bombisready = false;

void Visuals::Listener()
{
	g_GameEvents->AddListener(&bomb_planted_listener, "bomb_planted", false);
	g_GameEvents->AddListener(&round_end_listener, "round_end", false);
}

void round_end_event::FireGameEvent(IGameEvent* p_event)
{
	/* if we are connected */
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	/* return if not event */
	if (!p_event)
		return;

	if (strstr(p_event->GetName(), "round_end")) {
		bombisready = false;
	}
}

void bomb_planted_event::FireGameEvent(IGameEvent* p_event)
{
	/* if we are connected */
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	/* return if not event */
	if (!p_event)
		return;

	if (strstr(p_event->GetName(), "bomb_planted")) {
		bombisready = true;
	}
}

void Visuals::RenderSounds() {
	UpdateSounds();

	auto Add3DCircle = [](const Vector& position, Color color, float radius) {
		float precision = 160.0f;

		const float step = DirectX::XM_2PI / precision;

		for (float a = 0.f; a < DirectX::XM_2PI; a += step) {
			Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
			Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

			Vector start2d, end2d;
			if (!Math::WorldToScreen(start, start2d) || !Math::WorldToScreen(end, end2d))
				return;

			Render::Get().RenderLine(start2d.x, start2d.y, end2d.x, end2d.y, color);
		}
	};


	for (auto& [entIndex, sound] : m_Sounds) {
		if (sound.empty())
			continue;

		for (auto& info : sound) {
			if (info.soundTime + g_Options.esp_sounds_time < g_GlobalVars->realtime)
				info.alpha -= g_GlobalVars->frametime;

			if (info.alpha <= 0.0f)
				continue;

			float deltaTime = g_GlobalVars->realtime - info.soundTime;

			auto factor = deltaTime / g_Options.esp_sounds_time;
			if (factor > 1.0f)
				factor = 1.0f;

			float radius = g_Options.esp_sounds_radius * factor;
			Color color = Color(g_Options.color_esp_sounds[0], g_Options.color_esp_sounds[1], g_Options.color_esp_sounds[2],
				info.alpha);

			Add3DCircle(info.soundPos, color, radius);
		}

		while (!sound.empty()) {
			auto& back = sound.back();
			if (back.alpha <= 0.0f)
				sound.pop_back();
			else
				break;
		}
	}
}

void Visuals::UpdateSounds() {

	sounds.RemoveAll();
	g_EngineSound->GetActiveSounds(sounds);
	if (sounds.Count() < 1)
		return;

	Vector eye_pos = g_LocalPlayer->GetEyePos();
	for (int i = 0; i < sounds.Count(); ++i) {
		SndInfo_t& sound = sounds.Element(i);
		if (sound.m_nSoundSource < 1)
			continue;

		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(sound.m_nSoundSource);
		if (!player)
			continue;

		if (player->m_hOwnerEntity().IsValid() && player->IsWeapon()) {
			player = (C_BasePlayer*)player->m_hOwnerEntity().Get();
		}

		if (!player->IsPlayer() || !player->IsAlive())
			continue;

		if ((C_BasePlayer*)g_LocalPlayer == player || (g_Options.esp_sounds_enemy_only && g_LocalPlayer->m_iTeamNum() == player->m_iTeamNum()))
			continue;

		if (player->m_vecOrigin().DistTo(g_LocalPlayer->m_vecOrigin()) > 900)
			continue;

		auto& player_sound = m_Sounds[player->EntIndex()];
		if (player_sound.size() > 0) {
			bool should_break = false;
			for (const auto& snd : player_sound) {
				if (snd.guid == sound.m_nGuid) {
					should_break = true;
					break;
				}
			}

			if (should_break)
				continue;
		}

		SoundInfo_t& snd = player_sound.emplace_back();
		snd.guid = sound.m_nGuid;
		snd.soundPos = *sound.m_pOrigin;
		snd.soundTime = g_GlobalVars->realtime;
		snd.alpha = 1.0f;
	}
}

RECT GetBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}

Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() {
	DeleteCriticalSection(&cs);
}

extern bool CanSee[65];

//--------------------------------------------------------------------------------
void Visuals::Render() {
}
//--------------------------------------------------------------------------------
bool Visuals::Player::Begin(C_BasePlayer* pl)
{
	if (!pl->IsAlive())
		return false;

	if (pl->IsDormant() && flPlayerAlpha[pl->EntIndex()] > 0) 
		flPlayerAlpha[pl->EntIndex()] -= 0.33f;
	else if (!pl->IsDormant())
		flPlayerAlpha[pl->EntIndex()] = 255;
	int alpha = (int)flPlayerAlpha[pl->EntIndex()];
	std::clamp(alpha, 0, 255);

	ctx.dormantfade = alpha;
	ctx.pl = pl;
	ctx.is_enemy = g_LocalPlayer->m_iTeamNum() != pl->m_iTeamNum();
	ctx.is_visible = CanSee[pl->EntIndex()];

	if(!ctx.is_visible && g_Options.esp_visible_only)
		return false;

	if (!ctx.is_enemy && g_Options.esp_enemy_only)
		return false;

	ctx.clr = ctx.is_visible ? Color(g_Options.color_esp_visible) : Color(g_Options.color_esp_occluded);
	if (ctx.dormantfade < 254)
		ctx.clr = Color(80, 80, 80, ctx.dormantfade);

	ctx.bbox = GetBBox(pl);

	if (!ctx.bbox.bottom || !ctx.bbox.right || !ctx.bbox.left || !ctx.bbox.top)
		return false;

	std::swap(ctx.bbox.top, ctx.bbox.bottom);
	return true;
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderBox() {
	float edge_size;

	float
		length_horizontal = (ctx.bbox.right - ctx.bbox.left) * 0.2f,
		length_vertical = (ctx.bbox.bottom - ctx.bbox.top) * 0.2f;

	switch (g_Options.esp_player_box) {
	case 1:
		Render::Get().RenderBox(ctx.bbox, ctx.clr);
		Render::Get().RenderBox(ctx.bbox.left - 1.f, ctx.bbox.top - 1.f, ctx.bbox.right + 1.f, ctx.bbox.bottom + 1.f, Color(0, 0, 0, ctx.clr.a()));
		Render::Get().RenderBox(ctx.bbox.left + 1.f, ctx.bbox.top + 1.f, ctx.bbox.right - 1.f, ctx.bbox.bottom - 1.f, Color(0, 0, 0, ctx.clr.a()));
		break;

	case 2:
		Render::Get().RenderBoxFilled(ctx.bbox.left - 1.f, ctx.bbox.top - 1.f, ctx.bbox.left + 1.f + length_horizontal, ctx.bbox.top + 2.f, Color(0, 0, 0, ctx.clr.a()));
		Render::Get().RenderBoxFilled(ctx.bbox.right - 1.f - length_horizontal, ctx.bbox.top - 1.f, ctx.bbox.right + 1.f, ctx.bbox.top + 2.f, Color(0, 0, 0, ctx.clr.a()));
		Render::Get().RenderBoxFilled(ctx.bbox.left - 1.f, ctx.bbox.bottom - 2.f, ctx.bbox.left + 1.f + length_horizontal, ctx.bbox.bottom + 1.f, Color(0, 0, 0, ctx.clr.a()));
		Render::Get().RenderBoxFilled(ctx.bbox.right - 1.f - length_horizontal, ctx.bbox.bottom - 2.f, ctx.bbox.right + 1.f, ctx.bbox.bottom + 1.f, Color(0, 0, 0, ctx.clr.a()));

		Render::Get().RenderBoxFilled(ctx.bbox.left - 1.f, ctx.bbox.top + 2.f, ctx.bbox.left + 2.f, ctx.bbox.top + 1.f + length_vertical, Color(0,0,0, ctx.clr.a()));
		Render::Get().RenderBoxFilled(ctx.bbox.right - 2.f, ctx.bbox.top + 2.f, ctx.bbox.right + 1.f, ctx.bbox.top + 1.f + length_vertical, Color(0, 0, 0, ctx.clr.a()));
		Render::Get().RenderBoxFilled(ctx.bbox.left - 1.f, ctx.bbox.bottom - 1.f - length_vertical, ctx.bbox.left + 2.f, ctx.bbox.bottom - 2.f, Color(0, 0, 0, ctx.clr.a()));
		Render::Get().RenderBoxFilled(ctx.bbox.right - 2.f, ctx.bbox.bottom - 1.f - length_vertical, ctx.bbox.right + 1.f, ctx.bbox.bottom - 2.f, Color(0, 0, 0, ctx.clr.a()));


		Render::Get().RenderLine(float(ctx.bbox.left), float(ctx.bbox.top), ctx.bbox.left + length_horizontal - 1.f, float(ctx.bbox.top), ctx.clr);
		Render::Get().RenderLine(ctx.bbox.right - length_horizontal, float(ctx.bbox.top), ctx.bbox.right - 1.f, float(ctx.bbox.top), ctx.clr);
		Render::Get().RenderLine(float(ctx.bbox.left), ctx.bbox.bottom - 1.f, ctx.bbox.left + length_horizontal - 1.f, ctx.bbox.bottom - 1.f, ctx.clr);
		Render::Get().RenderLine(ctx.bbox.right - length_horizontal, ctx.bbox.bottom - 1.f, ctx.bbox.right - 1.f, ctx.bbox.bottom - 1.f, ctx.clr);

		Render::Get().RenderLine(float(ctx.bbox.left), float(ctx.bbox.top), float(ctx.bbox.left), ctx.bbox.top + length_vertical - 1.f, ctx.clr);
		Render::Get().RenderLine(ctx.bbox.right - 1.f, float(ctx.bbox.top), ctx.bbox.right - 1.f, ctx.bbox.top + length_vertical - 1.f, ctx.clr);
		Render::Get().RenderLine(float(ctx.bbox.left), ctx.bbox.bottom - length_vertical, float(ctx.bbox.left), ctx.bbox.bottom - 1.f, ctx.clr);
		Render::Get().RenderLine(ctx.bbox.right - 1.f, ctx.bbox.bottom - length_vertical, ctx.bbox.right - 1.f, ctx.bbox.bottom - 1.f, ctx.clr);
		break;

	case 3:
		Render::Get().RenderBox(ctx.bbox, ctx.clr, 1.0f, 7.0f);
		//Render::Get().RenderBox(ctx.bbox, ctx.clr, 7.0f);
		break;
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderSkeleton()
{
	studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel(ctx.pl->GetModel());

	if (pStudioModel) {
		static matrix3x4_t pBoneToWorldOut[128];
		//if (ctx.pl->HandleBoneSetup(pBoneToWorldOut, 256, g_GlobalVars->curtime)) {
		if (ctx.pl->SetupBones(pBoneToWorldOut, 128, 256, g_GlobalVars->curtime)) {
			for (int i = 0; i < pStudioModel->numbones; i++) {
				mstudiobone_t* pBone = pStudioModel->GetBone(i);
				if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
					continue;

				Vector vBonePos1;
				if (!Math::WorldToScreen(Vector(pBoneToWorldOut[i][0][3], pBoneToWorldOut[i][1][3], pBoneToWorldOut[i][2][3]), vBonePos1))
					continue;

				Vector vBonePos2;
				if (!Math::WorldToScreen(Vector(pBoneToWorldOut[pBone->parent][0][3], pBoneToWorldOut[pBone->parent][1][3], pBoneToWorldOut[pBone->parent][2][3]), vBonePos2))
					continue;

				Render::Get().RenderLine(vBonePos1.x, vBonePos1.y, vBonePos2.x, vBonePos2.y, ctx.pl->IsDormant() ? ctx.clr : Color(g_Options.color_esp_skeleton));
			}
		}
	}
}

#define FLAG_MACRO std::pair<std::string, Color> // :joy:
#define FLAG(string, color) vecFlags.push_back(FLAG_MACRO(string, color)) 

void Visuals::Player::DrawFlags() /*Not the best way to do this, tbh*/
{
	if (!g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		return;
	std::vector<FLAG_MACRO> vecFlags;

	int alpha = ctx.clr.a(); // so now, this is epic
	if (g_Options.esp_flags_armor)
		if (ctx.pl->m_ArmorValue() > 0)
			FLAG(ctx.pl->GetArmorName(), Color(255, 128, 0, alpha));

	if (g_Options.esp_flags_scoped)
		if (ctx.pl->m_bIsScoped())
			FLAG("ZOOM", Color(52, 165, 207, alpha));

	if (g_Options.esp_flags_money)
		if (ctx.pl->m_iAccount())
			FLAG(std::string("$").append(std::to_string(ctx.pl->m_iAccount())), Color(0, 200, 0, alpha));

	if (g_Options.esp_flags_defusing)
		if (ctx.pl->m_bIsDefusing())
			FLAG("DEFUSING", Color(200, 0, 0, alpha));

	if (g_Options.esp_flags_c4)
		if (ctx.pl->HasC4())
			FLAG("C4", Color(200, 0, 0, alpha));

	if (g_Options.esp_flags_hostage)
		if (ctx.pl->m_hCarriedHostage() || ctx.pl->m_bIsGrabbingHostage())
			FLAG("HOSTAGE", ctx.pl->m_bIsGrabbingHostage() ?  Color(200, 0, 0, alpha) : Color(52, 165, 207, alpha));

	if (g_Options.esp_flags_kit)
		if (ctx.pl->m_bHasDefuser())
			FLAG("KIT", Color(52, 165, 207, alpha));

	int offset = 0; //smh, have to think about a better way just because of this lmao
	for (auto Text : vecFlags)
	{
		Render::Get().RenderText(Text.first, ctx.bbox.right + 4 + ((ctx.pl->m_ArmorValue() > 0 && g_Options.esp_player_armour) ? 5 : 0), ctx.bbox.top - 2 + offset,14.f, Text.second);
		offset += 12;
	}
}


void Visuals::Player::RenderCustomSkeleton(matrix3x4_t* mat)
{
	studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel(ctx.pl->GetModel());

	if (pStudioModel) {
		if (mat) {
			for (int i = 0; i < pStudioModel->numbones; i++) {
				mstudiobone_t* pBone = pStudioModel->GetBone(i);
				if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
					continue;

				Vector vBonePos1;
				if (!Math::WorldToScreen(Vector(mat[i][0][3], mat[i][1][3], mat[i][2][3]), vBonePos1))
					continue;

				Vector vBonePos2;
				if (!Math::WorldToScreen(Vector(mat[pBone->parent][0][3], mat[pBone->parent][1][3], mat[pBone->parent][2][3]), vBonePos2))
					continue;

				Render::Get().RenderLine(vBonePos1.x, vBonePos1.y, vBonePos2.x, vBonePos2.y, ctx.pl->IsDormant() ? ctx.clr : Color(g_Options.color_esp_skeleton));
			}
		}
	}
}

//--------------------------------------------------------------------------------

void GetVisFov() {

}

void Visuals::DrawFOV() {
	auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
	if (!pWeapon)
		return;

	auto settings = g_Options.aimbot[pWeapon->m_Item().m_iItemDefinitionIndex()];

	if (settings.enabled) {

		float fov = static_cast<float>(g_LocalPlayer->GetFOV());

		int w, h;
		g_EngineClient->GetScreenSize(w, h);

		

		int x = w / 2;
		int y = h / 2;
		int dy = h / 97;
		int dx = w / 97;

		QAngle punchAngle = g_LocalPlayer->m_aimPunchAngle() * (g_CVar->FindVar("weapon_recoil_scale")->GetFloat() / 2);
		x -= (dx * (punchAngle.yaw));
		y += (dy * (punchAngle.pitch));


		Vector2D screenSize = Vector2D(w, h);
		Vector2D center = screenSize * 0.5f;

		float ratio = screenSize.x / screenSize.y;
		float screenFov = atanf((ratio) * (0.75f) * tan(DEG2RAD(fov * 0.5f)));
		QAngle shit;
		g_EngineClient->GetViewAngles(&shit);
		float radiusFOV;
		if (settings.fov_type == 1)
		{
			Vector src3D, dst3D, forward;
			trace_t tr;
			Ray_t ray;
			CTraceFilter filter;

			QAngle angles = shit;
			Math::AngleVectors(angles, forward);
			filter.pSkip = g_LocalPlayer;
			src3D = g_LocalPlayer->GetEyePos();
			dst3D = src3D + (forward * 8192);

			ray.Init(src3D, dst3D);
			g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

			QAngle leftViewAngles = QAngle(angles.pitch, angles.yaw - 90.f, 0.f);
			Math::Normalize3(leftViewAngles);
			Math::AngleVectors(leftViewAngles, forward);
			forward *= settings.fov * 5.f;

			Vector maxAimAt = tr.endpos + forward;

			Vector max2D;
			if (g_DebugOverlay->ScreenPosition(maxAimAt, max2D))
				return;
			radiusFOV = fabsf(x - max2D.x);


		}
		else
			radiusFOV = tanf(DEG2RAD(g_Aimbot.GetFov())) / tanf(screenFov) * center.x;



		Render::Get().RenderCircle(x, y, radiusFOV, 128, Color(0, 0, 0, 170));

		if (settings.silent) {
			float silentRadiusFOV = tanf(DEG2RAD(settings.silent_fov)) / tanf(screenFov) * center.x;
			Render::Get().RenderCircle(x, y, silentRadiusFOV, 128, Color(255, 25, 10, 170));
		}

		/*QAngle shit;
		float radius;
		g_EngineClient->GetViewAngles(&shit);

		if (settings.fov_type == 1)
		{
			Vector src3D, dst3D, forward;
			trace_t tr;
			Ray_t ray;
			CTraceFilter filter;

			QAngle angles = shit;
			Math::AngleVectors(angles, forward);
			filter.pSkip = g_LocalPlayer;
			src3D = g_LocalPlayer->GetEyePos();
			dst3D = src3D + (forward * 8192);

			ray.Init(src3D, dst3D);
			g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

			QAngle leftViewAngles = QAngle(angles.pitch, angles.yaw - 90.f, 0.f);
			Math::Normalize3(leftViewAngles);
			Math::AngleVectors(leftViewAngles, forward);
			forward *= settings.fov * 5.f;

			Vector maxAimAt = tr.endpos + forward;

			Vector max2D;
			if (g_DebugOverlay->ScreenPosition(maxAimAt, max2D))
				return;

			radius = fabsf(w / 2 - max2D.x);
		}
		else
			radius = ((settings.fov / g_Options.misc_fov) * w) / 2;


		Render::Get().RenderCircle(x, y, radius, 128, Color(255, 25, 10, 170));*/
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderName()
{
	player_info_t info = ctx.pl->GetPlayerInfo();

	auto sz = g_pDefaultFont->CalcTextSizeA(15.f, FLT_MAX, 0.0f, info.szName);

	Render::Get().RenderText(info.szName, ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) * 0.5f - sz.x * 0.5f, ctx.bbox.top - sz.y - 1, 14.f, ctx.pl->IsDormant() ? ctx.clr : Color::White, false, false);
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderHealth()
{
	auto hp = ctx.pl->m_iHealth();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 7;

	int height = (box_h * hp) / 100;

	int green = int(hp * 2.55f);
	int red = 255 - green;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;

	Render::Get().RenderBox(x, y - 1, x + w, y + h + 2, ctx.pl->IsDormant() ? ctx.clr : Color::Black, 1.f, true);
	Render::Get().RenderBox(x + 1, y, x + w - 1, y + height + 1, ctx.pl->IsDormant() ? Color(50, 50, 50, ctx.dormantfade) : Color(red, green, 0, 255), 1.f, true);
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderArmour()
{
	auto armour = ctx.pl->m_ArmorValue();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 4;

	int height = (((box_h * armour) / 100));

	int x = ctx.bbox.right + off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;
	if (armour > 1) {
		Render::Get().RenderBox(x, y, x + w, y + h, ctx.pl->IsDormant() ? ctx.clr : Color::Black, 1.0f, true);
		Render::Get().RenderBox(x + 1, y + 1, x + w - 1, y + height - 2, ctx.pl->IsDormant() ? ctx.clr : Color(0, 50, 255, 255), 1.f, true);
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderWeaponName()
{

	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};
	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon || !weapon->GetCSWeaponData()) return;

	auto text = clean_item_name(weapon->GetClientClass()->m_pNetworkName);

	//size_t len = wcstombs(text, text1, wcslen(text1));
	//if (len > 0u)
	//	text[len] = '\0';

	std::string ammo = string_format("[%i/%i]", weapon->m_iPrimaryReserveAmmoCount(), weapon->m_iClip1());

	auto sz = g_pDefaultFont->CalcTextSizeA(15.f, FLT_MAX, 0.0f, text);

	auto sz2 = g_pDefaultFont->CalcTextSizeA(15.f, FLT_MAX, 0.0f, ammo.c_str());

	Render::Get().RenderText(text, ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) * 0.5f - sz.x * 0.5f, ctx.bbox.bottom + 1.f, 14.f, ctx.pl->IsDormant() ? ctx.clr : Color::White, false, false);
	if(g_Options.show_ammo)
		Render::Get().RenderText(ammo, ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) * 0.5f - sz2.x * 0.5f, ctx.bbox.bottom + 1.f + sz2.y, 14.f, ctx.pl->IsDormant() ? ctx.clr : Color::White, false, false);
}
//--------------------------------------------------------------------------------
void Visuals::RenderWeapon(C_BaseCombatWeapon* ent)
{
	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	// We don't want to Render weapons that are being held
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	auto name = clean_item_name(ent->GetClientClass()->m_pNetworkName);

	auto sz = g_pDefaultFont->CalcTextSizeA(15.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;


	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color::White, false, false);
}

//--------------------------------------------------------------------------------
void Visuals::RenderDefuseKit(C_BaseEntity* ent)
{

	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;


	auto name = "Defuse Kit";
	auto sz = g_pDefaultFont->CalcTextSizeA(15.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;
	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color::White, false, false);
}
//--------------------------------------------------------------------------------
RECT GetViewport()
{
	RECT viewport = { 0, 0, 0, 0 };
	int w, h;
	g_EngineClient->GetScreenSize(w, h);
	viewport.right = w; viewport.bottom = h;

	return viewport;
}

void DrawZeusRange()
{
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_Options.zeusrange)
		return;

	auto local_player = g_LocalPlayer;
	if (!local_player) return;
	if (local_player->m_iHealth() <= 0) return;

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon) return;

	if (weapon->m_iItemDefinitionIndex() != WEAPON_TASER) return;

	float step = M_PI * 2.0 / 1023;
	float rad = 130.f;
	Vector origin = local_player->GetEyePos();

	static double rainbow;

	Vector screenPos;
	static Vector prevScreenPos;

	for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
	{
		Vector pos(rad * cos(rotation) + origin.x, rad * sin(rotation) + origin.y, origin.z);

		CTraceFilter filter;
		CGameTrace trace;
		filter.pSkip = local_player;
		Ray_t t;
		t.Init(origin, pos);

		g_EngineTrace->TraceRay(t, MASK_SHOT_BRUSHONLY, &filter, &trace);

		if (g_DebugOverlay->ScreenPosition(trace.endpos, screenPos))
			continue;

		if (!prevScreenPos.IsZero() && !screenPos.IsZero() && screenPos.DistTo(Vector(-107374176, -107374176, -107374176)) > 3.f && prevScreenPos.DistTo(Vector(-107374176, -107374176, -107374176)) > 3.f) {
			rainbow += 0.00001;
			if (rainbow > 1.f)
				rainbow = 0;
			Color color = Color::FromHSB(rainbow, 1.f, 1.f);
			VGSHelper::Get().DrawLine(prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color);
		}
		prevScreenPos = screenPos;
	}
}

void DrawKnifeRange()
{
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_Options.zeusrange)
		return;

	auto local_player = g_LocalPlayer;
	if (!local_player) return;
	if (local_player->m_iHealth() <= 0) return;

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon) return;


	/*	WEAPON_BAYONET = 500,
	WEAPON_KNIFE_CSS = 503,
	WEAPON_KNIFE_FLIP = 505,
	WEAPON_KNIFE_GUT = 506,
	WEAPON_KNIFE_KARAMBIT = 507,
	WEAPON_KNIFE_M9_BAYONET = 508,
	WEAPON_KNIFE_TACTICAL = 509,
	WEAPON_KNIFE_FALCHION = 512,
	WEAPON_KNIFE_SURVIVAL_BOWIE = 514,
	WEAPON_KNIFE_BUTTERFLY = 515,
	WEAPON_KNIFE_PUSH = 516,
	WEAPON_KNIFE_CORD = 517,
	WEAPON_KNIFE_CANIS = 518,
	WEAPON_KNIFE_URSUS = 519,
	WEAPON_KNIFE_GYPSY_JACKKNIFE = 520,
	WEAPON_KNIFE_OUTDOOR = 521,
	WEAPON_KNIFE_STILETTO = 522,
	WEAPON_KNIFE_WIDOWMAKER = 523,
	WEAPON_KNIFE_SKELETON = 525,
	*/
	if (weapon->m_iItemDefinitionIndex() != WEAPON_KNIFE , WEAPON_BAYONET , WEAPON_KNIFE_CSS, WEAPON_KNIFE_FLIP, WEAPON_KNIFE_GUT, WEAPON_KNIFE_KARAMBIT, WEAPON_KNIFE_M9_BAYONET, WEAPON_KNIFE_TACTICAL, WEAPON_KNIFE_FALCHION, WEAPON_KNIFE_SURVIVAL_BOWIE, WEAPON_KNIFE_BUTTERFLY, WEAPON_KNIFE_PUSH, WEAPON_KNIFE_CORD, WEAPON_KNIFE_CANIS, WEAPON_KNIFE_URSUS, WEAPON_KNIFE_GYPSY_JACKKNIFE, WEAPON_KNIFE_OUTDOOR, WEAPON_KNIFE_STILETTO, WEAPON_KNIFE_WIDOWMAKER, WEAPON_KNIFE_SKELETON) return;

	float step = M_PI * 2.0 / 1023;
	float rad = 130.f;
	Vector origin = local_player->GetEyePos();

	static double rainbow;

	Vector screenPos;
	static Vector prevScreenPos;

	for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
	{
		Vector pos(rad * cos(rotation) + origin.x, rad * sin(rotation) + origin.y, origin.z);

		CTraceFilter filter;
		CGameTrace trace;
		filter.pSkip = local_player;
		Ray_t t;
		t.Init(origin, pos);

		g_EngineTrace->TraceRay(t, MASK_SHOT_BRUSHONLY, &filter, &trace);

		if (g_DebugOverlay->ScreenPosition(trace.endpos, screenPos))
			continue;

		if (!prevScreenPos.IsZero() && !screenPos.IsZero() && screenPos.DistTo(Vector(-107374176, -107374176, -107374176)) > 3.f && prevScreenPos.DistTo(Vector(-107374176, -107374176, -107374176)) > 3.f) {
			rainbow += 0.00001;
			if (rainbow > 1.f)
				rainbow = 0;
			Color color = Color::FromHSB(rainbow, 1.f, 1.f);
			VGSHelper::Get().DrawLine(prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color);
		}
		prevScreenPos = screenPos;
	}
}

void VGSHelper::DrawLine(float x1, float y1, float x2, float y2, Color color, float size)
{
	g_VGuiSurface->DrawSetColor(color);

	if (size == 1.f)
		g_VGuiSurface->DrawLine(x1, y1, x2, y2);
	else
		g_VGuiSurface->DrawFilledRect(x1 - (size / 2.f), y1 - (size / 2.f), x2 + (size / 2.f), y2 + (size / 2.f));
}

void Visuals::RenderPlantedC4(C_BaseEntity* entity)
{
	RECT scrn = GetViewport();

	char text[64];
	std::string text2;
	float TimeRemaining = entity->m_flC4Blow() - g_GlobalVars->curtime;
	float defusing_countdown = entity->m_flDefuseCountDown() - (g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick);
	int tw2, th2;

	std::string buf3;
	if (entity->m_nBombSite() == 0)
		buf3 = "A";
	else if (entity->m_nBombSite() == 1)
		buf3 = "B";
	else
		buf3 = "C 0_o?";

	Color time;

	if (TimeRemaining > 0) {
		if (TimeRemaining <= 10 && TimeRemaining >= 5)
			time = Color(215, 200,0);
		else if (TimeRemaining <= 5)
			time = Color(200, 0, 0);
		else
			time = Color(73, 200, 0);
	}

	C_BasePlayer* mainent = g_LocalPlayer->IsAlive() ? g_LocalPlayer : g_LocalPlayer->m_hObserverTarget().Get();

	if (!mainent)
		return;

	float a = 450.7f;
	float b = 75.68f;
	float c = 789.2f;
	float d = ((mainent->GetEyePos().DistTo(entity->m_vecOrigin()) - b) / c);
	float flDamage = a * expf(-d * d);
	
	int damage = (int)std::fmaxf(ceilf(GetArmourHealth(flDamage, mainent->m_ArmorValue())), 0.f);

	Color damagecol;

	if (damage > 0) {
		if (damage > 75)
			damagecol = Color(200,0, 0);
		else if (damage > 40)
			damagecol = Color(215, 200, 0);
		else
			damagecol = Color(73, 200, 0);
	}

	if (TimeRemaining > 0.f) 
	{
		sprintf(text, "Bomb on %s - %.1f sec", buf3.c_str(), TimeRemaining);
		if (damage < mainent->m_iHealth())
			text2 = string_format("-%iHP", damage);
		else
			text2 = string_format("%s","FATAL");

		if (text && bombisready && damage && defusing_countdown != 0)
		{	
			Render::Get().RenderText(text, scrn.left + 6, scrn.bottom / 2, 24.f, time, false, true, g_pSecondFont);
			if (entity->m_hBombDefuser() > 0)
			    Render::Get().RenderText(text2, scrn.left + 6, scrn.bottom / 2 + 40, 24.f, damagecol, false, true, g_pSecondFont);
			else 
				Render::Get().RenderText(text2, scrn.left + 6, scrn.bottom / 2 + 20, 24.f, damagecol, false, true, g_pSecondFont);
		}	
	}
}

void Visuals::RenderDefusingC4(C_BaseEntity* entity)
{
	char text[64];
	Color time;
	RECT scrn = GetViewport();

	if (entity->m_hBombDefuser() > 0)
	{
		float countdown = entity->m_flDefuseCountDown() - (g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick);

		if (countdown > 0)
		{
			if (countdown <= 4 && countdown >= 2)
				time = Color(215, 200, 0);
			else if (countdown <= 2)
				time = Color(215, 0, 0);
			else
				time = Color(73, 200, 0);
		}

		if (countdown > 0.01f)
		{
			sprintf(text, "Defusing - %.1f sec", countdown);

			if (text && bombisready)
			{
				Render::Get().RenderText(text, scrn.left + 6, scrn.bottom / 2 + 20, 24.f, time, false, true, g_pSecondFont);
			}
		}
	}
}
//--------------------------------------------------------------------------------
void Visuals::RenderItemEsp(C_BaseEntity* ent)
{
	if (ent->m_hOwnerEntity().IsValid())
		return;
	std::string itemstr = "Undefined";
	const model_t * itemModel = ent->GetModel();
	if (!itemModel)
		return;
	studiohdr_t * hdr = g_MdlInfo->GetStudiomodel(itemModel);
	if (!hdr)
		return;
	itemstr = hdr->szName;
	if (ent->GetClientClass()->m_ClassID == ClassId_CBumpMine)
		itemstr = "";
	else if (itemstr.find("case_pistol") != std::string::npos)
		itemstr = "Pistol Case";
	else if (itemstr.find("case_light_weapon") != std::string::npos)
		itemstr = "Light Case";
	else if (itemstr.find("case_heavy_weapon") != std::string::npos)
		itemstr = "Heavy Case";
	else if (itemstr.find("case_explosive") != std::string::npos)
		itemstr = "Explosive Case";
	else if (itemstr.find("case_tools") != std::string::npos)
		itemstr = "Tools Case";
	else if (itemstr.find("random") != std::string::npos)
		itemstr = "Airdrop";
	else if (itemstr.find("dz_armor_helmet") != std::string::npos)
		itemstr = "Full Armor";
	else if (itemstr.find("dz_helmet") != std::string::npos)
		itemstr = "Helmet";
	else if (itemstr.find("dz_armor") != std::string::npos)
		itemstr = "Armor";
	else if (itemstr.find("upgrade_tablet") != std::string::npos)
		itemstr = "Tablet Upgrade";
	else if (itemstr.find("briefcase") != std::string::npos)
		itemstr = "Briefcase";
	else if (itemstr.find("parachutepack") != std::string::npos)
		itemstr = "Parachute";
	else if (itemstr.find("dufflebag") != std::string::npos)
		itemstr = "Cash Dufflebag";
	else if (itemstr.find("ammobox") != std::string::npos)
		itemstr = "Ammobox";
	else if (itemstr.find("dronegun") != std::string::npos)
		itemstr = "Turrel";
	else if (itemstr.find("exojump") != std::string::npos)
		itemstr = "Exojump";
	else if (itemstr.find("healthshot") != std::string::npos)
		itemstr = "Healthshot";
	else {
		/*May be you will search some missing items..*/
		/*static std::vector<std::string> unk_loot;
		if (std::find(unk_loot.begin(), unk_loot.end(), itemstr) == unk_loot.end()) {
			Utils::ConsolePrint(itemstr.c_str());
			unk_loot.push_back(itemstr);
		}*/
		return;
	}
	
	if (ent->m_vecOrigin().DistTo(g_LocalPlayer->GetEyePos()) > g_Options.esp_distance_dz)
		return;
	auto bbox = GetBBox(ent);
	if (bbox.right == 0 || bbox.bottom == 0)
		return;
	auto sz = g_pDefaultFont->CalcTextSizeA(15.f, FLT_MAX, 0.0f, itemstr.c_str());
	int w = bbox.right - bbox.left;

	Render::Get().RenderText(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color::White, false, false);
}
//--------------------------------------------------------------------------------
void Visuals::ThirdPerson() {
	if (!g_LocalPlayer)
		return;

	static size_t lastTime = 0;

	if (GetAsyncKeyState(g_Options.misc_thirdperson_bind))
	{
		if (GetTickCount64() > lastTime) {
			g_Options.misc_thirdperson = !g_Options.misc_thirdperson;

			lastTime = GetTickCount64() + 650;
		}
	}

	if (g_Options.misc_thirdperson && g_LocalPlayer->IsAlive())
		g_Input->m_fCameraInThirdPerson = true;
	else
		g_Input->m_fCameraInThirdPerson = false;

	auto GetCorrectDistance = [](float ideal_distance) -> float
	{
		/* vector for the inverse angles */
		QAngle inverseAngles;
		g_EngineClient->GetViewAngles(&inverseAngles);

		/* inverse angles by 180 */
		inverseAngles.pitch *= -1.f, inverseAngles.yaw += 180.f;

		/* vector for direction */
		Vector direction;
		Math::AngleVectors(inverseAngles, direction);

		/* ray, trace & filters */
		Ray_t ray;
		trace_t trace;
		CTraceFilter filter;

		/* dont trace local player */
		filter.pSkip = g_LocalPlayer;

		/* create ray */
		ray.Init(g_LocalPlayer->GetEyePos(), g_LocalPlayer->GetEyePos() + (direction * ideal_distance));

		/* trace ray */
		g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);

		/* return the ideal distance */
		return (ideal_distance * trace.fraction) - 10.f;
	};

	QAngle angles;
	g_EngineClient->GetViewAngles(&angles);
	angles.roll = GetCorrectDistance(g_Options.misc_thirdperson_dist); // 150 is better distance
	g_Input->m_vecCameraOffset = Vector(angles.pitch,angles.yaw,angles.roll);
}

void DrawAngles(C_BasePlayer* pl) {
	auto drawAngleLine = [&](const Vector& origin, const Vector& w2sOrigin, const float& angle, const char* text, Color clr) {
		Vector forward;
		Math::AngleVectors(QAngle(0.0f, angle, 0.0f), forward);
		float AngleLinesLength = 30.0f;

		Vector w2sReal;
		if (Math::WorldToScreen(origin + forward * AngleLinesLength, w2sReal)) {
			Render::Get().RenderLine(w2sOrigin.x, w2sOrigin.y, w2sReal.x, w2sReal.y, Color::White, 1.0f);
			Render::Get().RenderText(text, w2sReal.x, w2sReal.y - 5.0f, 14.f, clr, true, true);
		
		}
	};

	if (!pl || !pl->GetPlayerAnimState())
		return;

	if (!pl->IsAlive())
		return;

	if (g_Options.esp_angle_lines) {
		Vector w2sOrigin;
		extern float anglereal;
		extern float anglefake;
		if (Math::WorldToScreen(pl->GetAbsOrigin(), w2sOrigin)) {
			static float view;
			if(Globals::bSendPacket)
			view = Globals::cmd->viewangles.yaw;
			drawAngleLine(pl->GetAbsOrigin(), w2sOrigin, pl->GetPlayerAnimState()->m_flGoalFeetYaw, "fake", Color(0.937f, 0.713f, 0.094f, 1.0f));
			drawAngleLine(pl->GetAbsOrigin(), w2sOrigin, pl->m_flLowerBodyYawTarget(), "lby", Color(0.0f, 0.0f, 1.0f, 1.0f));
			drawAngleLine(pl->GetAbsOrigin(), w2sOrigin, anglereal, "real", Color(0.0f, 1.0f, 0.0f, 1.0f));
			drawAngleLine(pl->GetAbsOrigin(), w2sOrigin, view, "view", Color(1.0f, 0.0f, 0.0f, 1.0f));
		}
	}
}
#include "damageindicator.h"
#include "Notification.h"


void Visuals::AddToDrawList() {

	for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) {
		auto entity = C_BaseEntity::GetEntityByIndex(i);

		if (!entity)
			continue;

		if (g_Options.misc_radar)
			entity->m_bSpotted() = true;

		if (!g_LocalPlayer)
			continue;

		if (entity == g_LocalPlayer && !g_Input->m_fCameraInThirdPerson)
			continue;

		if(g_LocalPlayer->m_hObserverTarget().Get() == entity)
			continue;

		if (entity->IsPlantedC4() && g_Options.esp_planted_c4)
			RenderPlantedC4(entity);

		if (entity->IsPlantedC4() && g_Options.esp_defusing_c4)
			RenderDefusingC4(entity);

		if (!g_Options.esp_enabled)
			continue;



	//	if(entity->IsPlayer())
			//if (g_Options.esp_visible_only && !g_LocalPlayer->CanSeePlayer((C_BasePlayer*)entity, HITBOX_CHEST, tu))
			//	continue;
		if (g_Options.esp_defuse_kit && entity->IsDefuseKit())
			RenderDefuseKit(entity);
		if (g_Options.esp_dropped_weapons && entity->IsWeapon())
			RenderWeapon(static_cast<C_BaseCombatWeapon*>(entity));
		if (entity->IsLoot() && g_Options.esp_items)
			RenderItemEsp(entity);
		if (entity->IsPlayer()) {
			auto player = Player();
			if (player.Begin((C_BasePlayer*)entity)) {
				
				if (g_Options.esp_player_box)		player.RenderBox();
				if (g_Options.esp_player_skeleton)	player.RenderSkeleton();
				if (g_Options.esp_player_weapons)   player.RenderWeaponName();
				if (g_Options.esp_player_names)     player.RenderName();
				if (g_Options.esp_player_health)    player.RenderHealth();
				if (g_Options.esp_player_armour)    player.RenderArmour();
				//DrawAngles(player.ctx.pl);
				player.DrawFlags();

				if (g_Options.esp_player_skeleton_bt && g_Backtrack.data.count(player.ctx.pl->EntIndex()) > 0) {
					auto& data = g_Backtrack.data.at(player.ctx.pl->EntIndex());
					if (data.size() > 0) {

						auto& back = data.back();
						player.RenderCustomSkeleton(back.boneMatrix);
					
					}
					
				}
			}

				//player.debug_visuals();

			
		}
		
	}

	if(g_Options.esp_angle_lines)
		DrawAngles(g_LocalPlayer);

	if (g_Options.other_drawfov)
		DrawFOV();

	if (g_Options.zeusrange)
		DrawZeusRange();

	if (g_Options.kniferange)
		DrawKnifeRange();

	//if (g_Options.ping)
		//PingIndicator();

	if(g_Options.esp_damageindicator)
		damage_indicators.paint();

	if(g_Options.esp_sounds)
		RenderSounds();
#ifndef _DEBUG
	if (g_Options.esp_grenade_prediction)
		GrenadePrediction::Get().Paint(nullptr);
#endif
}
