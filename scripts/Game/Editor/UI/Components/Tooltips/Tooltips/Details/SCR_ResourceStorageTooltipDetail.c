[BaseContainerProps(), BaseContainerCustomStringTitleField("Stored Resources")]
class SCR_ResourceStorageTooltipDetail : SCR_BaseResourceTooltipDetail
{
	//------------------------------------------------------------------------------------------------
	protected override SCR_ResourceConsumer GetConsumer()
	{
		if (!m_ResourceComponent)
			return null;
		
		return SCR_ResourceSystemHelper.GetStorageConsumer(m_ResourceComponent, m_eResourceType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool IsValid()
	{
		if (!m_ResourceComponent)
			return false;
		
		float resources, maxResources;
		return SCR_ResourceSystemHelper.GetStoredAndMaxResources(m_ResourceComponent, resources, maxResources);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		super.UpdateDetail(entity);
		
		if (!m_ResourceComponent)
			return;
	
		float stored, maxStored;
		if (!SCR_ResourceSystemHelper.GetStoredAndMaxResources(m_ResourceComponent, stored, maxStored))
			return;
		
		m_wText.SetTextFormat(m_sResourceFormatting, SCR_ResourceSystemHelper.SuppliesToString(stored), SCR_ResourceSystemHelper.SuppliesToString(maxStored));
	}
}
