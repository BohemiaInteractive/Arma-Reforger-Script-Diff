[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FuelTooltipDetail : SCR_EntityTooltipDetail
{
	[Attribute(desc: "If any of these are set and none of the Ignore are then it is used to display the fuel tooltip. (Leave empty to get all and none SCR_FuelNodes are always displayed)", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EFuelNodeTypeFlag))]
	protected SCR_EFuelNodeTypeFlag m_eFuelNodeTypes;
	
	[Attribute(desc: "If any of these are present then the fuel node is ignored (Leave empty to ignore none and none SCR_FuelNodes are always displayed) ", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EFuelNodeTypeFlag))]
	protected SCR_EFuelNodeTypeFlag m_eIgnoreFuelNodeTypes;
	
	[Attribute("#AR-ValueUnit_Percentage")]
	protected string m_sPercentageText;
	
	protected TextWidget m_Value;
	protected SCR_WLibProgressBarComponent m_wBar;
	
	protected ref array<SCR_FuelManagerComponent> m_aFuelManagers = {};
	
	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		float totalFuel, totalMaxFuel, percentage;
		
		SCR_FuelManagerComponent.GetTotalValuesOfFuelNodesOfFuelManagers(m_aFuelManagers, totalFuel, totalMaxFuel, percentage, m_eFuelNodeTypes, m_eIgnoreFuelNodeTypes);
		
		if (m_wBar)
			m_wBar.SetValue(percentage);

		if (m_Value)
			m_Value.SetTextFormat(m_sPercentageText, Math.Round(percentage * 100));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Value = TextWidget.Cast(m_Widget.FindAnyWidget("Value"));
		
		Widget barWidget = m_Widget.FindAnyWidget("ProgressBar");
		if (barWidget)
			m_wBar = SCR_WLibProgressBarComponent.Cast(barWidget.FindHandler(SCR_WLibProgressBarComponent));
		
		if (!m_Value && !m_wBar)
			return false;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.GetOwner().FindComponent(DamageManagerComponent));
		if (damageManager && damageManager.GetState() == EDamageState.DESTROYED)
			return false;
		
		SCR_FuelManagerComponent.GetAllFuelManagers(entity.GetOwner(), m_aFuelManagers);
		if (m_aFuelManagers.IsEmpty())
			return false;
		
		float totalFuel, totalMaxFuel, percentage;
		SCR_FuelManagerComponent.GetTotalValuesOfFuelNodesOfFuelManagers(m_aFuelManagers, totalFuel, totalMaxFuel, percentage, m_eFuelNodeTypes, m_eIgnoreFuelNodeTypes);
		
		return totalMaxFuel > 0;
	}
}
