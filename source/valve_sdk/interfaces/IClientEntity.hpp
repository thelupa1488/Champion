#pragma once 

#include "IClientNetworkable.hpp"
#include "IClientRenderable.hpp"
#include "IClientUnknown.hpp"
#include "IClientThinkable.hpp"

struct SpatializationInfo_t;

class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	virtual void Release(void) = 0;
};

#pragma pack(push, 1)
class CCSWeaponInfo { //xSeeker
public:
	int8_t pad0[20];
	int iMaxClip1;                       // 0x0014
	int iMaxClip2;                       // 0x0018
	int iDefaultClip1;                   // 0x001C
	int iDefaultClip2;                   // 0x0020
	char pad_0024[8];                    // 0x0024
	char* szWorldModel;                  // 0x002C
	char* szViewModel;                   // 0x0030
	char* szDroppedMode;                 // 0x0034
	char pad_0038[4];                    // 0x0038
	char* szShotSound;                   // 0x003C
	char pad_0040[56];                   // 0x0040
	char* szEmptySound;                  // 0x0078
	char pad_007C[4];                    // 0x007C
	char* szBulletType;                  // 0x0080
	char pad_0084[4];                    // 0x0084
	char* szHudName;
	char* szWeaponName;
	int8_t pad3[56];
	int32_t iWeaponType;
	int8_t pad4[4];
	int32_t iWeaponPrice;
	int32_t iKillAward;
	int8_t pad5[20];
	uint8_t bFullAuto;
	int8_t pad6[3];
	int32_t iDamage;
	float_t flArmorRatio;
	int32_t iBullets;
	float_t flPenetration;
	int8_t pad7[8];
	float_t flRange;
	float_t flRangeModifier;
	float		flThrowVelocity;			// 0x010C
	int8_t pad8[16];
	bool		bHasSilencer;				// 0x011C
	char		pad_0x0119[3];			// 0x011D
	char* pSilencerModel;				// 0x0120
	int			iCrosshairMinDistance;		// 0x0124
	float		flMaxPlayerSpeed;	        // 0x0128
	float		flMaxPlayerSpeedAlt;		// 0x012C
	char		pad_0x0130[4];		    // 0x0130
	float		flSpread;					// 0x0134
	float		flSpreadAlt;				// 0x0138
	float		flInaccuracyCrouch;			// 0x013C
	float		flInaccuracyCrouchAlt;		// 0x0140
	float		flInaccuracyStand;			// 0x0144
	float		flInaccuracyStandAlt;		// 0x0148
	float		flInaccuracyJumpInitial;	// 0x014C
	float		flInaccuracyJump;			// 0x0150
	float		flInaccuracyJumpAlt;		// 0x0154
	float		flInaccuracyLand;			// 0x0158
	float		flInaccuracyLandAlt;		// 0x015C
	float		flInaccuracyLadder;			// 0x0160
	float		flInaccuracyLadderAlt;		// 0x0164
	float		flInaccuracyFire;			// 0x0168
	float		flInaccuracyFireAlt;		// 0x016C
	float		flInaccuracyMove;			// 0x0170
	float		flInaccuracyMoveAlt;		// 0x0174
	float		flInaccuracyReload;			// 0x0178
	int			iRecoilSeed;				// 0x017C
	float		flRecoilAngle;				// 0x0180
	float		flRecoilAngleAlt;			// 0x0184
	float		flRecoilAngleVariance;		// 0x0188
	float		flRecoilAngleVarianceAlt;	// 0x018C
	float		flRecoilMagnitude;			// 0x0190
	float		flRecoilMagnitudeAlt;		// 0x0194
	float		flRecoilMagnitudeVariance;	// 0x0198
	float		flRecoilMagnitudeVarianceAlt;	// 0x019C
	float		flRecoveryTimeCrouch;		// 0x01A0
	float		flRecoveryTimeStand;		// 0x01A4
	float		flRecoveryTimeCrouchFinal;	// 0x01A8
	float		flRecoveryTimeStandFinal;	// 0x01AC
	int			iRecoveryTransitionStartBullet;	// 0x01B0 
	int			iRecoveryTransitionEndBullet;	// 0x01B4
	bool		bUnzoomAfterShot;			// 0x01B8
	bool		bHideViewModelZoomed;		// 0x01B9
	char		pad_0x01B5[2];			// 0x01BA
	char		iZoomLevels[3];			// 0x01BC
	int			iZoomFOV[2];				// 0x01C0
	float		fZoomTime[3];				// 0x01C4
	char* szWeaponClass;				// 0x01D4
	float		flAddonScale;				// 0x01D8
	char		pad_0x01DC[4];			// 0x01DC
	char* szEjectBrassEffect;			// 0x01E0
	char* szTracerEffect;				// 0x01E4
	int			iTracerFrequency;			// 0x01E8
	int			iTracerFrequencyAlt;		// 0x01EC
	char* szMuzzleFlashEffect_1stPerson;	// 0x01F0
	char		pad_0x01F4[4];				// 0x01F4
	char* szMuzzleFlashEffect_3rdPerson;	// 0x01F8
	char		pad_0x01FC[4];			// 0x01FC
	char* szMuzzleSmokeEffect;		// 0x0200
	float		flHeatPerShot;				// 0x0204
	char* szZoomInSound;				// 0x0208
	char* szZoomOutSound;				// 0x020C
	float		flInaccuracyPitchShift;		// 0x0210
	float		flInaccuracySoundThreshold;	// 0x0214
	float		flBotAudibleRange;			// 0x0218
	char		pad_0x0218[8];			// 0x0220
	char* pWrongTeamMsg;				// 0x0224
	bool		bHasBurstMode;				// 0x0228
	char		pad_0x0225[3];			// 0x0229
	bool		bIsRevolver;				// 0x022C
	bool		bCannotShootUnderwater;		// 0x0230
};
#pragma pack(pop)

class IWeaponSystem
{
	virtual void unused0() = 0;
	virtual void unused1() = 0;
public:
	virtual CCSWeaponInfo* GetWpnData(unsigned ItemDefinitionIndex) = 0;
};