//! Type of consumable gadget
enum SCR_EConsumableType
{
	NONE,
	BANDAGE,
	HEALTH,
	TOURNIQUET,
	SALINE,
	MORPHINE,
	MED_KIT
}

//! Reason why consumable cannot be applied
enum SCR_EConsumableFailReason
{
	NONE = 0,
	UNKOWN = 1,
	IS_BLEEDING = 10,
	NOT_BLEEDING = 11,
	ALREADY_APPLIED = 12,
	DAMAGED = 13,
	UNDAMAGED = 14
}

//! Effect assigned to the consumable gadget
[BaseContainerProps()]
class SCR_ConsumableEffectBase : Managed
{
	protected TAnimGraphCommand m_iPlayerApplyToSelfCmdId = -1;
	protected TAnimGraphCommand m_iPlayerApplyToOtherCmdId = -1;
	protected TAnimGraphCommand m_iPlayerReviveCmdId = -1;
	
	[Attribute("true", UIWidgets.CheckBox, "Whether consumable should be deleted directly after completing use", category: "General")]
	protected bool m_bDeleteOnUse;
	
	[Attribute("1", UIWidgets.EditBox, "Duration of the animation for using consumable on self", category: "General")]
	protected float m_fApplyToSelfDuration;	
	
	[Attribute("1", UIWidgets.EditBox, "Duration of the animation for using consumable on other", category: "General")]
	protected float m_fApplyToOtherDuration;

	SCR_EConsumableType m_eConsumableType;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] target
	//! \param[in] user
	//! \param[in] item
	//! \param[in] animParams
	//! \return
	bool ActivateEffect(IEntity target, IEntity user, IEntity item, ItemUseParameters animParams = null)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
		
		bool activatedAction;

		if (animParams)
		{
			activatedAction = controller.TryUseItemOverrideParams(animParams);
		}
		else
			activatedAction = controller.TryUseItem(item);
		
		return activatedAction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Apply consumable effect
	//! \param[in] target is the character who is having the effect applied
	//! \param[in] user
	//! \param[in] item
	//! \param[in] animParams
	void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams);
	
	//------------------------------------------------------------------------------------------------
	//! Condition whether this effect can be applied
	//! \param[in] target is the character who is having the effect applied
	//! \param[in] user
	//! \param[in] failReason is reason why CanApplyEffect returned false
	//! \return
	bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE);
	
	//------------------------------------------------------------------------------------------------
	//! Update the animCommands for all animations related to consumable
	bool UpdateAnimationCommands(IEntity user);
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] target
	//! \param[in] group
	//! \return
	ItemUseParameters GetAnimationParameters(IEntity item, notnull IEntity target, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL)
	{
		ItemUseParameters params = ItemUseParameters();
		params.SetEntity(item);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(false);
		params.SetCommandID(GetApplyToSelfAnimCmnd(target));
		params.SetCommandIntArg(1);
		params.SetCommandFloatArg(0.0);
		params.SetMaxAnimLength(m_fApplyToSelfDuration);
		params.SetIntParam(0);
			
		return params;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetDeleteOnUse()
	{
		return m_bDeleteOnUse;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetApplyToOtherDuraction()
	{
		return m_fApplyToOtherDuration;
	}	

	//------------------------------------------------------------------------------------------------
	//! \param[in] user
	//! \return
	TAnimGraphCommand GetApplyToSelfAnimCmnd(IEntity user)
	{
		UpdateAnimationCommands(user);
		return m_iPlayerApplyToSelfCmdId;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] user
	//! \return
	TAnimGraphCommand GetApplyToOtherAnimCmnd(IEntity user)
	{
		UpdateAnimationCommands(user);
		return m_iPlayerApplyToOtherCmdId;
	}
}
