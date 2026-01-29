[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FullBioIdentityTooltipDetail: SCR_EntityTooltipDetail
{		
	[Attribute(defvalue: "0.5 0.5 0.5 1", desc: "Color of Text.")]
	protected ref Color m_WidgetColor;
	
	//------------------------------------------------------------------------------------------------
	override bool CreateDetail(SCR_EditableEntityComponent entity, Widget parent, TextWidget label, bool setFrameslot = true)
	{
		if (label)
			label.SetColor(m_WidgetColor);
		
		return super.CreateDetail(entity, parent, label, setFrameslot);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{		
		SCR_ExtendedIdentityComponent extendedIdentityComponent = SCR_ExtendedIdentityComponent.Cast(entity.GetOwner().FindComponent(SCR_ExtendedIdentityComponent));
		if (!extendedIdentityComponent)
			return false;
		
		//~ Check if allowed to show bio
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		bool showBioTooltip = true;
		
		if (editorSettings)
			editorSettings.Get("m_bShowIdentityBioTooltip", showBioTooltip);
		
		if (!showBioTooltip)
			return false;
		
		TextWidget textWidget = TextWidget.Cast(widget);
		if (!textWidget)
			textWidget = TextWidget.Cast(widget.FindAnyWidget("Text"));
		
		if (!textWidget)
			return false;
		
		textWidget.SetColor(m_WidgetColor);
		
		SCR_IdentityBio bio = extendedIdentityComponent.GetIdentityBio();
		
		if (!bio)
			return false;
		
		textWidget.SetText(bio.GetBioText());
	
		return true;
	}
}




