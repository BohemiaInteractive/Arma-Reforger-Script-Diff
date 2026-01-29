[EntityEditorProps(category: "GameScripted/Gadgets", description: "Consumable gadget")]
class SCR_ConsumableItemComponentClass : SCR_GadgetComponentClass
{
}

// Consumable gadget component
class SCR_ConsumableItemComponent : SCR_GadgetComponent
{
	[Attribute("", UIWidgets.Object, category: "Consumable" )]
	protected ref SCR_ConsumableEffectBase m_ConsumableEffect;

	[Attribute("0", UIWidgets.CheckBox, "Switch model on use, f.e. from packaged version", category: "Consumable")]
	protected bool m_bAlternativeModelOnAction;	
	
	[Attribute("0", UIWidgets.CheckBox, "Show the model of the item on the character after using", category: "Consumable")]
	protected bool m_bVisibleEquipped;

	protected SCR_CharacterControllerComponent m_CharController;
	
	//! Target character is the target set when it's not m_CharacterOwner
	protected IEntity m_TargetCharacter;
	
	protected int m_iStoredHealedGroup;
	
	//------------------------------------------------------------------------------------------------
	//! Apply consumable effect
	//! \param[in] target is the target entity
	//! \param[in] animParams
	//! \param[in] item
	//! \param[in] deleteItem
	void ApplyItemEffect(IEntity target, IEntity user, ItemUseParameters animParams, IEntity item, bool deleteItem = true)
	{
		if (!m_ConsumableEffect)
			return;

		m_ConsumableEffect.ApplyEffect(target, user, item, animParams);

		ModeClear(EGadgetMode.IN_HAND);
		
		if (deleteItem)
			RplComponent.DeleteRplEntity(GetOwner(), false);
	}

	//------------------------------------------------------------------------------------------------
	//! OnItemUseBegan event from SCR_CharacterControllerComponent
	protected void OnUseBegan(IEntity item, ItemUseParameters animParams)
	{
		if (m_bAlternativeModelOnAction)
			SetAlternativeModel(true);

		if (!m_CharacterOwner || !animParams)
			return;
		
		if (!m_TargetCharacter)
			SetTargetCharacter(m_CharacterOwner);

		SetHealedGroup(animParams.GetIntParam(), true);
	}

