[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_SuppliesBaseTooltipDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_Value;
	protected SCR_WLibProgressBarComponent m_Bar;
	protected SCR_CampaignSuppliesComponent m_SuppliesComponent;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
//		if (!m_SuppliesComponent)
//			return;
//
//		if (m_Bar)
//			m_Bar.SetValue(m_SuppliesComponent.GetSupplies() / m_SuppliesComponent.GetSuppliesMax());
//
//		if (m_Value)
//			m_Value.SetText(m_SuppliesComponent.GetSupplies().ToString());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
//		m_SuppliesComponent = SCR_CampaignSuppliesComponent.GetSuppliesComponent(entity.GetOwner());
//		if (!m_SuppliesComponent)
//			return false;
//
//		if (m_SuppliesComponent.GetSuppliesMax() <= 0)
//			return false;
//
//		m_Value = TextWidget.Cast(m_Widget.FindAnyWidget("Value"));
//
//		Widget barWidget = m_Widget.FindAnyWidget("ProgressBar");
//		if (barWidget)
//			m_Bar = SCR_WLibProgressBarComponent.Cast(barWidget.FindHandler(SCR_WLibProgressBarComponent));
//
//		return (m_Value || m_Bar);
		
		return false;
	}
}
