[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "DO NOT USE BASE CLASS", "(Disabled) DO NOT USE BASE CLASS", 1)]
class SCR_BaseArsenalRefundEffect
{		
	[Attribute("1", desc: "If entry is disabled it will be removed on init")]
	bool m_bEnabled;
	
	[Attribute("0", desc: "Will not have any effect if within the list of: SCR_MultiExecuteListArsenalRefundEffect or SCR_WeightedListArsenalRefundEffect.\n\nThe entries with the highest priority will be checked first if they are valid. Equal priority will be checked in order of the array")]
	int m_iPriority;
	
	[Attribute(SCR_EArsenalTypes.STATIC_ENTITIES.ToString(), desc: "Which arsenal types can the item be handed in at for this effect to trigger", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalTypes))]
	protected SCR_EArsenalTypes m_eHandInArsenalTypes;
	
	//------------------------------------------------------------------------------------------------
	//! Init of effect before adding them to the order array in SCR_ArsenalRefundEffectComponentClass
	//! \param[in] arsenalRefundEffectClass Class it is attached to
	void Init(SCR_ArsenalRefundEffectComponentClass arsenalRefundEffectClass);
	
	//------------------------------------------------------------------------------------------------
	//! Returns if effect is valid always called before ExecuteRefundEffect_S
	//! \param[in] item Item that is being refunded
	//! \param[in] playerController Player controller of player that refunds the item
	//! \param[in] arsenal Arsenal the item was refunded at (Can potentially be null)
	//! \return If the refund effect is valid
	bool IsValid(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		//! Check if handed in at arsenal and if the arsenal type is supported
		return (arsenal && SCR_Enum.HasFlag(m_eHandInArsenalTypes, arsenal.GetArsenalType()));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Execute refund effect (Called on server only)
	//! \param[in] item Item that is being refunded
	//! \param[in] playerController Player controller of player that refunds the item
	//! \param[in] arsenal Arsenal the item was refunded at (Can potentially be null)
	//! \return True if effect was succesfully executed
	bool ExecuteRefundEffect(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal);
	
	//------------------------------------------------------------------------------------------------
	//! Check if item was refunded at a friendly arsenal
	//! \param[in] item Item that is being refunded
	//! \param[in] playerController Player controller of player that refunds the item
	//! \param[in] arsenal Arsenal the item was refunded at (Can potentially be null)
	//! \return True if item was handed in at a friendly arsenal. If there was no arsenal, if arsenal has no faction or if it is hostile faction it will return false
	protected bool UsedFriendlyArsenal(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		//~ No arsenal so can never check if it was handed in at a friendly arsenal
		if (!arsenal)
			return false;

		SCR_Faction arsenalFaction = arsenal.GetAssignedFaction();
		if (!arsenalFaction)
			return false;
			
		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerController.GetPlayerId());	
		if (!playerFaction)
			return false;
		
		return arsenalFaction.DoCheckIfFactionFriendly(playerFaction);
	}
}