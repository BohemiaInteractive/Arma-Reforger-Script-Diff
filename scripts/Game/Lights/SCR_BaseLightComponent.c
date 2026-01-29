[EntityEditorProps(category: "GameScripted/BaseLightComponent", description: "Basic light component to spawn a light")]
class SCR_BaseLightComponentClass : ScriptComponentClass
{
}

class SCR_BaseLightComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] lightData
	//! \param[in] position
	//! \param[in] direction
	//! \param[in] emissivity
	//! \return
	//LightEntity CreateLight(LightType lightType, LightFlags flags, float effectRadius, vector color, float emissivity, vector position, vector direction = vector.Zero, LightLensFlareType flareType, float lensFlareScale, vector lensFlareOffset)
	LightEntity CreateLight(SCR_BaseLightData lightData, vector position, vector direction, float emissivity)
	{	
		if (!lightData)
			return null;
		
		LightEntity light = LightEntity.CreateLight(lightData.GetLightType(), lightData.GetLightFlag(), lightData.GetEffectRadius(), Color.FromVector(lightData.GetLightColor()), emissivity, position, direction);
		if (!light)
			return null;
			
		GetOwner().AddChild(light, -1, EAddChildFlags.AUTO_TRANSFORM | EAddChildFlags.RECALC_LOCAL_TRANSFORM);
		light.SetLensFlareType(lightData.GetFlareType());
		light.SetLensFlareScale(lightData.GetLensFlareScale());
		light.SetLensFlareOffset(lightData.GetLensFlareOffset());
		light.SetConeAngle(lightData.GetLightConeAngle());
		light.SetIntensityEVClip(lightData.GetIntensityClipEV());
		
		return light;
	}
}
