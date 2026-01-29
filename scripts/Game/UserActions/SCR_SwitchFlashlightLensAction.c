class SCR_SwitchFlashlightLensAction : ScriptedUserAction
{
	[Attribute("1", desc: "If script should go from first to last (true) or last to first (false)")]
	protected bool m_bCycleDirection;

	SCR_FlashlightComponent m_FlashlightComp;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_FlashlightComp = SCR_FlashlightComponent.Cast(pOwnerEntity.FindComponent(SCR_FlashlightComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_FlashlightComp || m_FlashlightComp.GetNumberOfAvailableLenses() < 2)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		return controller && controller.GetInspectEntity() == GetOwner();
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (!m_FlashlightComp)
			return;

		m_FlashlightComp.CycleThroughLenses(m_bCycleDirection);
	}
}