	//------------------------------------------------------------------------------------------------
	//! OnItemUseComplete event from SCR_CharacterControllerComponent
	protected void OnApplyToCharacter(IEntity item, bool successful, ItemUseParameters animParams)
	{
		ClearInvokers(animParams.GetIntParam());
		
		if (!successful)
		{
			if (animParams.GetCommandIntArg() == -1)
			{
				Print("Consumable item OnItemUseComplete event called with empty SCR_ConsumableEffectAnimationParameters", LogLevel.ERROR);
				return;
			}

			if (item && !item.IsDeleted() && m_bAlternativeModelOnAction)
				SetAlternativeModel(false);

			return;	
		}

		bool deleteItem = true;
		if (GetConsumableEffect())
			deleteItem = GetConsumableEffect().GetDeleteOnUse();
		
		IEntity target = GetTargetCharacter();
		if (!target)
			target = m_CharacterOwner;
		if (!m_CharacterOwner)
			return;

		ApplyItemEffect(target, m_CharacterOwner, animParams, item, deleteItem);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetHealedGroup(EBandagingAnimationBodyParts group, bool healed)
	{
		m_iStoredHealedGroup = group * healed;

		ChimeraCharacter targetChar = ChimeraCharacter.Cast(GetTargetCharacter());;
		if (!targetChar)
		{
			targetChar = m_CharacterOwner;

			if (!targetChar)
				return;
		}

		SCR_CharacterDamageManagerComponent targetDamageMan = SCR_CharacterDamageManagerComponent.Cast(targetChar.GetDamageManager());
		if (!targetDamageMan)
			return;

		targetDamageMan.SetGroupIsBeingHealed(targetDamageMan.FindAssociatedHitZoneGroup(group), healed);
	}
	
	//-----------------------------------------------------------------------------
	//! Switch item model
	//! \param[in] useAlternative determines whether alternative or base model is used
	void SetAlternativeModel(bool useAlternative)
	{
		InventoryItemComponent inventoryItemComp = InventoryItemComponent.Cast( GetOwner().FindComponent(InventoryItemComponent));
		if (!inventoryItemComp)
			return;

		ItemAttributeCollection attributeCollection = inventoryItemComp.GetAttributes();
		if (!attributeCollection)
			return;

		SCR_AlternativeModel additionalModels = SCR_AlternativeModel.Cast( attributeCollection.FindAttribute(SCR_AlternativeModel));
		if (!additionalModels)
			return;

		ResourceName consumableModel;
		if (useAlternative)
			consumableModel = additionalModels.GetAlternativeModel();
		else if (GetOwner())
			consumableModel = SCR_Global.GetPrefabAttributeResource(GetOwner(), "MeshObject", "Object");

		Resource resource = Resource.Load(consumableModel);
		if (!resource)
			return;

		BaseResourceObject baseResource = resource.GetResource();
		if (baseResource)
		{
			VObject asset = baseResource.ToVObject();
			if (asset)
				GetOwner().SetObject(asset, "");
		}
	}

	//-----------------------------------------------------------------------------
	// TODO: fix comment
	// Get visibility of item
	// \Sets item visible on the outside of the physical character
	override bool IsVisibleEquipped()
	{
		return m_bVisibleEquipped;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);

		if (mode == EGadgetMode.IN_HAND)
		{
			m_CharController = SCR_CharacterControllerComponent.Cast(charOwner.FindComponent(SCR_CharacterControllerComponent));
			m_CharController.m_OnItemUseFinishedInvoker.Insert(OnApplyToCharacter);
			m_CharController.m_OnItemUseBeganInvoker.Insert(OnUseBegan);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{
		if (mode == EGadgetMode.IN_HAND)
		{
			// CallLater to ensure the invokers are removed after the m_OnItemUseFinishedInvoker is called. This is ensured because m_OnItemUseFinishedInvoker is called on end of fixedFrame
			if (m_CharController)
				GetGame().GetCallqueue().CallLater(ClearInvokers, param1: -1);
		}

		super.ModeClear(mode);
	}

	//------------------------------------------------------------------------------------------------
	override void ActivateAction()
	{
		if (!m_ConsumableEffect || !m_ConsumableEffect.CanApplyEffect(m_CharacterOwner, GetOwner()))
			return;

		m_ConsumableEffect.ActivateEffect(m_CharacterOwner, m_CharacterOwner, GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearInvokers(int targetGroup = -1)
	{
		if (targetGroup == -1 && m_iStoredHealedGroup != 0)
			SetHealedGroup(m_iStoredHealedGroup, false);
		else
			SetHealedGroup(targetGroup, false);
	
		if (!m_CharController)
			return;
		
		m_CharController.m_OnItemUseFinishedInvoker.Remove(OnApplyToCharacter);
		m_CharController.m_OnItemUseBeganInvoker.Remove(OnUseBegan);
		
		m_CharController = null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		// Force clear these as we need to ensure that this item is not going to be blocking any healed groups
		// This is necessary because for proxies this item will be streamed out before ModeClear will be able to call CearInvokers
		ClearInvokers(-1);
		
		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get consumable type
	//! \return consumable type
	SCR_EConsumableType GetConsumableType()
	{
		if (m_ConsumableEffect)
			return m_ConsumableEffect.m_eConsumableType;

		return SCR_EConsumableType.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get consumable effect
	//! \return consumable effect
	SCR_ConsumableEffectBase GetConsumableEffect()
	{
		return m_ConsumableEffect;
	}
	
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.CONSUMABLE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] target
	void SetTargetCharacter(IEntity target)
	{
		m_TargetCharacter = target;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetTargetCharacter()
	{
		return m_TargetCharacter;
	}
}
