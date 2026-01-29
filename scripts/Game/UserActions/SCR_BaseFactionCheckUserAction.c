//------------------------------------------------------------------------------------------------
class SCR_BaseFactionCheckUserAction : SCR_BaseAudioScriptedUserAction
{
	[Attribute("3", desc: "Set faction of user that can use the action, will be ignored if non are selected. Requires a 'FactionAffiliationComponent' on the Owner or parent. It will always allow interaction if user does not have 'FactionAffiliationComponent'. Default faction is the faction associated with the Initial Owner faction. Flags are checked in order. DISALLOW ON NULL is only checked when Default faction check is not set.", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EActionFactionUsage), category: "Faction Settings")]
	protected EActionFactionUsage m_eFactionUsageCheck;
	
	[Attribute("#AR-ActionInvalid_HostileFaction", desc: "Faction invalid faction for the user that tries to use the action (By default this means faction is hostile, replace if there is a diffrent meaning)")]
	protected LocalizedString m_sInvalidFactionCannotPerform;
	
	[Attribute("0", desc: "If entity itself does not have faction affiliation component then it will try to get the parents faction affiliation component if this is true")]
	protected bool m_bAllowGetFactionOfParent;
	
	protected FactionAffiliationComponent m_OwnerFactionAffiliation;
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanBePerformedScript(IEntity user)
	{
		//~ Action is invalid. Set cannnot perform reason
		if (!IsFactionValid(user))
		{
			m_sCannotPerformReason = m_sInvalidFactionCannotPerform;
			return false;
		}
		//~ Action can be performed
		else 
		{
			m_sCannotPerformReason = string.Empty;
			return true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsFactionValid(IEntity user)
	{
		//~ No flags set so ignore
		if (m_eFactionUsageCheck == 0 || !user || !m_OwnerFactionAffiliation)
			return true;		
		
		FactionAffiliationComponent userFactionAffiliation = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
		
		//~ None have a faction so both return true
		if (!userFactionAffiliation)
			return true;
		
		//~ User is faction less so allow interaction
		Faction userFaction = userFactionAffiliation.GetAffiliatedFaction();
		if (!userFaction)
			return true;
		
		Faction ownerFaction;
		
		//~ Checks current faction
		if (m_eFactionUsageCheck & (EActionFactionUsage.SAME_CURRENT_FACTION | EActionFactionUsage.FRIENDLY_CURRENT_FACTION))
		{
			//~ Only check if has current faction
			ownerFaction = m_OwnerFactionAffiliation.GetAffiliatedFaction();
			if (ownerFaction)
			{
				//~ If faction is the same
				if (SCR_Enum.HasFlag(m_eFactionUsageCheck, EActionFactionUsage.SAME_CURRENT_FACTION) && ownerFaction.GetFactionKey() == userFaction.GetFactionKey())
					return true;
			
				//~ If faction is friendly
				if (SCR_Enum.HasFlag(m_eFactionUsageCheck, EActionFactionUsage.FRIENDLY_CURRENT_FACTION) && ownerFaction.IsFactionFriendly(userFaction))
					return true;
				
				//~ Current faction exists but is not the same (or if friendly is checked and is not friendly)
				return false;
			}
		}
		
		//~ Checks default faction
		if (m_eFactionUsageCheck & (EActionFactionUsage.SAME_DEFAULT_FACTION | EActionFactionUsage.FRIENDLY_DEFAULT_FACTION))
		{
			//~ No default faction so return true
			ownerFaction = m_OwnerFactionAffiliation.GetDefaultAffiliatedFaction();
			if (!ownerFaction)
				return true;
			
			//~ If default faction is the same
			if (SCR_Enum.HasFlag(m_eFactionUsageCheck, EActionFactionUsage.SAME_DEFAULT_FACTION) && ownerFaction.GetFactionKey() == userFaction.GetFactionKey())
				return true;
			
			//~ If default faction is friendly
			if (SCR_Enum.HasFlag(m_eFactionUsageCheck, EActionFactionUsage.FRIENDLY_DEFAULT_FACTION) && ownerFaction.IsFactionFriendly(userFaction))
				return true;
				
			//~ Default faction exists but is not the same (or if friendly is checked and is not friendly)
			return false;
		}
		
		//~	Has no current faction and did not check Default faction. So check if it is allowed to be used neutral
		return !SCR_Enum.HasFlag(m_eFactionUsageCheck, EActionFactionUsage.DISALLOW_USE_ON_CURRENT_FACTION_NULL);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_OwnerFactionAffiliation = FactionAffiliationComponent.Cast(GetOwner().FindComponent(FactionAffiliationComponent));
			
		if (!m_OwnerFactionAffiliation && m_bAllowGetFactionOfParent && GetOwner().GetParent())
			m_OwnerFactionAffiliation = FactionAffiliationComponent.Cast(GetOwner().GetParent().FindComponent(FactionAffiliationComponent));
	}
};

enum EActionFactionUsage
{
	SAME_CURRENT_FACTION = 1, ///< The user with the same faction as the current faction can interact with the action
	FRIENDLY_CURRENT_FACTION = 2,  ///< The user with a faction that is a friendly faction as the current faction can interact with the action
	SAME_DEFAULT_FACTION = 4, ///< The user with the same faction as the default faction of the action can interact with the action
	FRIENDLY_DEFAULT_FACTION = 8, ///< he user with a faction that is a friendly faction to the default faction of the action can interact with the action
	DISALLOW_USE_ON_CURRENT_FACTION_NULL = 16, ///< If set it disallows the action to be used if the faction of the owner is not set. Only valid if Default faction is not checked
};
