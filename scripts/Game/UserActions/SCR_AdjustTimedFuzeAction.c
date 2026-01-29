class SCR_AdjustTimedFuzeAction : SCR_AdjustSignalAction
{
	protected SCR_ExplosiveChargeComponent m_ChargeComp;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		m_ChargeComp = SCR_ExplosiveChargeComponent.Cast(pOwnerEntity.FindComponent(SCR_ExplosiveChargeComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_ChargeComp || m_ChargeComp.GetUsedFuzeType() != SCR_EFuzeType.NONE)
			return false;

		return super.CanBeShownScript(user);
	}

	override bool GetActionNameScript(out string outName)
	{
		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;

		outName = WidgetManager.Translate(actionInfo.GetName(), m_ChargeComp.GetFuzeTime());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		if (GetMaximumValue() - GetMinimumValue() != 0)
			return (m_fTargetValue - GetMinimumValue()) / (GetMaximumValue() - GetMinimumValue());

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		if (float.AlmostEqual(m_fTargetValue, m_ChargeComp.GetFuzeTime()))
			return false;

		return super.OnSaveActionData(writer);
	}

	//------------------------------------------------------------------------------------------------
	//! If the one performing the action packed some data in it everybody receiving the action.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	//! Only triggered if the sender wrote anyting to the buffer.
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		if (!super.OnLoadActionData(reader))
			return false;

		if (m_ChargeComp)
			m_ChargeComp.SetFuzeTime(m_fTargetValue);

		return true;
	}
}
