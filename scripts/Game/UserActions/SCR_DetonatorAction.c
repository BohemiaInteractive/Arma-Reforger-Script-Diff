class SCR_RemoteDetonatorAction : SCR_ScriptedUserAction
{
	SCR_DetonatorGadgetComponent m_DetonatorComp;
	SCR_BaseDeployableInventoryItemComponent m_DeployableInvItemComp;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_DetonatorComp = SCR_DetonatorGadgetComponent.Cast(pOwnerEntity.FindComponent(SCR_DetonatorGadgetComponent));
		m_DeployableInvItemComp = SCR_BaseDeployableInventoryItemComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseDeployableInventoryItemComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DetonatorComp)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterControllerComponent characterControllerComp = character.GetCharacterController();
		if (!characterControllerComp)
			return false;

		if (!characterControllerComp.GetInspect())
			if (!m_DeployableInvItemComp || !m_DeployableInvItemComp.IsDeployed())
				return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_DetonatorComp)
			return false;

		if (!m_DetonatorComp.GetNumberOfConnectedCharges())
		{
			UIInfo actionInfo = GetUIInfo();
			if (actionInfo)
				SetCannotPerformReason(actionInfo.GetDescription());

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;

		outName = actionInfo.GetName();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return true;
	}
}
