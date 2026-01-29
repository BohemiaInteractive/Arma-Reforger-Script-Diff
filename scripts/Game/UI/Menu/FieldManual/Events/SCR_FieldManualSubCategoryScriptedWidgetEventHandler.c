class SCR_FieldManualSubCategoryScriptedWidgetEventHandler : ScriptedWidgetEventHandler
{
	protected SCR_FieldManualUI m_UI;

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_UI)
			m_UI.OnSubCategoryClicked(w);

		return m_UI != null;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] ui
	void SCR_FieldManualSubCategoryScriptedWidgetEventHandler(notnull SCR_FieldManualUI ui)
	{
		m_UI = ui;
	}
}
