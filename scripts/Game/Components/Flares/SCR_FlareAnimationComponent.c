[EntityEditorProps(category: "GameScripted/Flares", description: "Base component for handling animation.")]
class SCR_FlareAnimationComponentClass : WeaponAnimationComponentClass
{
}

class SCR_FlareAnimationComponent : WeaponAnimationComponent
{
	protected SCR_CharacterControllerComponent m_UserController;

	protected AnimationEventID m_iFireFlare = -1;
	protected AnimationEventID m_iHideFlare = -1;

	protected bool m_bStartedProne;

	protected static const string CHARACTER_FIRE_INPUT_NAME = "CharacterFire";
	protected static const string ANIMATION_EVENT_FIRE = "Fire";
	protected static const string ANIMATION_EVENT_HIDE = "Weapon_Hide";

	protected static const int GARBAGE_COLLECTION_TIME = 30;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_FlareAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SCR_WeaponComponent weaponComp = SCR_WeaponComponent.Cast(ent.FindComponent(SCR_WeaponComponent));
		if (weaponComp)
			weaponComp.GetOnWeaponStateChanged().Insert(OnWeaponStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! \returns true if animations should be possible to continue, otherwise false
	protected bool CanContinueTheAnimation()
	{
		if (!m_UserController.IsWeaponRaised())
			return false;
		
		if (m_UserController.IsPartiallyLowered())
			return false;
		
		if (!m_UserController.GetCanFireWeapon())
			return false;

		bool isProne = m_UserController.GetStance() == ECharacterStance.PRONE;
		return m_bStartedProne == isProne;
	}

	//------------------------------------------------------------------------------------------------
	override protected event void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		super.OnAnimationEvent(animEventType, animUserString, intParam, timeFromStart, timeToEnd);

		if (animEventType == m_iHideFlare && Replication.IsServer())
		{
			IEntity owner = GetOwner();
			SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(owner);
			if (garbageSystem)
				garbageSystem.Insert(owner, GARBAGE_COLLECTION_TIME);
		}

		if (!m_UserController)
			return;

		if (!CanContinueTheAnimation())
		{
 			CharacterAnimationComponent animationComponent = m_UserController.GetAnimationComponent();
			if (!animationComponent)
				return;

			CharacterCommandHandlerComponent commandHandler = animationComponent.GetCommandHandler();
			if (commandHandler)
				commandHandler.CancelItemUse();

			return;
		}

		if (animEventType != m_iHideFlare && animEventType != m_iFireFlare)
			return;

		CharacterInputContext inputContext = m_UserController.GetInputContext();
		if (!inputContext)
			return;

		if (animEventType == m_iFireFlare)
		{
			inputContext.SetWeaponPullTrigger(true);
			LockItem();
			// Observe the player in case he does something that would prevent the animation from reaching the point at which we discard this item
			// As in such case it could cause problems due to the fact that the item is now locked
			m_UserController.m_OnItemUseEndedInvoker.Insert(OnItemUseEnded);
			return;
		}

		if (animEventType != m_iHideFlare)
			return;

		inputContext.SetWeaponPullTrigger(false);
		DisposeOfFlare();
		OnWeaponStateChanged(null, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to start flare firing procedure
	protected void FireFlare()
	{
		if (!m_UserController)
			return;

		if (m_UserController.GetCharacter() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		if (m_UserController.IsUsingItem())
			return;

		m_UserController.SetPartialLower(false);
		m_bStartedProne = m_UserController.GetStance() == ECharacterStance.PRONE;
		if (!CanContinueTheAnimation())
			return;

		m_UserController.TryUseItem(GetOwner(), true, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method intended to capture interruption of the item usage
	//! \param[in] item
	//! \param[in] successful
	//! \param[in] animParams
	protected void OnItemUseEnded(IEntity item, bool successful, ItemUseParameters animParams)
	{
		IEntity owner = GetOwner();
		if (item != owner)
			return;

		WeaponComponent wpnComp = WeaponComponent.Cast(owner.FindComponent(WeaponComponent));
		if (!wpnComp)
			return;

		BaseMagazineComponent currentMag = wpnComp.GetCurrentMagazine();
		if (currentMag)
			return;

		LockItem();
		DisposeOfFlare();
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method called when weapon which is the owner of this component, is being used or stops being used
	//! \param[in] weapon
	//! \param[in] active
	protected void OnWeaponStateChanged(WeaponComponent weapon, bool active)
	{
		if (!active)
		{
			GetGame().GetInputManager().RemoveActionListener(CHARACTER_FIRE_INPUT_NAME, EActionTrigger.DOWN, FireFlare);
			m_iFireFlare = -1;
			m_iHideFlare = -1;
			if (!m_UserController)
				return;

			m_UserController.m_OnItemUseEndedInvoker.Remove(OnItemUseEnded);
			if (weapon)
			{
				BaseMagazineComponent currentMag = weapon.GetCurrentMagazine();
				if (!currentMag) // If player was somehow able to equip used flare, then we should make sure that he will get rid of it
					DisposeOfFlare();
			}

			m_UserController = null;
			return;
		}

		m_UserController = null;
		ChimeraCharacter character;
		IEntity parent = GetOwner().GetParent();
		while (parent)
		{
			character = SCR_ChimeraCharacter.Cast(parent);
			if (character)
				break;

			parent = parent.GetParent();
		}

		if (!character)
			return;

		m_iHideFlare = GameAnimationUtils.RegisterAnimationEvent(ANIMATION_EVENT_HIDE);

		bool isAi;
		if (SCR_CharacterHelper.IsPlayerOrAIOwner(character, isAi))
			m_UserController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());

		if (!m_UserController)
			return;

		BaseMagazineComponent currentMag = weapon.GetCurrentMagazine();
		if (!currentMag)
		{
			LockItem();
			DisposeOfFlare();
			return;
		}

		m_iFireFlare = GameAnimationUtils.RegisterAnimationEvent(ANIMATION_EVENT_FIRE);

		if (!isAi)
			GetGame().GetInputManager().AddActionListener(CHARACTER_FIRE_INPUT_NAME, EActionTrigger.DOWN, FireFlare);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to ensure that players wont be able to interact with this item
	protected void LockItem()
	{
		SCR_WeaponAttachmentsStorageComponent weaponStorage = SCR_WeaponAttachmentsStorageComponent.Cast(GetOwner().FindComponent(SCR_WeaponAttachmentsStorageComponent));
		if (!weaponStorage)
			return;

		weaponStorage.RequestUserLock(m_UserController.GetCharacter(), true);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to drop the item on the ground
	protected void DisposeOfFlare()
	{
		if (!m_UserController)
			return;

		SCR_ChimeraCharacter character = m_UserController.GetCharacter();
		if (!character)
			return;

		CharacterWeaponManagerComponent weaponManager = CharacterWeaponManagerComponent.Cast(character.GetWeaponManager());
		if (!weaponManager)
			return;

		WeaponSlotComponent currentSlot = weaponManager.GetCurrentSlot();
		if (!currentSlot)
			return;

		GetGame().GetInputManager().RemoveActionListener(CHARACTER_FIRE_INPUT_NAME, EActionTrigger.DOWN, FireFlare);
		m_UserController.m_OnItemUseEndedInvoker.Remove(OnItemUseEnded);
		m_UserController.DropWeapon(currentSlot);

		CharacterAnimationComponent animationComponent = m_UserController.GetAnimationComponent();
		if (!animationComponent)
			return;

		CharacterCommandHandlerComponent commandHandler = animationComponent.GetCommandHandler();
		if (commandHandler)
			commandHandler.FinishItemUse(true);
	}
}
