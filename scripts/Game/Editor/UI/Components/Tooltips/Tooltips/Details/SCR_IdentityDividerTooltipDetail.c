[BaseContainerProps(), BaseContainerCustomStringTitleField("<IDENTITY DIVIDER>")]
class SCR_IdentityDividerTooltipDetail: SCR_EntityTooltipDetail
{
	[Attribute(desc: "Only show if IdentityComponent has Bio")]
	protected bool m_bNeedsBio;
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		SCR_IdentityManagerComponent identityManager =  SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager)
			return false;
		
		SCR_ExtendedIdentityComponent identityComponent = SCR_ExtendedIdentityComponent.Cast(entity.GetOwner().FindComponent(SCR_ExtendedIdentityComponent));
		if (!identityComponent)
			return false;
		
		//~ Check if allowed to show bio
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		bool showBioTooltip = true;
		
		if (editorSettings)
			editorSettings.Get("m_bShowIdentityBioTooltip", showBioTooltip);
		
		if (!showBioTooltip)
			return false;
		
		return !m_bNeedsBio || (m_bNeedsBio && identityComponent.GetIdentityBio() != null && !identityComponent.GetIdentityBio().GetBioText().IsEmpty());
	}
}