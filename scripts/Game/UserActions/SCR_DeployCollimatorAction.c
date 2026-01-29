class SCR_DeployCollimatorAction : SCR_AdjustCollimatorAction
{
	[Attribute(desc: "When true, this sight is stowed by default")]
	protected bool m_bSightStowedOnStart;
	
	protected bool m_bSightIsUnstowed;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_bSightIsUnstowed = !m_bSightStowedOnStart;

		super.Init(pOwnerEntity, pManagerComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		bool saved = super.OnSaveActionData(writer);
		if (!m_SightsComponent)
			return saved;
		
		m_bSightIsUnstowed = Math.Round(m_fTargetValue);
		SCR_HelicopterCollimatorSightComponent helicopterCollimator = SCR_HelicopterCollimatorSightComponent.Cast(m_SightsComponent);
		if (helicopterCollimator)
			helicopterCollimator.OverrideSightState(m_bSightIsUnstowed);

		return saved;
	}

	//------------------------------------------------------------------------------------------------
	//! If the one performing the action packed some data in it everybody receiving the action.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	//! Only triggered if the sender wrote anyting to the buffer.
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		bool loaded = super.OnLoadActionData(reader);
		if (m_bIsAdjustedByPlayer)
			return loaded;

		m_bSightIsUnstowed = Math.Round(m_fTargetValue);
		SCR_HelicopterCollimatorSightComponent helicopterCollimator = SCR_HelicopterCollimatorSightComponent.Cast(m_SightsComponent);
		if (helicopterCollimator)
			helicopterCollimator.OverrideSightState(m_bSightIsUnstowed);
		
		return loaded;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;

		if (Math.Round(GetCurrentValue()) == 0)
			outName = actionInfo.GetName();
		else
			outName = actionInfo.GetDescription();
		
		return true;
	}
}
