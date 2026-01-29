/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

sealed class LightHandle: pointer
{
	/*!
	Create new static light by type
	\param type Type of light
	\param flags Creation flags.
	\param radius Radius
	\param color Color in RGB form
	\param LV 		LV value of a light, final light color is combined from color and LV properties (color * LV^2)
	\param origin Origin coordinates
	\param direction Direction of spot-vector (used for spotlights)
	\return LightHandle
	*/
	static proto LightHandle AddStaticLight(notnull BaseWorld world, LightType type, LightFlags flags, float radius, notnull Color color, float LV, vector origin, vector direction = vector.Zero);
	/*!
	Create new dynamic light
	\param owner Owner entity
	\param type Type of light
	\param flags Creation flags. See enf::LightFlags
	\param radius Radius
	\param color Color in RGB form
	\param LV 		LV value of a light, final light color is combined from color and LV properties (color * LV^2)
	\return LightHandle
	*/
	static proto LightHandle AddDynamicLight(notnull IEntity owner, LightType type, LightFlags flags, float radius, notnull Color color, float LV);
	/*!
	set light flags
	\param 	flags new flags combined internally with older flags
	\return currently set flags
	*/
	proto external int SetFlags(LightFlags flags);
	/*!
	clear light flags
	\param flags 	flags to be cleared
	\return currently set flags
	*/
	proto external int ClearFlags(LightFlags flags);
	/*!
	removes light
	*/
	proto external void RemoveLight(notnull BaseWorld world);
	/*!
	set light's new parameters
	\param radius 	light radius
	\param color 	color of a light
	\param LV 		LV value of a light, final light color is combined from color and LV properties (color * LV^2)
	*/
	proto external void Set(notnull BaseWorld world, float radius, notnull Color color, float LV);
	/*!
	Sets light intensity clip in EV. Lights is clipped when its intensity is low but this could be too early for some lights.
	This value is bias to move the intensity clip test. The value is in EV thus 1 unit means double of light
	\param clipEV		Negative values make the test less strict, useful e.g. for lights with low intensities lying near geometry
	*/
	proto external void SetIntensityEVClip(notnull BaseWorld world, float clipEV);
	/*!
	Sets light cone in degrees (useful for LightType.SPOT only)
	*/
	proto external void SetCone(notnull BaseWorld world, float cone);
	/*!
	Set light distance attenuation -> power function, default is 2 corresponding to light without
	any other internal reflector
	*/
	proto external void SetLightDistanceAtt(notnull BaseWorld world, float attenuation);
	/*!
	Sets color-lookup texture, light color with LV is multiplied by the texture
	* point lights -> texture must be cube map
	* spot lights, 2D texture -> the texture serves as projector texture
	* spot light, 1D texture of <someWidth, 1> size -> texture serves is angle lookup texture (parametrized by cos^2 angle
	from spot direction and pixel world pos, IES profile, but not supported fully now as we don't have IES converter)
	*/
	proto external void SetTexture(notnull BaseWorld world, ResourceName textureName);
	/*!
	Sets light projection texture blending parameters
	\param mip0			LUP texture mipmap blend start in normalized <0..1> range
	\param mipMax		LUP texture mipmap blend end in normalized <0..1> range
	*/
	proto external void SetProjectionTextureBlendParams(notnull BaseWorld world, float mip0, float mipMax);
	/*!
	Sets near plane of a light. It's meaningful for shadow casting LT_SPOT/LT_POINT lights only!
	\param nearPlane	near plane when casting shadows
	*/
	proto external void SetNearPlane(notnull BaseWorld world, float nearPlane);
	/*!
	Sets light volumetric effect
	\param intensity	vol light intensity
	\param offset			signed offset of light volume effect according base light
	\param scale			scale of light (mainly for optimization)
	*/
	proto external void SetVolumeEffect(notnull BaseWorld world, float intensity, float offset, float scale);
	/*!
	Sets lens flare type of a light.
	\param type 		Lens flare type (Automatic = lens flare is determined by light type, Manual = use index, Disabled = lens flare disabled)
	*/
	proto external void SetLensFlareType(notnull BaseWorld world, LightLensFlareType type);
	/*!
	Sets lens flare index of a light. Use value < 0 to disable lens flare
	\param index		Index of the lens flare in the world lens flare config
	*/
	proto external void SetLensFlareIndex(notnull BaseWorld world, int index);
	/*!
	Sets lens flare scale for a light
	\param scale		Scale of the lens flare
	*/
	proto external void SetLensFlareScale(notnull BaseWorld world, float scale);
	/*!
	Sets lens flare offset for a light
	\param offset		Offset from the light original position: vector(offset, offsetUp, offsetSide)
	*/
	proto external void SetLensFlareOffset(notnull BaseWorld world, vector offset);
	/*!
	Sets position of light. It's meaningful for LT_SPOT/LT_DIRECTIONAL only!
	\param position Position of light
	*/
	proto external void SetPosition(notnull BaseWorld world, vector position);
	/*!
	Sets direction of light. It's meaningful for LT_SPOT/LT_DIRECTIONAL only!
	\param direction Direction of light
	*/
	proto external void SetDirection(notnull BaseWorld world, vector direction);
	/*!
	Sets transform of light. It's meaningful for static LT_SPOT/LT_DIRECTIONAL only!
	\param transform Transform matrix of light
	*/
	proto external void SetTransform(notnull BaseWorld world, vector[] transform);
	/*!
	Set mask for light vs camera visibility. By default, all bits are set
	what means the light will be visible in all cameras
	\param mask Bit-mask of cameras where is light visible
	\return Returns previous mask
	*/
	proto external int SetCameraMask(int mask);
}

/*!
\}
*/
