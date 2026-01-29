class SCR_CharacterEditableEntityUIComponent : SCR_CustomEditableEntityUIComponent
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds")]
	protected ResourceName m_PlayerIcon;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		if (entity && entity.HasEntityState(EEditableEntityState.PLAYER))
		{
			ImageWidget iconWidget = ImageWidget.Cast(GetWidget().FindAnyWidget(m_sIconWidgetName));
			iconWidget.LoadImageTexture(0, m_PlayerIcon);
		}
		else
		{
			super.OnInit(entity, info, slot);
		}
	}
}
