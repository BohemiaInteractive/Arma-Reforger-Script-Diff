// Reusable flask with immediate stamina recovery plus a non-stacking regeneration boost
[BaseContainerProps()]
class SCR_ConsumableFlaskEffect : SCR_ConsumableEffectHealthItems
{
	protected static const float MINIMUM_HYDRATION_DEFICIT = 0.1;

	[Attribute("0", UIWidgets.CheckBox, "Use water stored in SCR_WaterContainerComponent", category: "Water")]
	protected bool m_bUseWater;

	[Attribute("0.1", UIWidgets.EditBox, "Water consumed after a successful use\n[Liters]", "0 inf", category: "Water")]
	protected float m_fWaterConsumptionPerUse;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ConsumableFlaskEffect()
	{
		m_eConsumableType = SCR_EConsumableType.STAMINA_KIT;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason)
	{
		failReason = SCR_EConsumableFailReason.NONE;

		if (target != user)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(target);
		if (!character)
			return false;

		CharacterControllerComponent characterController = character.GetCharacterController();
		if (!characterController || characterController.GetLifeState() != ECharacterLifeState.ALIVE)
			return false;
		
		SCR_CharacterStaminaComponent staminaComponent = SCR_CharacterStaminaComponent.Cast(character.FindComponent(SCR_CharacterStaminaComponent));

		if (!staminaComponent)
			return false;

		if (!staminaComponent.IsHydrationEnabled())
			return true;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());

		if (!damageManager)
			return false;

		float hydration = staminaComponent.GetHydration();
		float hydrationThreshold = SCR_CharacterStaminaComponent.HYDRATION_MAXIMUM - MINIMUM_HYDRATION_DEFICIT;

		if (hydration > hydrationThreshold)
		{
			failReason = SCR_EConsumableFailReason.NOT_DEHYDRATED;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanApplyItemEffect(notnull IEntity target, notnull IEntity user, notnull IEntity item, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		if (m_bUseWater)
		{
			SCR_WaterContainerComponent waterContainer = SCR_WaterContainerComponent.Cast(item.FindComponent(SCR_WaterContainerComponent));
			if (!waterContainer)
			{
				failReason = SCR_EConsumableFailReason.NOT_APPLICABLE;
				return false;
			}

			if (waterContainer.GetCurrentWater() < Math.Max(m_fWaterConsumptionPerUse, 0))
			{
				failReason = SCR_EConsumableFailReason.CONSUMABLE_EMPTY;
				return false;
			}
		}

		return CanApplyEffect(target, user, failReason);
	}

	//------------------------------------------------------------------------------------------------
	override void OnApplyEffectRejected(SCR_EConsumableFailReason failReason)
	{
		switch (failReason)
		{
			case SCR_EConsumableFailReason.CONSUMABLE_EMPTY:
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_FLASK_EMPTY);
				break;
			
			case SCR_EConsumableFailReason.NOT_DEHYDRATED:
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_HYDRATION_FULL);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		SCR_WaterContainerComponent waterContainer;
		float waterConsumption = Math.Max(m_fWaterConsumptionPerUse, 0);
	
		if (m_bUseWater)
		{
			waterContainer = SCR_WaterContainerComponent.Cast(item.FindComponent(SCR_WaterContainerComponent));
			if (!waterContainer || !waterContainer.CanRemoveWater(waterConsumption))
			{
				InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
				if (itemComponent)
					itemComponent.RequestUserLock(user, false);
	
				return;
			}
		}
	
		super.ApplyEffect(target, user, item, animParams);
	
		if (!waterContainer || waterConsumption <= 0)
			return;
	
		if (waterContainer.RemoveWater_S(waterConsumption) <= 0)
			return;
	
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
	
		int playerId = playerManager.GetPlayerIdFromControlledEntity(user);
		if (playerId <= 0)
			return;
	
		SCR_NotificationsComponent.SendToPlayer(
			playerId,
			ENotification.PLAYER_FLASK_WATER_REMAINING,
			Math.Round(waterContainer.GetCurrentWater() * 100),
			Math.Round(waterContainer.GetMaximumWater() * 100)
		);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		return CanApplyEffect(target, user, failReason);
	}

	//------------------------------------------------------------------------------------------------
	override bool UpdateAnimationCommands(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterAnimationComponent animationComponent = character.GetAnimationComponent();

		if (!animationComponent)
			return false;

		m_iPlayerApplyToSelfCmdId = animationComponent.BindCommand("CMD_Item_Action");
		m_iPlayerApplyToOtherCmdId = m_iPlayerApplyToSelfCmdId;

		if (m_iPlayerApplyToSelfCmdId < 0)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override ItemUseParameters GetAnimationParameters(IEntity item, notnull IEntity target, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL)
	{
		ItemUseParameters itemUseParams = super.GetAnimationParameters(item, target, group);
		itemUseParams.SetAllowMovementDuringAction(true);
		return itemUseParams;
	}
}
