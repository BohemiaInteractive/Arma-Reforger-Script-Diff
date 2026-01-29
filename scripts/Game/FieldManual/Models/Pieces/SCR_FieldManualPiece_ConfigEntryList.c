[BaseContainerProps(), SCR_BaseContainerStaticTitleField(customTitle: "Config Entry List")]
class SCR_FieldManualPiece_ConfigEntryList : SCR_FieldManualPiece
{
	[Attribute()]
	protected ref array<ref SCR_FieldManualPiece_ConfigEntry> m_aConfigEntries;

	[Attribute(defvalue: "{A89964BD3E21BEED}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_ConfigEntryList.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	protected ResourceName m_Layout;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_FieldManualPiece_ConfigEntryList()
	{
		if (!m_aConfigEntries) // can be config-provided
		{
			m_aConfigEntries = {};
		}
	}

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		Widget createdWidget = workspace.CreateWidgets(m_Layout, parent);
		if (!createdWidget)
		{
			Print("could not create Config Entry List widget | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return;
		}

		Widget configEntriesLayout = SCR_WidgetHelper.GetWidgetOrChild(createdWidget, "ConfigEntriesLayout");
		if (configEntriesLayout)
			CreateConfigEntries(configEntriesLayout);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateConfigEntries(Widget configEntriesLayout)
	{
		SCR_WidgetHelper.RemoveAllChildren(configEntriesLayout);

		if (!m_aConfigEntries || m_aConfigEntries.IsEmpty())
			return;

		for (int i, cnt = m_aConfigEntries.Count(); i < cnt; i++)
		{
			m_aConfigEntries[i].CreateWidget(configEntriesLayout);
		}
	}
}
