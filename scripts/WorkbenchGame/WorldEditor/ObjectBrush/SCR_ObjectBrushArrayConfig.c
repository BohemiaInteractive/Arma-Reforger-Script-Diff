#ifdef WORKBENCH // this config is only related to a Workbench tool (SCR_ObjectBrushTool)
[BaseContainerProps(configRoot: true)]
class SCR_ObjectBrushArrayConfig
{
	[Attribute()]
	ref array<ref SCR_ObjectBrushObjectBase> m_aObjectArray;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ObjectBrushArrayConfig()
	{
		if (!m_aObjectArray)
			m_aObjectArray = {};
	}
}

[BaseContainerProps()]
class SCR_ObjectBrushObjectBase : SCR_ForestGeneratorTreeBase
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "Maximum random Yaw angle", "0 180 1")]
	float m_fRandomYawAngle;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "Sets the prefabs Y position offset", "-1000 1000 0.5")]
	float m_fPrefabOffsetY;

	[Attribute(defvalue: "0", params: "-1000 0 inf", precision: 2, uiwidget: UIWidgets.Slider, desc: "Minimum random vertical offset, considers Prefab Offset Y")]
	float m_fMinRandomVerticalOffset;

	[Attribute(defvalue: "0", params: "0 1000 inf", precision: 2, uiwidget: UIWidgets.Slider, desc: "Maximum random vertical offset, considers Prefab Offset Y")]
	float m_fMaxRandomVerticalOffset;

	[Attribute(defvalue: "0", desc: "Override the default prefab randomization")]
	bool m_bOverrideRandomization;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Sets the probability of this object being selected", "0 100000 1")]
	float m_fWeight;

	[Attribute(defvalue: "0", desc: "Should the prefab align to the terrain?")]
	bool m_bAlignToNormal;

	[Attribute(defvalue: "0", desc: "Should this prefab be affected by the scale falloff?")]
	bool m_bScaleFalloff;

	[Attribute(defvalue: "0.8", uiwidget: UIWidgets.Slider, desc: "Min scale after fall off of this object", "0 1000 0.1")]
	float m_fLowestScaleFalloffValue;

	int m_iSubareaIndex;
}
#endif // WORKBENCH
