class SCR_ActivateMineUserAction : SCR_BaseMineAction
{
	protected static const float COMMAND_INT_ARGUMENT = 3;
	protected static const string ANIMATION_BINDING_COMMAND = "CMD_Item_Action";

	//------------------------------------------------------------------------------------------------
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		m_MineTriggereComp.SetUser(pUserEntity);

		vector matUser[4], mat[4];
		pUserEntity.GetTransform(matUser);
		pOwnerEntity.GetTransform(mat);

		SCR_EntityHelper.SnapToGround(pOwnerEntity, {pUserEntity}, startOffset: mat[1] * 0.1, onlyStatic: true);

		OrientToForward(matUser[2], mat);
		pOwnerEntity.SetTransform(mat);

		CharacterAnimationComponent pAnimationComponent = controller.GetAnimationComponent();
		int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");

		matUser[3] = pOwnerEntity.GetOrigin();
		PointInfo ptWS = new PointInfo();
		ptWS.Set(null, string.Empty, matUser);

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(pOwnerEntity);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(true);
		params.SetCommandID(itemActionId);
		params.SetCommandIntArg(COMMAND_INT_ARGUMENT);
		params.SetAlignmentPoint(ptWS);

		controller.TryUseItemOverrideParams(params);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		return !m_MineTriggereComp.IsActivated();
	}
}
