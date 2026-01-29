enum ESpawnMethod
{
	ENTITY,
	TERRAIN,
	SOUNDMAP
}

[BaseContainerProps(configRoot: true)]
class SCR_InsectGroup
{
	[Attribute("1", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(ESpawnMethod))]
	ESpawnMethod m_eSpawnMethod;

	[Attribute("", UIWidgets.EditBox, "Sound event name")]
	string m_sSoundEvent;

	[Attribute("", UIWidgets.Object, "")]
	ref array<ref SCR_InsectType> m_aInsectType;
}
