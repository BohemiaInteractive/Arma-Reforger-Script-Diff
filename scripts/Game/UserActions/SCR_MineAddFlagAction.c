//------------------------------------------------------------------------------------------------
class SCR_MineAddFlagAction : ScriptedUserAction
{
	[Attribute("0 0 0.19")]
	protected vector m_vFlagOffset;

	//------------------------------------------------------------------------------------------------
	protected IEntity FindFlagInInventory(InventoryStorageManagerComponent storageManager)
	{
		return storageManager.FindItemWithComponents({SCR_MineFlagComponent}, EStoragePurpose.PURPOSE_ANY);
	}

	//------------------------------------------------------------------------------------------------
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		RplComponent userRplComponent = SCR_EntityHelper.GetEntityRplComponent(pUserEntity);
		if (!userRplComponent)
			return;

		RplComponent ownerRplComponent = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
		if (!ownerRplComponent)
			return;

		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return;

		IEntity flag = FindFlagInInventory(storageManager);
		if (!flag)
			return;

		SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(flag.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (!itemComponent)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		RplComponent characterRplComp = character.GetRplComponent();
		if (!characterRplComp)
			return;

		RplComponent rplComponent = RplComponent.Cast(flag.FindComponent(RplComponent));
		if (rplComponent && !rplComponent.IsProxy())
		{
			vector mat[4];
			pOwnerEntity.GetTransform(mat);
			mat[3] = mat[3] + m_vFlagOffset;
			itemComponent.SetPlacementPosition(mat[0], mat[1], mat[2], mat[3], characterRplComp.Id());
		}

		SCR_SoundManagerModule.CreateAndPlayAudioSource(flag, SCR_SoundEvent.SOUND_MINEFLAG_PLACE);

		if (userRplComponent.IsOwner())
			storageManager.TryRemoveItemFromStorage(flag, itemComponent.GetParentSlot().GetStorage());

		if (ownerRplComponent.IsOwner())
		{
			SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(pOwnerEntity.FindComponent(SCR_MineWeaponComponent));
			if (mine)
				mine.SetFlag(flag);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(GetOwner().FindComponent(SCR_MineWeaponComponent));
		if (!mine || mine.IsFlagged())
			return false;

		SCR_PressureTriggerComponent mineTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!mineTriggerComponent || !mineTriggerComponent.IsActivated())
			return false;

		SCR_MineAwarenessComponent mineAwarenessComponent = SCR_MineAwarenessComponent.GetLocalInstance();
		if (!mineAwarenessComponent)
			return super.CanBeShownScript(user);

		if (!mineAwarenessComponent.IsDetected(GetOwner()))
			return false;
		
		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return false;

		if (!FindFlagInInventory(storageManager))
			return false;

		return super.CanBeShownScript(user);
	}
}
