class SCR_DisarmMineAction : SCR_BaseMineAction
{
	protected static const float COMMAND_INT_ARGUMENT = 0;
	protected static const string ANIMATION_BINDING_COMMAND = "CMD_Item_Action";

	[Attribute(defvalue: "1", desc: "Distance in meters from which the player will be able to see this user action. [m]\nThis distance also hides the context blip when the player uses freelook, until they are within range.\nDistance is measured from both the position of the feet and the position of the eyes, and the closer one is used to determine if the condition is met.\nWhen value is set to 0 then it means that distance checks are skipped, and visibility distance is dictated by value from 'Visibility Range'.", params: "0 inf 0.01")]
	protected float m_fSpottingDistance;

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

		vector matUser[4], mat[4];
		pUserEntity.GetTransform(matUser);
		pOwnerEntity.GetTransform(mat);

		SCR_EntityHelper.SnapToGround(pOwnerEntity, {pUserEntity}, startOffset: mat[1] * 0.1, onlyStatic: true);

		OrientToForward(matUser[2], mat);
		pOwnerEntity.SetTransform(mat);

		m_MineTriggereComp.SetUser(pUserEntity);

		CharacterAnimationComponent pAnimationComponent = controller.GetAnimationComponent();
		int itemActionId = pAnimationComponent.BindCommand(ANIMATION_BINDING_COMMAND);

		matUser[3] = pOwnerEntity.GetOrigin();
		PointInfo ptWS = new PointInfo();
		ptWS.Set(null, string.Empty, matUser);

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(pOwnerEntity);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(true);
		params.SetIsMainUserOfTheItem(true);
		params.SetKeepGadgetVisible(true);
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

		if (!m_MineTriggereComp.IsActivated())
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		if (m_fSpottingDistance <= 0)
			return true;

		//distance check here to prevent showing of the interaction blip when using freelook from further distances
		vector position = GetOwner().GetOrigin();
		if (vector.Distance(position, character.EyePosition()) < m_fSpottingDistance)
			return true;

		return vector.Distance(position, character.GetOrigin()) < m_fSpottingDistance;
	}
}
