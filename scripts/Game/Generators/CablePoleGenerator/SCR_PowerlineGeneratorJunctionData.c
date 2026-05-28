[BaseContainerProps()]
class SCR_PowerlineGeneratorJunctionData
{
	[Attribute(desc: "Junction prefab to be used (if not specified, the generator's default one is used)", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")] //  class=SCR_PowerPole : no, allow power station too
	ResourceName m_sJunctionPrefab;

	[Attribute(defvalue: "1", desc: "Apply pitch and randomisation to this junction")]
	bool m_bApplyPitchAndRollRandomisation;

	[Attribute(defvalue: "0", desc: "Set the junction's yaw offset; can be used to setup the Prefab properly", uiwidget: UIWidgets.Slider, params: "-180 180")]
	float m_fYawOffset;

	[Attribute(defvalue: "0", desc: "Set the junction's altitude offset; helps preventing the junction Prefab to dig into the ground", params: string.Format("-%1 %1 0.01", MAX_Y_OFFSET), precision: 2)]
	float m_fYOffset;

	[Attribute(defvalue: "0", desc: "Define whether or not this junction is a power source")]
	bool m_bPowerSource;

	static int MAX_Y_OFFSET = 100;
}
