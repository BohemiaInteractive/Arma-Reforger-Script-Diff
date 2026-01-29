[BaseContainerProps()]
class SCR_InsectDef
{
	[Attribute(desc: "Sound name to be used")]
	string 	m_sSoundName;

	[Attribute(desc: "Particle effect to be created (Do not use together with Prefab Name as Prefabs will be preferred first)")]
	ResourceName 	m_sParticleEffect;

	[Attribute(desc: "Insect prefab to be created (Do not use together with Particle Effect as Prefabs will be preferred first)")]
	ResourceName 	m_sPrefabName;

	[Attribute("1", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EDayTimeCurve))]
	EDayTimeCurve m_eDayTimeCurve;
}
