class SCR_BaseMineAction : ScriptedUserAction
{
	[Attribute(defvalue: "10", desc: "How long the user action will be shown as inactive after starting arming. [s]", params: "0 inf 0.01")]
	protected float m_fActionTimeout;

	[Attribute(defvalue: "0", desc: "Can this user action be used while prone")]
	protected bool m_bCanBeUsedProne;

	[Attribute(desc: "Additional rotation that will be applied when orientating the mine")]
	protected vector m_vAdditionalRotation;

	protected InventoryItemComponent m_Item;
	protected SCR_PressureTriggerComponent m_MineTriggereComp;
	protected bool m_bBlockUserAction;

	protected const float MAX_HEIGHT_DIFFERENCE = 0.4;
	protected const string CANNOT_PERFORM_PRONE = "#AR-UserAction_CannotPerformWhileProne";

	//------------------------------------------------------------------------------------------------
	protected void OrientToForward(vector forward, inout vector mat[4])
	{
		vector angles = forward.VectorToAngles() + m_vAdditionalRotation;
		forward = angles.AnglesToVector();
		mat[0] = forward * mat[1];
		mat[2] = mat[1] * -mat[0];
	}

	//------------------------------------------------------------------------------------------------
	protected void AllowActionUsage()
	{
		m_bBlockUserAction = false;
	}

	//------------------------------------------------------------------------------------------------
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_bBlockUserAction = true;
		GetGame().GetCallqueue().CallLater(AllowActionUsage, 1000 * m_fActionTimeout);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_bBlockUserAction)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		if (!m_bCanBeUsedProne && controller.GetStance() == ECharacterStance.PRONE)
		{
			SetCannotPerformReason(CANNOT_PERFORM_PRONE);
			return false;
		}
		
		IEntity owner = GetOwner();
		if (!owner)
			return false;

		if (SCR_WorldTools.IsObjectUnderwater(owner))
			return false;

		vector mat[4];
		owner.GetTransform(mat);
		if (vector.Dot(mat[1], vector.Up) < 0.5) // Rject based on the angle of placement
			return false;

		if (Math.AbsFloat(mat[3][1] - user.GetOrigin()[1]) > MAX_HEIGHT_DIFFERENCE)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_Item = InventoryItemComponent.Cast(pOwnerEntity.FindComponent(InventoryItemComponent));
		m_MineTriggereComp = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_MineTriggereComp)
			return false;

		if (!m_Item)
			return false;

		if (m_Item.IsLocked())
			return false;

		if (m_Item.GetParentSlot())
			return false;

		return super.CanBeShownScript(user);
	}
}
