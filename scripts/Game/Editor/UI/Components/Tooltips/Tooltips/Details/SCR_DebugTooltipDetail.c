[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_DebugTooltipDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] object
	//! \return
	string GetGUID(Managed object)
	{
		string result = object.ToString();
		int indexLeft = result.IndexOf("<") + 1;
		int indexRight = result.IndexOf(">");
		return result.Substring(indexLeft, indexRight - indexLeft);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		string guid = GetGUID(entity);
		string prefab = FilePath.StripPath(entity.GetPrefab());
		m_Text.SetTextFormat("%1<br />%2<br />%3", guid, prefab, SCR_Enum.FlagsToString(EEditableEntityFlag, entity.GetEntityFlags()));
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = TextWidget.Cast(widget);
		m_Text.SetColor(Color.FromInt(Color.PINK));
		return m_Text && DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_GUI_TOOLTIP_DEBUG);
	}
}
