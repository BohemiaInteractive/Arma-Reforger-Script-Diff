
//------------------------------------------------------------------------------------------------
//! Light parameters
[BaseContainerProps()]
class SCR_BaseLightData
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(LightType))]
	private LightType m_eLightType;	
	
	[Attribute("0 0.3 0", UIWidgets.EditBox, "Light offset in local space from the origin of the entity", params: "inf inf purpose=coords space=entity")]
	private vector m_vLightOffset;
	
	[Attribute("1 1 1", UIWidgets.ColorPicker, "Color of the light")]
	private vector m_vLightColor;
	
	[Attribute("0 0 1", UIWidgets.EditBox, "Direction of the light cone (useful for LightType.SPOT only).")]
	private vector m_vLightConeDirection;
	
	[Attribute("0", UIWidgets.EditBox, "Angle of the light cone (useful for LightType.SPOT only).")]
	private float m_fConeAngle;
	
	[Attribute("0", UIWidgets.EditBox, "Radius of the light effect")]
	private float m_fRadius;
	
	[Attribute("0", UIWidgets.EditBox, "Clip intensity value of light effect in EV, use negative value to see light even in brighter times.", params: "-10 10 0.1")]
	private float m_fIntensityClipEV;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(LightLensFlareType))]
	private LightLensFlareType m_eFlareType;
	
	[Attribute("0", UIWidgets.EditBox, "Radius of the light effect")]
	private vector m_vFlareOffset;
	
	[Attribute("0", UIWidgets.EditBox, "Scale of lens flare")]
	private float m_fScale;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(LightFlags))]
	private LightFlags m_eLightFlags;
	
	//------------------------------------------------------------------------------------------------
	vector GetLightOffset()
	{
		return m_vLightOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetLightColor()
	{
		return m_vLightColor;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetLightConeDirection()
	{
		return m_vLightConeDirection;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLightConeAngle()
	{
		return m_fConeAngle;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetEffectRadius()
	{
		return m_fRadius;
	}
		
	//------------------------------------------------------------------------------------------------
	float GetIntensityClipEV()
	{
		return m_fIntensityClipEV;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetLensFlareOffset()
	{
		return m_vFlareOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLensFlareScale()
	{
		return m_fScale;
	}
	
	//------------------------------------------------------------------------------------------------
	LightLensFlareType GetFlareType()
	{
		return m_eFlareType;
	}
	
	//------------------------------------------------------------------------------------------------
	LightType GetLightType()
	{
		return m_eLightType;
	}	
	
	//------------------------------------------------------------------------------------------------
	LightFlags GetLightFlag()
	{
		return m_eLightFlags;
	}
};