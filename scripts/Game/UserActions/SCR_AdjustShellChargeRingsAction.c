class SCR_AdjustShellChargeRingsAction : SCR_AdjustSignalAction
{
	protected SCR_MortarShellGadgetComponent m_ShellGadgetComp;
	protected ChimeraCharacter m_Character;
	protected SCR_ShellConfig m_ShellConfig;
	protected int m_iDefaultConfigId;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_ShellGadgetComp = SCR_MortarShellGadgetComponent.Cast(pOwnerEntity.FindComponent(SCR_MortarShellGadgetComponent));
		if (m_ShellGadgetComp)
		{
			SCR_MortarShellGadgetComponentClass data = SCR_MortarShellGadgetComponentClass.Cast(m_ShellGadgetComp.GetComponentData(pOwnerEntity));
			m_iDefaultConfigId = data.FindDefaultChargeRingConfig();
		}

		super.Init(pOwnerEntity, pManagerComponent);
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

		if (!m_ShellGadgetComp || m_ShellGadgetComp.GetNumberOfChargeRingConfigurations() < 2)
			return false;

		if (!m_ShellConfig)
			m_ShellConfig = m_ShellGadgetComp.GetSavedConfig(character);

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

		if (!m_ShellConfig || m_ShellConfig.GetChargeRingConfigId() == -1 || m_ShellConfig.GetChargeRingConfigId() != m_ShellGadgetComp.GetCurentChargeRingConfigId() || m_ShellConfig.GetChargeRingConfigId() == m_iDefaultConfigId)
			outName = WidgetManager.Translate(actionInfo.GetName(), m_ShellGadgetComp.GetCurentChargeRingConfig()[0].ToString(0, 0));
		else 
			outName = WidgetManager.Translate(actionInfo.GetDescription(), m_ShellGadgetComp.GetChargeRingConfig(m_ShellConfig.GetChargeRingConfigId())[0].ToString(0, 0));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		m_Character = ChimeraCharacter.Cast(pUserEntity);

		if (m_ShellGadgetComp)
			m_fTargetValue = m_ShellGadgetComp.GetCurentChargeRingConfigId();

		super.OnActionStart(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected override float SCR_GetCurrentValue()
	{
		if (m_ShellGadgetComp)
			return m_ShellGadgetComp.GetCurentChargeRingConfigId();

		return GetCurrentValue();
	}

	//------------------------------------------------------------------------------------------------
	protected override float SCR_GetMinimumValue()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected override float SCR_GetMaximumValue()
	{
		if (m_ShellGadgetComp)
			return m_ShellGadgetComp.GetNumberOfChargeRingConfigurations() - 1;

		return GetMaximumValue();
	}

	//------------------------------------------------------------------------------------------------
	protected override bool OnSaveActionData(ScriptBitWriter writer)
	{
		if (m_fTargetValue == m_ShellGadgetComp.GetCurentChargeRingConfigId())
			return false;

		if (!super.OnSaveActionData(writer))
			return false;

		if (m_ShellGadgetComp)
		{
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(m_Character.GetCharacterController());
			if (!controller)
				return true;

			EntityPrefabData shellPrefab = GetOwner().GetPrefabData();
			if (!shellPrefab)
				return true;

			int savedValue = m_fTargetValue;
			if (savedValue < 0 || savedValue > SCR_GetMaximumValue() || savedValue == m_iDefaultConfigId)
				savedValue = -1;

			SCR_ShellConfig savedConfig = controller.GetSavedShellConfig(shellPrefab);
			if (!savedConfig)
				savedConfig = new SCR_ShellConfig(shellPrefab, 0, false, savedValue);
			else
				savedConfig.SetChargeRingConfigId(savedValue);

			controller.SetShellConfig(savedConfig);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool OnLoadActionData(ScriptBitReader reader)
	{
		if (!super.OnLoadActionData(reader))
			return false;

		if (m_ShellGadgetComp)
			m_ShellGadgetComp.SetChargeRingConfig(m_fTargetValue, false, false);

		return true;
	}
}
