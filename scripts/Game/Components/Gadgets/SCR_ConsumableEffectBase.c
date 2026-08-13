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
	
	[Attribute("1", UIWidgets.EditBox, "Duration of the looping part of the animation for using consumable on self\nThis has no effect on animation which dont loop any part of it.", category: "General")]
	protected float m_fApplyToSelfDuration;	
	
	[Attribute("1", UIWidgets.EditBox, "Duration of the looping part of the animation for using consumable on other.\nThis has no effect on animation which dont loop any part of it.", category: "General")]
	protected float m_fApplyToOtherDuration;

	[Attribute(defvalue: SCR_EGroupRole.NONE.ToString(), uiwidget: UIWidgets.ComboBox, enumType: SCR_EGroupRole, desc: "Group roles which determine if user recives usage speed bonus, if his group matches at least one of the roles from this list\nIf character's group role isnt in the list, then penalty value is used.\nIf left empty, then this condition is not evaluated.", category: "General")]
	protected ref array<SCR_EGroupRole> m_aGroupRoles;

	[Attribute(defvalue: EEditableEntityLabel.NONE.ToString(), uiwidget: UIWidgets.ComboBox, enumType: EEditableEntityLabel, desc: "Character labels which determine if user recives usage speed bonus, if character matches at least one of the labels from this list\nIf character's labels dont match any labels from this list, then penalty value is used.\nIf left empty, then this condition is not evaluated.", category: "General")]
	protected ref array<EEditableEntityLabel> m_aCharacterLabels;

	[Attribute(defvalue: "0", desc: "Bonus usage speed which is applied if character has at least one matching label, or his group role matches.\nIt is used as a divider of the value from ApplyTo<target>Duration.\n0 means that there is no bonus, and default speed will be used.", params: "0 inf 0.01", category: "General")]
	protected float m_fRoleSpeedBonus;

	[Attribute(defvalue: "0", desc: "Penalty to the usage speed for character not having at least one label, nor his group matching the role.\nIt is used as a divider of the value from ApplyTo<target>Duration.\n0 means that there is no penalty, and default speed will be used.", params: "0 inf 0.01", category: "General")]
	protected float m_fRoleSpeedPenalty;

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
	//! \return character who is using provided gadget
	protected SCR_ChimeraCharacter GetItemUser(notnull IEntity item)
	{
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(item.FindComponent(SCR_GadgetComponent));
		if (!gadgetComp)
			return null;

		return SCR_ChimeraCharacter.Cast(gadgetComp.GetCharacterOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! Provides usage animation speed modifier based on character's labels and group role. Which is meant to be a divider of time used looping part of item usage animation
	//! \param[in] character for which data will be evaluated
	//! \return factor where value is always larger than 0, with default value being 1
	float GetUsageSpeedFactor(notnull SCR_ChimeraCharacter character)
	{
		bool meetsBonusRequirement = (!m_aGroupRoles.IsEmpty() && character.HasRole(m_aGroupRoles)) || (!m_aCharacterLabels.IsEmpty() && character.HasLabel(m_aCharacterLabels));
		if (meetsBonusRequirement)
		{
			if (m_fRoleSpeedBonus > 0)
				return m_fRoleSpeedBonus;
		}
		else if (m_fRoleSpeedPenalty > 0)
		{
			return m_fRoleSpeedPenalty;
		}

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_EGroupRole GetGroupRole(notnull SCR_ChimeraCharacter character)
	{
		SCR_AIGroup group = character.GetCharacterGroup();
		if (!group)
			return SCR_EGroupRole.NONE;

		return group.GetGroupRole();
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
