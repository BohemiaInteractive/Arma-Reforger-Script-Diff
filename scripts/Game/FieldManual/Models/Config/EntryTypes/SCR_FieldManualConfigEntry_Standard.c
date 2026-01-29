[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sTitle", "Standard: %1")]
class SCR_FieldManualConfigEntry_Standard : SCR_FieldManualConfigEntry
{



	[Attribute(defvalue: "{E7BD479E6A46BCC4}UI/layouts/Menus/FieldManual/Entries/FieldManual_Entry_Standard.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	ResourceName m_Layout;



	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_FieldManualConfigEntry_Standard()
	{
		if (m_Layout.Trim().IsEmpty())
			m_Layout = "{E7BD479E6A46BCC4}UI/layouts/Menus/FieldManual/Entries/FieldManual_Entry_Standard.layout";
	}

	//------------------------------------------------------------------------------------------------
	override Widget CreateWidget(notnull Widget parent)
	{
		return CreateWidgetFromLayout(m_Layout, parent);
	}
};
