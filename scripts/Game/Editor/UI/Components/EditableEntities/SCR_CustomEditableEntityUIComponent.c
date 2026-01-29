class SCR_CustomEditableEntityUIComponent : SCR_BaseEditableEntityUIComponent
{
	[Attribute("Icon")]
	protected string m_sIconWidgetName;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		if (info)
		{
			//--- Set custom texture
			ImageWidget iconWidget = ImageWidget.Cast(GetWidget().FindAnyWidget(m_sIconWidgetName));
			info.SetIconTo(iconWidget);
		}
	}
}
