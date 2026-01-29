[ComponentEditorProps(category: "GameScripted/Inventory", description: "")]
class SCR_HandSlotStorageComponentClass : SCR_UniversalInventoryStorageComponentClass
{
}

class SCR_HandSlotStorageComponent : SCR_UniversalInventoryStorageComponent
{
	protected bool m_bSkipNextWaitForAnimation;
	protected IEntity m_HandSlotItem;
	protected RplComponent m_RplComponent;
	protected AnimationEventID m_iGestureAnimationEndEvent = -1;
	protected SCR_CharacterInventoryStorageComponent m_CharacterInventory;

	protected static const string ANIM_EVENT_NAME_FIRE = "Character_RefreshIK";
	protected static const string VAR_CAN_BE_HELD_NAME = "m_bCanBeHeld";
	
	protected override InventoryStorageSlot GetEmptySlotForItem(IEntity item)
	{
		SCR_HandSlotStorageSlot slot;
		for (int i, count = GetSlotsCount(); i < count; i++)
		{
			slot = SCR_HandSlotStorageSlot.Cast(GetSlot(i));
			if (!slot)
				continue;

			if (slot.GetAttachedEntity())
				continue;

			return slot;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanStoreItem(IEntity item, int slotID)
	{
		SCR_GadgetComponent gadget = SCR_GadgetComponent.Cast(item.FindComponent(SCR_GadgetComponent));
		if (!gadget || !gadget.CanBeHeld())
			return false;

		if (m_CharacterInventory && !m_CharacterInventory.CanUseItem(item, ESlotFunction.TYPE_GADGET))
			return false;

		return GetEmptySlotForItem(item);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanStoreResource(ResourceName resourceName, int slotID)
	{
		Resource entityResource = Resource.Load(resourceName);
		if (!entityResource.IsValid())
			return false;

		BaseResourceObject resourceObj = entityResource.GetResource();
		if (!resourceObj)
			return false;

		IEntitySource entitySrc = resourceObj.ToEntitySource();
		if (!entitySrc)
			return false;

		IEntityComponentSource componentSrc;
		for (int i, componentsCount = entitySrc.GetComponentCount(); i < componentsCount; i++)
		{
			componentSrc = entitySrc.GetComponent(i);
			if (!componentSrc)
				continue;

			if (!componentSrc.GetClassName().ToType().IsInherited(SCR_GadgetComponent))
				continue;

			bool canBeHeld;
			return componentSrc.Get(VAR_CAN_BE_HELD_NAME, canBeHeld) && canBeHeld && GetEmptySlotForItem(null);
		}
	
		return false;
	}

	//------------------------------------------------------------------------------------------------
 	override bool CanReplaceItem(IEntity nextItem, int slotID)
	{
		if (!nextItem.FindComponent(SCR_GadgetComponent))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetHandSlotItem()
	{
		return m_HandSlotItem;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnAddedToSlot(IEntity item, int slotID)
	{
		super.OnAddedToSlot(item, slotID);

		InventoryItemComponent iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (iic)
			iic.HideOwner();

		if (!m_RplComponent)
			m_RplComponent = SCR_EntityHelper.GetEntityRplComponent(GetOwner());

		if (m_RplComponent && !m_RplComponent.IsOwner())
			return;

		m_HandSlotItem = item;
		if (m_bSkipNextWaitForAnimation)
		{
			SkipAnimation();
			return;
		}

		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character || !SCR_CharacterHelper.IsPlayerOrAIOwner(character))
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;

		m_iGestureAnimationEndEvent = GameAnimationUtils.RegisterAnimationEvent(ANIM_EVENT_NAME_FIRE);
		if (m_iGestureAnimationEndEvent >= 0)
		{
			controller.GetOnAnimationEvent().Remove(OnAnimationEvent);
			controller.GetOnAnimationEvent().Insert(OnAnimationEvent);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DelayedOnAddedToSlot(IEntity item)
	{
		if (!m_CharacterInventory)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
			if (!character)
				return;

			CharacterControllerComponent controller = character.GetCharacterController();
			if (!controller)
				return;

			SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
			if (!storageManager)

			m_CharacterInventory = storageManager.GetCharacterStorage();
			if (!m_CharacterInventory)
				return;
		}

		if (!item)
		{
			item = Get(0);//there will always be only one handslot and if for some reason item is null then check if we actually dont have an entity in that slot
			m_HandSlotItem = item;
			if (!item)
				return;
		}

		InventoryItemComponent iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (iic)
			iic.ShowOwner();

		//~ Equipt item
		m_CharacterInventory.UseItem(item, ESlotFunction.TYPE_GADGET);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority method used to inform others that this entity should skip the wait for the pick up animation
	void SkipAnimation_S()
	{
		Rpc(RPC_DoSkipAnimation);

		SkipAnimation();
	}

	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RPC_DoSkipAnimation()
	{
		m_bSkipNextWaitForAnimation = true;
	}

	//------------------------------------------------------------------------------------------------
	//! When animation is not used then this will remove itself from animation event script invoker and continue the proces of handling hand slot item
	void SkipAnimation()
	{
		m_bSkipNextWaitForAnimation = false;
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;

		controller.GetOnAnimationEvent().Remove(OnAnimationEvent);
		DelayedOnAddedToSlot(m_HandSlotItem);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback called when animation event happens.
	//! \param[in] animEventType ID of animation event.
	//! \param[in] animUserString String parameter of animation event
	//! \param[in] intParam int parameter of animation event
	//! \param[in] timeFromStart time from Start of animation event
	//! \param[in] timeToEnd time to end of animation event
	protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		if (animEventType != m_iGestureAnimationEndEvent)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;

		controller.GetOnAnimationEvent().Remove(OnAnimationEvent);
		DelayedOnAddedToSlot(m_HandSlotItem);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnRemovedFromSlot(IEntity item, int slotID)
	{
		m_HandSlotItem = null;
		super.OnRemovedFromSlot(item, slotID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnManagerChanged(InventoryStorageManagerComponent manager)
	{
		super.OnManagerChanged(manager);

		SCR_InventoryStorageManagerComponent scrMgr = SCR_InventoryStorageManagerComponent.Cast(manager);
		if (scrMgr)
			m_CharacterInventory = scrMgr.GetCharacterStorage();
	}
}
