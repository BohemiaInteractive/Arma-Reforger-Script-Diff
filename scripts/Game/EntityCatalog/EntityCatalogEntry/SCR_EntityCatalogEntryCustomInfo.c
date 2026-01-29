/**
Entity Entry within the SCR_EntityCatalog. Will allow for custom UIInfo, overwriting any that is set by other systems
*/
[BaseContainerProps(), BaseContainerCustomDoubleCheckIntResourceNameTitleField("m_bEnabled", "m_sEntityPrefab", 1, "(Custom Info) %1", "DISABLED - (Custom Info) %1")]
class SCR_EntityCatalogEntryCustomInfo : SCR_EntityCatalogEntryNonEditable
{
	[Attribute(desc: "Set the UI info for the non-editable entity. NOTE! Only use this when the Prefab is non-editable otherwise it will take this UIinfo instead of the EditableEntity UI info on the prefab. If this is required add your own UIInfo to a custom SCR_BaseEntityCatalogData to prevent breaking data for other")]
	protected ref SCR_UIInfo m_UiInfo;
	
	//======================================== UI INFO ========================================\\
	//--------------------------------- Get UI Info ---------------------------------\\
	override SCR_UIInfo GetEntityUiInfo()
	{
		return m_UiInfo;
	}
}