[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Component for nametag config selection")]
class SCR_NametagConfigComponentClass : ScriptComponentClass
{}

//------------------------------------------------------------------------------------------------
//! Attached to BaseGameMode, used for nametag config selection
class SCR_NametagConfigComponent : ScriptComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Nametag config", "conf class=SCR_NameTagConfig")]
	protected ResourceName m_sConfigPath;
	
	//------------------------------------------------------------------------------------------------
	//! Get patch to nametag config
	ResourceName GetConfigPath()
	{
		return m_sConfigPath;
	}

};
