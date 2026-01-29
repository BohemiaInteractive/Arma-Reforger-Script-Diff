class SCR_AdjustShellTimeFuzeAction : SCR_AdjustSignalAction
{
	protected SCR_MortarShellGadgetComponent m_ShellGadgetComp;
	protected ChimeraCharacter m_Character;
	protected SCR_ShellConfig m_ShellConfig;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		m_ShellGadgetComp = SCR_MortarShellGadgetComponent.Cast(pOwnerEntity.FindComponent(SCR_MortarShellGadgetComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterControllerComponent characterController = character.GetCharacterController();
		if (!characterController || characterController.GetInspectEntity() != GetOwner())
			return false;

		if (!m_ShellGadgetComp || !m_ShellGadgetComp.IsUsingTimeFuze() || m_ShellGadgetComp.IsLoaded())
			return false;

		if (!m_ShellConfig)
		{
			m_ShellConfig = m_ShellGadgetComp.GetSavedConfig(character);
			if (m_ShellConfig)
				m_fTargetValue = m_ShellConfig.GetSavedTime();
		}

		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_ShellGadgetComp)
			return false;

		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;


		if (!m_ShellConfig || !m_ShellConfig.IsUsingManualTime())
		{
			outName = actionInfo.GetName();
		}
		else
		{
			string fuzeTime = WidgetManager.Translate(UIConstants.VALUE_UNIT_SECONDS, m_ShellConfig.GetSavedTime().ToString(-1, 1));
			outName = WidgetManager.Translate(actionInfo.GetDescription(), fuzeTime);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override float SCR_GetCurrentValue()
	{
		if (m_ShellConfig)
			return m_ShellConfig.GetSavedTime();

		if (m_ShellGadgetComp)
			return m_ShellGadgetComp.GetFuzeTime();

		return super.SCR_GetCurrentValue();
	}

	//------------------------------------------------------------------------------------------------
	protected override float SCR_GetMinimumValue()
	{
		if (m_ShellGadgetComp)
			return m_ShellGadgetComp.GetMinFuzeTime() - m_fAdjustmentStep;//to allow for resetting the time fuze to automatic mode by going below min value

		return super.SCR_GetMinimumValue();
	}

	//------------------------------------------------------------------------------------------------
	protected override float SCR_GetMaximumValue()
	{
		if (m_ShellGadgetComp)
			return m_ShellGadgetComp.GetMaxFuzeTime();

		return super.SCR_GetMaximumValue();
	}

	//------------------------------------------------------------------------------------------------
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(m_ShellGadgetComp.GetComponentData(m_ShellGadgetComp.GetOwner()));
		if (data && data.IsUsingTimeFuze() && data.GetMaxFuzeTime() > data.GetMinFuzeTime())
			return (m_fTargetValue - data.GetMinFuzeTime()) / (data.GetMaxFuzeTime() - data.GetMinFuzeTime());

		return 0;
	}

	override void OnActionStart(IEntity pUserEntity)
	{
		m_Character = ChimeraCharacter.Cast(pUserEntity);

		super.OnActionStart(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		if (float.AlmostEqual(m_fTargetValue, m_ShellGadgetComp.GetFuzeTime()))
			return false;

		if (!m_ShellGadgetComp)
			return false;

		m_ShellGadgetComp.SetFuzeTime(m_fTargetValue);

		if (!super.OnSaveActionData(writer))
			return false;

		if (!m_Character)
			return true;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(m_Character.GetCharacterController());
		if (!controller)
			return true;

		EntityPrefabData shellPrefab = GetOwner().GetPrefabData();
		if (!shellPrefab)
			return true;

		bool useManualTime;
		if (m_fTargetValue >= m_ShellGadgetComp.GetMinFuzeTime())
			useManualTime = true;

		SCR_ShellConfig savedConfig = controller.GetSavedShellConfig(shellPrefab);
		if (!savedConfig)
		{
			savedConfig = new SCR_ShellConfig(shellPrefab, m_ShellGadgetComp.GetFuzeTime(), useManualTime, -1);
		}
		else
		{
			savedConfig.SetSavedTime(m_ShellGadgetComp.GetFuzeTime());
			savedConfig.SetManualTimeUsage(useManualTime);
		}

		controller.SetShellConfig(savedConfig);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! If the one performing the action packed some data in it everybody receiving the action.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	//! Only triggered if the sender wrote anyting to the buffer.
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		if (!super.OnLoadActionData(reader))
			return false;

		if (m_bIsAdjustedByPlayer)
			return true;

		if (m_ShellGadgetComp)
			m_ShellGadgetComp.SetFuzeTime(m_fTargetValue);

		return true;
	}
}
