[BaseContainerProps()]
class SCR_PowerlineGeneratorJunctionData
{
	[Attribute(desc: "Junction prefab to be used (if not specified, the generator's default one is used)", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")] //  class=SCR_PowerPole : no, allow power station too
	ResourceName m_sJunctionPrefab;

	[Attribute(defvalue: "1", desc: "Apply pitch and randomisation to this junction")]
	bool m_bApplyPitchAndRollRandomisation;

	[Attribute(defvalue: "0", desc: "Set the junction's yaw offset; can be used to setup the Prefab properly", uiwidget: UIWidgets.Slider, params: "-180 180")]
	float m_fYawOffset;

	[Attribute(defvalue: "0", desc: "Set the junction's altitude offset; helps preventing the junction Prefab to dig into the ground", params: "-100 100 0.01", precision: 2)]
	float m_fYOffset;

	[Attribute(defvalue: "0", desc: "Define whether or not this junction is a power source")]
	bool m_bPowerSource;
}
