[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_IdentityInventoryItemComponentClass : InventoryItemComponentClass
{
}

class SCR_IdentityInventoryItemComponent : InventoryItemComponent 
{	
	[Attribute("1", desc: "Check if identity item is of a hostile faction and only then will it be considered a valuable intel. The CanShowValuableIntel() function is able to force the check for faction")]
	protected bool m_bCheckIfItemIsHostileFaction;
	
	[RplProp()]//onRplName: "OnLinkedIdentitySet")]
	protected RplId m_RplIdLinkedIdentity = -1;
	
	[RplProp(onRplName: "OnIntelFactionSet")] //~ Will be set to faction int which will determine if the item has valuable intel of the given faction
	protected int m_iValuableIntelFaction = -1;
	
	protected Faction m_IntelFaction;
	
	//------------------------------------------------------------------------------------------------
	void SCR_IdentityInventoryItemComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && !gameMode.IsMaster()) || (!gameMode && Replication.IsClient()))
			return;
		
		//~ Call a frame later so SCR_ExtendedCharacterIdentityComponent can init correctly
		GetGame().GetCallqueue().CallLater(DelayedInit, param1: ent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool ShouldChangeVisibilityOfHierarchy()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		ChimeraCharacter character;
		IEntity parent = owner.GetParent();
		
		while (parent)
		{
			character = ChimeraCharacter.Cast(parent);
			if (character)
				break;
			
			parent = parent.GetParent();
		}
		
		if (!character)
			return;

		SCR_ExtendedCharacterIdentityComponent rootExtendedIdentity = SCR_ExtendedCharacterIdentityComponent.Cast(character.FindComponent(SCR_ExtendedCharacterIdentityComponent));
		if (!rootExtendedIdentity)
			return;
		
		RplComponent rplComp = RplComponent.Cast(rootExtendedIdentity.GetOwner().FindComponent(RplComponent));
		if (!rplComp)
			return;
		
		m_RplIdLinkedIdentity = Replication.FindId(rplComp);
		
		//~ This identity holds valuable intel. So make sure that is shown
		m_iValuableIntelFaction = rootExtendedIdentity.OnIdentityItemAdded_S(this, SCR_IdentityManagerComponent.Static_IsGenerateValuableIntelEnabled());
		
		if (m_iValuableIntelFaction >= 0)
			OnIntelFactionSet();

		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ExtendedIdentityComponent GetLinkedExtendedIdentity()
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(m_RplIdLinkedIdentity));
		if (!rplComponent)
			return null;
		
		IEntity owner = rplComponent.GetEntity();
		if (!owner)
			return null;
		
		return SCR_ExtendedIdentityComponent.Cast(owner.FindComponent(SCR_ExtendedIdentityComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if Identity item is valuable intel for given (or local) player
	//! \param[in] forceCheckForFaction If true will check if the faction of the player is friendly even if OnlyShowIntilIfHostileFaction is false
	//! \param[in] playerController Player controller to check for faction. If null will use local player controller
	//! \return True if the Identity Item has valuable intel for the character
	bool HasValuableIntel(bool forceCheckForFaction = false, PlayerController playerController = null)
	{
		if (m_iValuableIntelFaction < 0 || !m_IntelFaction || !SCR_IdentityManagerComponent.Static_IsGenerateValuableIntelEnabled())
			return false;
		
		if (forceCheckForFaction || ShouldCheckIfItemIsHostileFaction())
		{
			//~ Get local player controller if none was given
			if (!playerController)
				playerController = GetGame().GetPlayerController();
			
			Faction playerFaction;
			
			//~ Get player faction
			if (playerController)
				playerFaction = SCR_FactionManager.SGetPlayerFaction(playerController.GetPlayerId());
			else 
				playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
			
			if (!playerFaction)
				return false;
			
			//~ Only show intel for hostile faction identity items
			SCR_Faction scrFaction = SCR_Faction.Cast(playerFaction);
			if ((scrFaction && scrFaction.DoCheckIfFactionFriendly(m_IntelFaction)) || (!scrFaction && playerFaction.IsFactionFriendly(m_IntelFaction)))
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Should it check if the identity item is of a hostile faction?
	bool ShouldCheckIfItemIsHostileFaction()
	{
		return m_bCheckIfItemIsHostileFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetValuableIntelFactionID()
	{
		return m_iValuableIntelFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetValuableIntelFaction()
	{
		return m_IntelFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CallsignBaseComponent GetLinkedCallsignComponent()
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(m_RplIdLinkedIdentity));
		if (!rplComponent)
			return null;
		
		IEntity owner = rplComponent.GetEntity();
		if (!owner)
			return null;
		
		return SCR_CallsignBaseComponent.Cast(owner.FindComponent(SCR_CallsignBaseComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnIntelFactionSet()
	{
		if (m_iValuableIntelFaction < 0)
			return;
		
		m_IntelFaction = GetGame().GetFactionManager().GetFactionByIndex(m_iValuableIntelFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	/*void ReplicateIdentity()
	{
		//~ Call on Parent changed
		
		//~ Create the identity component 
		//~ Send over values 
		//~ Fill in the identity components with given values
		
		//~ Other values to set up are: GroupCallsign, Rank
	}*/
	
	//------------------------------------------------------------------------------------------------
	/*protected void OnLinkedIdentitySet()
	{
		//Print(m_RplIdLinkedIdentity);
	}	*/
}
