#pragma once

#include "../helpers/singleton.hpp"

class IMatRenderContext;
struct DrawModelState_t;
struct ModelRenderInfo_t;
class matrix3x4_t;
class IMaterial;
class Color;

class Chams
    : public Singleton<Chams>
{
    friend class Singleton<Chams>;

    Chams();
    ~Chams();

public:
	void OnDrawModelExecute(
        IMatRenderContext* ctx,
        const DrawModelState_t &state,
        const ModelRenderInfo_t &pInfo,
        matrix3x4_t *pCustomBoneToWorld
		);

private:
    void OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, const Color& rgba, bool modulate = true, bool gold = false, bool crystal = false, bool crystal2 = false, bool silver = false);

    IMaterial* materialRegular = nullptr;
    IMaterial* materialRegularIgnoreZ = nullptr;
    IMaterial* materialFlatIgnoreZ = nullptr;
    IMaterial* materialFlat = nullptr;
	IMaterial* materialGold = nullptr;
	IMaterial* materialGlass = nullptr;
	IMaterial* materialGloss = nullptr;
	IMaterial* materialCrystal = nullptr;
	IMaterial* materialCrystalBlue = nullptr;
	IMaterial* materialSilver = nullptr;
};