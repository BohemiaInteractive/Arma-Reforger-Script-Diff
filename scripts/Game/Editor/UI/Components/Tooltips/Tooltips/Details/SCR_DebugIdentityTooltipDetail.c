[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_DebugIdentityTooltipDetail: SCR_EntityTooltipDetail
{			
	//------------------------------------------------------------------------------------------------
	override bool CreateDetail(SCR_EditableEntityComponent entity, Widget parent, TextWidget label, bool setFrameslot = true)
	{
		if (label)
			label.SetColor(Color.Pink);
		
		return super.CreateDetail(entity, parent, label, setFrameslot);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		if (!DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_GUI_TOOLTIP_DEBUG))
			return false;
		
		SCR_ExtendedIdentityComponent extendedIdentityComponent = SCR_ExtendedIdentityComponent.Cast(entity.GetOwner().FindComponent(SCR_ExtendedIdentityComponent));
		if (!extendedIdentityComponent)
			return false;
		
		TextWidget textWidget = TextWidget.Cast(widget);
		if (!textWidget)
			textWidget =  TextWidget.Cast(widget.FindAnyWidget("Text"));
		
		if (!textWidget)
			return false;
		
		textWidget.SetColor(Color.Pink);
		
		int bioGroupIndex, bioIndex;
		extendedIdentityComponent.GetBioIndexes(bioGroupIndex, bioIndex);
		textWidget.SetTextFormat("Identity seed: %1 | BioGroup: %2 | Bio: %3",  extendedIdentityComponent.GetRandomizeSeed(), bioGroupIndex, bioIndex);

		return true;
	}
}
