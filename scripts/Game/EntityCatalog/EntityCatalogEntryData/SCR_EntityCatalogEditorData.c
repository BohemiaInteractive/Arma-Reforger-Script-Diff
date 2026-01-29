//! Info for which editor modes the entity is added to
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntWithFlagTitleField(EEditorMode, "m_eValidEditorModes", "m_bEnabled", 1, "EditorData - %1", "DISABLED - EditorData")]
class SCR_EntityCatalogEditorData : SCR_BaseEntityCatalogData
{
	[Attribute("1", desc: "Editor modes the entity can be placed in (If the content Browser component takes data from the catalog). If the mode is not selected than the they cannot be placed in that mode", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditorMode))]
	protected EEditorMode m_eValidEditorModes;
	
	//---------------------------------------------------------------------------------------------
	// !Check if entry is valid for given mode
	// !\param[in] editor mode to check for entry
	// !\return true if mode is valid for this entry
	bool IsValidInEditorMode(EEditorMode editorMode)
	{
		return SCR_Enum.HasFlag(m_eValidEditorModes, editorMode);
	}
	
	//---------------------------------------------------------------------------------------------
	//! \Return given valid editor modes
	EEditorMode GetValidEditorModes()
	{
		return m_eValidEditorModes;
	}
}
