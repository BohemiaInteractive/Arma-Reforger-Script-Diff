class SCR_RippleFireIntervalAction : SCR_AdjustSignalAction
{
	protected SCR_FireModeManagerComponent fireModeManager;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		fireModeManager = SCR_FireModeManagerComponent.Cast(GetOwner().FindComponent(SCR_FireModeManagerComponent));
		
		if (pOwnerEntity.GetParent())
			m_SoundComponent = SoundComponent.Cast(pOwnerEntity.GetParent().FindComponent(SoundComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (fireModeManager && fireModeManager.GetFireMode() == EWeaponGroupFireMode.RIPPLE)
			return true;
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		bool saved = super.OnSaveActionData(writer);
		bool noChanges = float.AlmostEqual(m_fTargetValue, fireModeManager.GetRippleInterval());
		writer.WriteBool(noChanges);
		if (noChanges)
			return saved;

		writer.WriteFloat(m_fTargetValue);
		if (fireModeManager)
			fireModeManager.SetRippleInterval(m_fTargetValue);

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

		bool noChanges;
		reader.ReadBool(noChanges);
		if (noChanges)
			return loaded;

		int outVal;
		reader.ReadFloat(outVal);
		m_fTargetValue = outVal;
		if (float.AlmostEqual(m_fTargetValue, fireModeManager.GetRippleInterval()))
			return loaded;

		if (fireModeManager)
			fireModeManager.SetRippleInterval(m_fTargetValue);
		
		return loaded;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!fireModeManager)
			return false;
		
		UIInfo info = GetUIInfo();
		if (!info)
			return false;
		
		outName = info.GetName() + " " + fireModeManager.GetRippleInterval();

		return true;
	}
}
