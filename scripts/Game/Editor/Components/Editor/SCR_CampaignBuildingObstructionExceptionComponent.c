//! Prefab with this component will be skipped by Campaign Building obstruction system when evaluating blocking entities.
class SCR_CampaignBuildingObstructionExceptionComponentClass : ScriptComponentClass
{
}

class SCR_CampaignBuildingObstructionExceptionComponent : ScriptComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "List of prefab that can colide with this entity on Free Roam building obstruction test.", params: "et")]
	protected ref array<ResourceName> m_aWhiteListPrefabs;
	
	//------------------------------------------------------------------------------------------------
	//! return true when there is no prefab set on the whitelist.
	bool IsWhitelistEmpty()
	{
		return m_aWhiteListPrefabs.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the given ID is ont he list of whitelisted prefabs
	bool IsOnWhitelist(ResourceName resName)
	{
		return m_aWhiteListPrefabs.Contains(resName);
	}
}
