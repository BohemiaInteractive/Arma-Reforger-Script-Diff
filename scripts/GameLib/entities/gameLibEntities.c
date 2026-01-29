//Generic entities from GameLib (script side of c++ classes)

class WB_UIMenuItem: Managed
{
	string m_name;
	int m_id;
	bool m_enabled;

	void WB_UIMenuItem(string name, int id = 0, bool enabled = true)
	{
		m_name = name;
		m_id = id;
		m_enabled = enabled;
	}
}

// Flags to specify when entity/component event _WB_AfterWorldUpdate needs to be called
enum EEntityFrameUpdateSpecs
{
	CALL_ALWAYS = 1,								// called always (every frame)
	CALL_WHEN_ENTITY_VISIBLE = 2,		// called every frame but only when entity is visible
	CALL_WHEN_ENTITY_SELECTED = 4		// called every frame but only when entity is selected
}

class GenericEntityClass: EntityPrefabData
{
	void GenericEntityClass(BaseContainer prefab) {}
}

/*!
Indices of SkyMaterial properties
usage: g_Game.GetWorldEntity().GetSkyMaterial().SetParamByIndex(ESkyMaterialParams.MIE_DENSITY, 10);
*/
enum ESkyMaterialParams
{
	SKY_INTENSITY_LV, // float; -12 to 20; Intensity of sky in LV or EV if IntensityAbsoluteLV is false
	GROUND_INTENSITY_LV, // float; -12 to 20; Intensity of ground in LV or EV of IntensityAbsoluteLV is false
	GROUND_COLOR, // color - float[3]/float[4], Ground color (lower hemisphere of atmosphere
	IRRADIANCE_INTENSITY, // float; 0.0001 to 1; Intensity of atmosphere's direct irradiance. Lower = ambient/indirect lighting is more prevalent and direct lighting is weaker. Higher = direct lighting is stronger.
	INSCATTER_START, // float; 0 to 1; Start distance of inscatter (fog) from 0=near to 1=far plane.
	DISTANCE_FOG_START, // float; 0 to 1
	DISTANCE_FOG_END, // float; 0 to 1
	GROUND_DENSITY,	// float; 0 to 1
	RAYLEIGH, // float; 0.01 to 1; Amount of Rayleigh scattering.
	RAYLEIGH_SCALE_HEIGHT, // float; 0 to 36000; Height scale of Rayleigh scattering (meters), lower = Rayleigh is 'compressed' to this value. bigger = Rayleigh is 'stretched' to this value.
	MIE_DENSITY, // float; 0 to 10; Density of Mie scattering (multiplicator).");
	MIE_SCALE_HEIGHT, // float; 680 to 36000; Height scale of Mie scattering (meters), lower = Mie is 'compressed' to this value. bigger = Mie is 'stretched' to this value.");
	MIE_ANGSTROM_ALPHA, // float; 0 to 1; Luminosity/Intensity factor of Mie scattering.");
	MIE_ANGSTROM_BETA, // float; 0 to 1; Amount of Mie scattering. (bigger = foggy, lower = clear)");
	MIE_PHASE_FUNCTION_G, // float; 0 to 1; Defines the shape (anisotropy) of the Mie scattering. 1.0 = concentrated around the sun, 0.0 = constant through atmosphere.");
	BACK_SCATTERING, // float; 0 to 1; Amount of mie scattering lit by sun, at the opposite side of sun");
	BACK_SCATTERING_PHASE, // float; 0 to 1; Phase of the back scattering function, similar to MiePhaseFunctionG");
	OVERRIDE_VOLFOG, // bool, Fog colors will be overriden by atmosphere");
	OVERRIDE_VOLFOG_BRIGHTNESS, // float; -20 to 20; Fog intensity");
	SOLAR_IRRADIANCE_TINT, // color - float[3]/float[4]
	RAYLEIGH_SCATTERING_TINT, // color - float[3]/float[4]
	MIE_SCATTERING_TINT // color - float[3]/float[4]
}

class WorldEntityClass : GenericWorldEntityClass
{
}

class WorldEntity : GenericWorldEntity
{
}

enum CharacterMovement
{
	MOVEMENTTYPE_IDLE,
	MOVEMENTTYPE_WALK,
	MOVEMENTTYPE_RUN,
	MOVEMENTTYPE_SPRINT
}

enum CharacterStance
{
	STANCE_ERECT,
	STANCE_CROUCH,
	STANCE_PRONE,
	STANCE_ERECT_RAISED,
	STANCE_CROUCH_RAISED,
	STANCE_PRONE_RAISED
}

