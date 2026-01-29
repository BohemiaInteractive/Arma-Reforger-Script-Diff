[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sText", "Text: %1")]
class SCR_FieldManualPiece_Text : SCR_FieldManualPiece
{
	[Attribute(uiwidget: UIWidgets.EditBoxWithButton)]
	string m_sText;

	[Attribute(defvalue: "{D06D740EB045FC14}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_Text.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	ResourceName m_Layout;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_FieldManualPiece_Text()
	{
		if (m_Layout.Trim().IsEmpty())
		{
			m_Layout = "{D06D740EB045FC14}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_Text.layout";
		}
	}

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(m_Layout, parent);
		if (!createdWidget)
		{
			Print(string.Format("created widget (%2:%1) was null | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, m_Layout, m_sText), LogLevel.DEBUG);
			return;
		}

		TextWidget textWidget = TextWidget.Cast(createdWidget);
		if (!textWidget)
		{
			textWidget = TextWidget.Cast(SCR_WidgetHelper.GetWidgetOrChild(createdWidget, "Text"));
		}

		if (!textWidget)
		{
			Print("created widget could not be set text | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.DEBUG);
			return;
		}

		textWidget.SetText(m_sText);
	}
}
