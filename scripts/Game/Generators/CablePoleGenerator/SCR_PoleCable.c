[BaseContainerProps()]
class SCR_PoleCable
{
	[Attribute(defvalue: SCR_EPoleCableType.POWER_LV.ToString(), desc: "Targeted cable type", uiwidget: UIWidgets.ComboBox, enumType: SCR_EPoleCableType)]
	SCR_EPoleCableType m_eType;

	[Attribute(defvalue: "{AFC481E5CDFB2DF2}Assets/Structures/Infrastructure/Power/Powerlines/data/HV_01_powerline_wire.emat", params: "emat", desc: "Material applied to cables of this type")]
	ResourceName m_sMaterial;
}
