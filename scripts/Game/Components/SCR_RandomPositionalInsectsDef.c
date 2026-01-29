[BaseContainerProps(configRoot: true)]
class SCR_RandomPositionalInsectsDef
{
	[Attribute(defvalue: "", UIWidgets.ResourceNamePicker, desc: "", params: "conf")]
	ResourceName m_sInsectGroup;

	[Attribute(defvalue: "", UIWidgets.ResourceNamePicker, desc: "", params: "conf")]
	ResourceName m_sSpawnDef;

	[Attribute(defvalue: "50", UIWidgets.Slider, params: "0 100 1", desc: "When suitable place is found, with what chance is Insect spawned there")]
	int m_iSpawnChance;
}
