[BaseContainerProps(), BaseContainerCustomStringTitleField("Available Resources")]
class SCR_ResourceAvailableTooltipDetail : SCR_BaseResourceTooltipDetail
{
	//------------------------------------------------------------------------------------------------
	protected override SCR_ResourceConsumer GetConsumer()
	{
		if (!m_ResourceComponent)
			return null;
		
		return SCR_ResourceSystemHelper.GetAvailableResourceConsumer(m_ResourceComponent, m_eResourceType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool IsValid()
	{
		if (!m_ResourceComponent || !m_ResourceComponent.IsResourceTypeEnabled(m_eResourceType))
			return false;
		
		float resources;
		return SCR_ResourceSystemHelper.GetAvailableResources(m_ResourceComponent, resources);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		super.UpdateDetail(entity);
		
		if (!m_ResourceComponent)
			return;
	
		float resources;
		if (!SCR_ResourceSystemHelper.GetAvailableResources(m_ResourceComponent, resources))
			return;
		
		m_wText.SetTextFormat(m_sResourceFormatting, SCR_ResourceSystemHelper.SuppliesToString(resources));
	}
}
