[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sDescription", "Keybind: %1")]
class SCR_FieldManualPiece_Keybind : SCR_FieldManualPiece
{
	[Attribute("<action name=\"ActionName\"/>", uiwidget: UIWidgets.LocaleEditBox)]
	string m_sKeybind;

	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	string m_sDescription;

	[Attribute(defvalue: "{76A21E3BDBC15712}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_Keybind.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	ResourceName m_Layout; // m_s

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		CreateWidget(parent, true);
	}

	//------------------------------------------------------------------------------------------------
	void CreateWidget(notnull Widget parent, bool isLastElement)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(m_Layout, parent);
		if (!createdWidget)
			return;

		TextWidget keybindWidget = TextWidget.Cast(createdWidget.FindAnyWidget("Keybind"));
		if (keybindWidget)
			keybindWidget.SetText(m_sKeybind);

		TextWidget descriptionWidget = TextWidget.Cast(createdWidget.FindAnyWidget("Description"));
		if (descriptionWidget)
			descriptionWidget.SetText(m_sDescription);
		
		if (!isLastElement)
			return;

		Widget separator = createdWidget.FindAnyWidget("Separator");
		if (separator)
			separator.SetOpacity(0);
	}
}
