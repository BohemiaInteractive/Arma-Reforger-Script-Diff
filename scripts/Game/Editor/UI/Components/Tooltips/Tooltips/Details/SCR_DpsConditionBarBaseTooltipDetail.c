[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_DpsConditionBarBaseTooltipDetail : SCR_EntityTooltipDetail
{
	[Attribute("#AR-ValueUnit_Percentage")]
	private string m_sPercentageText;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Colors the bar if damage over time condition is true", enums: ParamEnumArray.FromEnum(EDamageType))]
	protected ref array<EDamageType> m_aDamageOverTimeConditions;
	
	[Attribute()]
	protected ref Color m_cHasDpsBarColor;
	
	protected ref Color m_cDefaultBarColor;
	
	protected TextWidget m_TextWidget;
	protected TextWidget m_Value;
	protected SCR_WLibProgressBarComponent m_Bar;
	protected DamageManagerComponent m_DamageManager;
	
	protected bool m_bBarIsColored;
	
	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return m_DamageManager != null;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (m_aDamageOverTimeConditions.IsEmpty())
			return;
		
		bool damageOverTime = false;
		foreach (EDamageType dpsType: m_aDamageOverTimeConditions)
		{
			if (m_DamageManager.IsDamagedOverTime(dpsType))
			{
				damageOverTime = true;
				break;
			}
		}
		
		SetBarColor(damageOverTime);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetBarColor(bool SetConditionColor)
	{
		if (!m_Bar || m_bBarIsColored == SetConditionColor)
			return;
		
		m_bBarIsColored = SetConditionColor;
		
		if (m_bBarIsColored)
			m_Bar.SetSliderColor(m_cHasDpsBarColor);
		else 
			m_Bar.SetSliderColor(m_cDefaultBarColor);
	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetBarAndPercentageValue(float newValue)
	{
		if (m_Bar)
			m_Bar.SetValue(newValue);
		
		if (m_Value)
			m_Value.SetTextFormat(m_sPercentageText, Math.Round(newValue * 100));
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_DamageManager = DamageManagerComponent.Cast(entity.GetOwner().FindComponent(DamageManagerComponent));
		if (!m_DamageManager)
			return false;
		
		m_Value = TextWidget.Cast(m_Widget.FindAnyWidget("Value"));
			
		Widget barWidget = m_Widget.FindAnyWidget("ProgressBar");
		if (barWidget)
			m_Bar = SCR_WLibProgressBarComponent.Cast(barWidget.FindHandler(SCR_WLibProgressBarComponent));
		
		if (!m_Value && !m_Bar)
			return false;
		
		m_cDefaultBarColor = m_Bar.m_SliderColor;
		
		return true;
	}
}
