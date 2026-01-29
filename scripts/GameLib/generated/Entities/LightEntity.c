/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class LightEntityClass: GenericEntityClass
{
}

/*!
Use LightEntity::CreateLight instead of spawning LightEntity via Game::SpawnEntity for the creation of light in a script.
If you spawn this entity with a prefab, all parameters sets will be automatically applied.
This should be used over LightHandle unless you want a low-level API.
*/
class LightEntity: GenericEntity
{
	/*!
	Create new static light by type
	\param type Type of light
	\param flags Creation flags.
	\param radius Radius
	\param color Color in RGB form
	\param LV 		LV value of a light, final light color is combined from color and LV properties (color * LV^2)
	\param position Position of the entity in world coordinates
	\param direction Direction of spot-vector (used for spotlights)
	\return LightEntity
	*/
	static proto LightEntity CreateLight(LightType type, LightFlags flags, float radius, Color color, float LV, vector position, vector direction = vector.Forward);
	/*!
	Set the enabled state of the light.
	\param state Enable state.
	*/
	proto external void SetEnabled(bool state);
	//! Returns true if the light is enabled
	proto external bool IsEnabled();
	/*!
	Set light flags
	\warning If you want to convert a static light to dynamic light, you need to delete the current static light and spawn a new light with the dynamic flag set.
	\param 	flags new flags combined internally with older flags
	\return currently set flags
	*/
	proto external int SetLightFlags(LightFlags flags);
	/*!
	Clear light flags
	\param flags 	flags to be cleared
	\return currently set flags
	*/
	proto external int ClearLightFlags(LightFlags flags);
	//! Returns true if the flags is set.
	proto external bool IsLightFlagsSet(LightFlags flags);
	//! Returns true if the light is dynamic, false otherwise
	proto external bool IsDynamic();
	proto external void SetCastShadow(bool enable);
	//! Returns true if the light is casting shadows.
	proto external bool IsCastShadow();
	/*!
	Set light's new color
	\param color 	color of a light
	\param LV 		LV value of a light, final light color is combined from color and LV properties (color * LV^2)
	*/
	proto external void SetColor(Color color, float LV);
	/*!
	Set light's new radius
	\param radius 	light radius in meters
	*/
	proto external void SetRadius(float radius);
	//! Returns the current radius of the light in meters.
	proto external float GetRadius();
	/*!
	Sets light intensity clip in EV. Lights is clipped when its intensity is low but this could be too early for some lights.
	This value is bias to move the intensity clip test. The value is in EV thus 1 unit means double of light
	\param clipEV		Negative values make the test less strict, useful e.g. for lights with low intensities lying near geometry
	*/
	proto external void SetIntensityEVClip(float clipEV);
	/*!
	Sets light cone angle in degrees (useful for LightType.SPOT only)
	*/
	proto external void SetConeAngle(float angle);
	/*!
	Set light distance attenuation -> power function, default is 2 corresponding to light without
	any other internal reflector
	\param att attenuation
	*/
	proto external void SetDistanceAtt(float att);
	/*!
	Sets light cone angle attenuation (SPOT)
	\param coneAngleAttenuation <0..1> value, where 0 = full angle attenuation, 1 = no angle attenuation
	*/
	proto external void SetConeAngleAttenuation(float coneAngleAttenuation);
	//! Returns the current cone angle in degrees (useful for LightType.SPOT only)
	proto external float GetConeAngle();
	/*!
	Sets color-lookup texture, light color with LV is multiplied by the texture
	* point lights -> texture must be cube map
	* spot lights, 2D texture -> the texture serves as projector texture
	* spot light, 1D texture of <someWidth, 1> size -> texture serves is angle lookup texture (parametrized by cos^2 angle
	from spot direction and pixel world pos, IES profile, but not supported fully now as we don't have IES converter)
	\param textureName Name of texture
	*/
	proto external void SetTexture(ResourceName textureName);
	/*!
	Sets near plane of a light. It's meaningful for shadow casting LT_SPOT/LT_POINT lights only!
	\param nearPlane	near plane when casting shadows in meters
	*/
	proto external void SetNearPlane(float nearPlane);
	//! Returns the current near plane of the light in meters.
	proto external float GetNearPlane();
	/*!
	Sets light volumetric effect
	\param intensity	vol light intensity
	\param offset			signed offset of light volume effect according base light
	\param scale			scale of light (mainly for optimization)
	*/
	proto external void SetVolumeEffect(float intensity, float offset, float scale);
	/*!
	Sets lens flare type of a light.
	\param type 		Lens flare type (Automatic = lens flare is determined by light type, Manual = use index, Disabled = lens flare disabled)
	*/
	proto external void SetLensFlareType(LightLensFlareType type);
	/*!
	Sets lens flare index of a light. Use value < 0 to disable lens flare
	\param index		Index of the lens flare in the world lens flare config
	*/
	proto external void SetLensFlareIndex(int index);
	/*!
	Sets lens flare scale for a light
	\param scale		Scale of the lens flare
	*/
	proto external void SetLensFlareScale(float scale);
	/*!
	Sets lens flare offset for a light
	\param offset		Offset from the light original position vector(offset, offsetUp, offsetSide)
	*/
	proto external void SetLensFlareOffset(vector offset);
	/*!
	Sets direction of light. It's meaningful for LT_SPOT/LT_DIRECTIONAL only!
	\param direction Direction of light
	*/
	proto external void SetLightDirection(vector direction);
	/*!
	Set mask for light vs camera visibility. By default, all bits are set
	what means the light will be visible in all cameras
	\param mask Bit-mask of cameras where is light visible
	\return Returns previous mask
	*/
	proto external int SetLightCameraMask(int mask);
}

/*!
\}
*/
