class SCR_AdjustCollimatorElevationAction : SCR_AdjustCollimatorAction
{
	[Attribute(defvalue: "1", desc: "")]
	float m_fAngleUnit;

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);

		if (m_SightsComponent)
			m_fTargetValue = m_SightsComponent.GetVerticalAngularCorrection();
	}

	//------------------------------------------------------------------------------------------------
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		if (float.AlmostEqual(m_fTargetValue, m_SightsComponent.GetVerticalAngularCorrection()))
			return false;

		if (m_SightsComponent)
			m_SightsComponent.SetVerticalAngularCorrection(m_fTargetValue * m_fAngleUnit);

		return super.OnSaveActionData(writer);
	}

	//------------------------------------------------------------------------------------------------
	//! If the one performing the action packed some data in it everybody receiving the action.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	//! Only triggered if the sender wrote anyting to the buffer.
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		if (m_bIsAdjustedByPlayer)
			return true;

		bool loaded = super.OnLoadActionData(reader);
		if (m_SightsComponent)
			m_SightsComponent.SetVerticalAngularCorrection(m_fTargetValue * m_fAngleUnit);
		
		return loaded;
	}
}
