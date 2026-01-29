class SCR_ExplosiveChargeAction : SCR_ScriptedUserAction
{
	[Attribute("CMD_Item_Action", desc: "Name of the command that will be used to bind usage animation")]
	protected string m_sUseItemCommandName;

	[Attribute("#AR-UserAction_ExplosiveCharge_ArmedWithDifferentFuze", desc: "Localized string of the text that will be shown when charge is armed with different fuze")]
	protected LocalizedString m_sDifferentFuzeInUseText;

	protected SCR_ExplosiveChargeComponent m_ChargeComp;
	protected ChimeraCharacter m_ActionUser;

	protected const float MAX_ALLOWED_ANIMATION_DISTANCE = 0.3;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_ChargeComp = SCR_ExplosiveChargeComponent.Cast(pOwnerEntity.FindComponent(SCR_ExplosiveChargeComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_ChargeComp)
			return false;

		if (!m_ChargeComp.GetTrigger())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_ChargeComp.GetRplId().IsValid())
			return false;

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
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_ActionUser = ChimeraCharacter.Cast(pUserEntity);
		if (!m_ActionUser)
			return;

		if (m_sUseItemCommandName != string.Empty)
			AnimateUsage(pUserEntity);
		else
			ProcesFinished(GetOwner(), true, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void AnimateUsage(IEntity pUserEntity)
	{
		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(m_ActionUser.GetCharacterController());
		if (!charController)
			return;

		CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
		int itemActionId = pAnimationComponent.BindCommand(m_sUseItemCommandName);
		vector charWorldMat[4];
		pUserEntity.GetWorldTransform(charWorldMat);
		float lerpFactor = vector.DistanceXZ(charWorldMat[3], GetOwner().GetOrigin());
		lerpFactor = Math.Clamp((lerpFactor - MAX_ALLOWED_ANIMATION_DISTANCE) / lerpFactor, 0.01, 1);
		charWorldMat[3][0] = Math.Lerp(charWorldMat[3][0], GetOwner().GetOrigin()[0], lerpFactor);
		charWorldMat[3][2] = Math.Lerp(charWorldMat[3][2], GetOwner().GetOrigin()[2], lerpFactor);
		PointInfo ptWS = new PointInfo();
		ptWS.Set(null, "", charWorldMat);

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(GetOwner());
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(true);
		params.SetCommandID(itemActionId);
		params.SetCommandIntArg(3);
		params.SetCommandFloatArg(0.0);
		params.SetAlignmentPoint(ptWS);

		charController.TryUseItemOverrideParams(params);
		charController.m_OnItemUseEndedInvoker.Insert(ProcesFinished);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by everyone when process was finished
	void ProcesFinished(IEntity item, bool successful, ItemUseParameters animParams)
	{
		if (!m_ActionUser)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(m_ActionUser.GetCharacterController());
		if (!characterController)
			return;

		characterController.m_OnItemUseEndedInvoker.Remove(ProcesFinished);
	}
}
