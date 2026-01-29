[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_DamageDisabledTooltipDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	
	[Attribute("loc INVINCIBLE")]
	protected LocalizedString m_sInvincibleText;
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{			
		m_Text = TextWidget.Cast(widget);
		if (!m_Text)
			return false;
		
		if (entity.Type() == SCR_EditablePlayerDelegateComponent)
			return false;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.GetOwner().FindComponent(DamageManagerComponent));
		if (damageManager && (damageManager.IsDamageHandlingEnabled() || damageManager.GetState() == EDamageState.DESTROYED))
			return false;
		
		m_Text.SetText(m_sInvincibleText);
		
		return true;
	}
}
