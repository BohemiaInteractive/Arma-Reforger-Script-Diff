[BaseContainerProps(configRoot: true), BaseContainerCustomTitleField("m_sLoadoutName")]
class SCR_PlayerLoadout : SCR_BasePlayerLoadout
{
	[Attribute("Loadout", UIWidgets.EditBox, params: "edds")]
	string m_sLoadoutName;
	
	[Attribute("", UIWidgets.ResourceNamePicker, params: "et")]
	ResourceName m_sLoadoutResource;
	
	[Attribute("", UIWidgets.ResourceNamePicker, params: "edds")]
	ResourceName m_sLoadoutImage;

	//------------------------------------------------------------------------------------------------
	override ResourceName GetLoadoutResource()
	{
		return SCR_EditableEntityComponentClass.GetRandomVariant(m_sLoadoutResource);
	}
	
	//------------------------------------------------------------------------------------------------
	override ResourceName GetLoadoutImageResource()
	{
		return m_sLoadoutImage;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetLoadoutName()
	{
		return m_sLoadoutName;
	}
}
