[EntityEditorProps(category: "GameScripted/Components", description: "Mine inventory item component.")]
class SCR_DeployableInventoryItemInventoryComponentClass : SCR_PlaceableInventoryItemComponentClass
{
	[Attribute(desc: "Additional offset to the position at which entity will be dropped from inventory.\nGame will use position of the user that dropped this item on the groun and add to it this offset.\nWhen both rotation and position offsets are set to zero. then default placement mechanism will be used, unless forced by m_bForceItemDropOnCharacterPosition.", category: "Setup")]
	protected vector m_vAdditionalDropOffset;

	[Attribute(desc: "Additional rotation to the orientation in which entity end up after it is dropped from the inventory.\nGame will use position of the user that dropped this item on the groun and add to it this offset.\nWhen both rotation and position offsets are set to zero. then default placement mechanism will be used, unless forced by m_bForceItemDropOnCharacterPosition.", category: "Setup")]
	protected vector m_vAdditionalDropRotation;

	[Attribute(desc: "Ensures that even when item offsets are not defined, then game will place dropped item on the position of the character that dropped it.", category: "Setup")]
	protected bool m_bForceItemDropOnCharacterPosition;

	//------------------------------------------------------------------------------------------------
	vector GetAdditionalDropOffset()
	{
		return m_vAdditionalDropOffset;
	}

	//------------------------------------------------------------------------------------------------
	vector GetAdditionalDropRotation()
	{
		return m_vAdditionalDropRotation;
	}

	//------------------------------------------------------------------------------------------------
	bool IsItemForceDroppedOnCharacterPosition()
	{
		return m_bForceItemDropOnCharacterPosition;
	}
}

class SCR_DeployableInventoryItemInventoryComponent : SCR_PlaceableInventoryItemComponent
{

	//------------------------------------------------------------------------------------------------
	override bool ShouldHideInVicinity()
	{
		SCR_BaseDeployableInventoryItemComponent deployableItemComponent = SCR_BaseDeployableInventoryItemComponent.Cast(GetOwner().FindComponent(SCR_BaseDeployableInventoryItemComponent));
		if (!deployableItemComponent)
			return false;
		
		return deployableItemComponent.IsDeployed();
	}

	//------------------------------------------------------------------------------------------------
	override bool OverridePlacementTransform(IEntity caller, out vector computedTransform[4])
	{
		if (m_bUseTransform)
			return super.OverridePlacementTransform(caller, computedTransform);

		IEntity owner = GetOwner();
		if (!owner || owner.IsDeleted())
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(caller);
		if (!character)
			return false;

		SCR_DeployableInventoryItemInventoryComponentClass data = SCR_DeployableInventoryItemInventoryComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return false;

		if (!data.IsItemForceDroppedOnCharacterPosition() && data.GetAdditionalDropOffset() == vector.Zero && data.GetAdditionalDropRotation() == vector.Zero)
			return false;

		TraceParam param = new TraceParam();
		array<IEntity> exclude = {owner, caller};
		param.ExcludeArray = exclude;

		//figure out where we can place this
		SCR_TerrainHelper.GetTerrainBasis(character.AimingPosition(), computedTransform, caller.GetWorld(), false, param);

		//rotate to where character is facing while being parallel to the ground
		vector charMat[4];
		character.GetTransform(charMat);
		vector right = computedTransform[1] * charMat[0];
		vector forward = computedTransform[1] * right;
		computedTransform[0] = forward.Normalized();
		computedTransform[2] = right.Normalized();

		//apply offsets
		vector additiveMat[4];
		Math3D.AnglesToMatrix(data.GetAdditionalDropRotation(), additiveMat);
		additiveMat[3] = data.GetAdditionalDropOffset();
		Math3D.MatrixMultiply4(computedTransform, additiveMat, computedTransform);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void PlayPlacedSound(vector up, vector position, string overrideSoundEvent = SCR_SoundEvent.SOUND_PLACE_OBJECT)
	{
		if (overrideSoundEvent == SCR_SoundEvent.SOUND_PLACE_OBJECT)
			overrideSoundEvent = SCR_SoundEvent.SOUND_DROP;

		super.PlayPlacedSound(up, position, overrideSoundEvent)
	}
}
