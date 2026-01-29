[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_HealthTooltipDetail : SCR_DpsConditionBarBaseTooltipDetail
{
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		super.UpdateDetail(entity);
		
		SetBarAndPercentageValue(m_DamageManager.GetHealthScaled());
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		if (!super.InitDetail(entity, widget))
			return false;
		
		if (m_DamageManager.GetState() == EDamageState.DESTROYED)
			return false; 

		return m_DamageManager.IsDamageHandlingEnabled();
	}
}
