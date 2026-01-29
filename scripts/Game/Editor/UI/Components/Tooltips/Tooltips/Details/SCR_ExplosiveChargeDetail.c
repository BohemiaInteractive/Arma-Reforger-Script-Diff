[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_ExplosiveChargeDetail : SCR_EntityTooltipDetail
{
	[Attribute("#AR-ValueUnit_Short_Seconds")]
	protected string m_sShortSecondsFormat;

	[Attribute("#AR-ValueUnit_Short_Minutes")]
	protected string m_sShortMinutesFormat;

	protected TextWidget m_wText;

	SCR_ExplosiveChargeComponent m_ExplosiveChargeComp;

	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (!m_ExplosiveChargeComp)
			return;

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity timeAndWeatherManager = world.GetTimeAndWeatherManager();
		if (!timeAndWeatherManager)
			return;

		int remainingTime;
		if (m_ExplosiveChargeComp.GetUsedFuzeType() == SCR_EFuzeType.TIMED)
			remainingTime = Math.Max((m_ExplosiveChargeComp.GetTimeOfDetonation() - timeAndWeatherManager.GetEngineTime()), 0);

		if (!m_wText)
			return;

		if (remainingTime < 120)
		{
			m_wText.SetTextFormat(m_sShortSecondsFormat, remainingTime);
		}
		else
		{
			remainingTime /= 60;
			m_wText.SetTextFormat(m_sShortMinutesFormat, remainingTime);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_wText = TextWidget.Cast(widget);

		if (!m_wText)
			return false;

		m_ExplosiveChargeComp = SCR_ExplosiveChargeComponent.Cast(entity.GetOwner().FindComponent(SCR_ExplosiveChargeComponent));
		if (!m_ExplosiveChargeComp)
			return false;

		return m_ExplosiveChargeComp.GetUsedFuzeType() == SCR_EFuzeType.TIMED;
	}
}
