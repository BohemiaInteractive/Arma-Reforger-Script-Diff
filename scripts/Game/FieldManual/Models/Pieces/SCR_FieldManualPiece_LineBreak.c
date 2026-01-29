[BaseContainerProps(insertable: false), SCR_BaseContainerStaticTitleField("<br/>")]
class SCR_FieldManualPiece_LineBreak : SCR_FieldManualPiece
{
	protected const ResourceName LAYOUT = "{D06D740EB045FC14}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_Text.layout";

	override void CreateWidget(notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(LAYOUT, parent);
		if (!createdWidget)
		{
			Print(string.Format("created widget (%1) was null | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LAYOUT), LogLevel.DEBUG);
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

		textWidget.SetText("");
	}
}
