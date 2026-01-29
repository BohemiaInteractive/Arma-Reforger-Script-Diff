[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_PlayersPresentTriggerEntityClass: SCR_BaseTriggerEntityClass
{
};

/*
	The trigger invokes the onActivate event in case the number of the connected players of the given faction 
	is present in the trigger
*/

//O B S O L E T E !!! DON'T USE IT
//WILL BE DELETED

class SCR_PlayersPresentTriggerEntity: SCR_BaseTriggerEntity
{
	[Attribute(desc: "Faction which is used for area control calculation. Leave empty for any faction.", category: "Trigger")]
	protected FactionKey 		m_sOwnerFactionKey;
	
	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkTriggerActivation), category: "Trigger")]
	protected SCR_EScenarioFrameworkTriggerActivation	m_eActivationPresence;
	
	/*
	[Attribute(desc: "Trigger activation - all connected players of the given faction or just one?", category: "Trigger")]
	protected bool 			m_bAllPlayersPresent;
	*/
	
	protected Faction 			m_OwnerFaction;
	protected int 				m_iEntitiesInside = 0;
	
	//------------------------------------------------------------------------------------------------
	void SetOwnerFaction(Faction faction)
	{
		m_OwnerFaction = faction;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsMaster()// IsServer
	{
		RplComponent comp = RplComponent.Cast(FindComponent(RplComponent));
		return comp && comp.IsMaster();
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetPlayersCountByFaction()
	{
		int iCnt = 0;
		array<int> aPlayerIDs = {};
		SCR_PlayerController playerController;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID: aPlayerIDs)
		{
			playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
			if (!playerController)
				continue;

			if (playerController.GetLocalControlledEntityFaction() == m_OwnerFaction)
				iCnt++;
		}
		return iCnt;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetPlayersByFaction(notnull out array<IEntity> aOut)
	{
		array<int> aPlayerIDs = {};
		SCR_PlayerController playerController;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID: aPlayerIDs)
		{
			playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
			if (!playerController)
				continue;

			if (playerController.GetLocalControlledEntityFaction() == m_OwnerFaction)
				aOut.Insert(playerController.GetLocalMainEntity());
		}
	}
	
	
	//! Override this method in inherited class to define a new filter.
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (!ChimeraCharacter.Cast(ent))
			return false;

		// take only players
		if (m_eActivationPresence == SCR_EScenarioFrameworkTriggerActivation.PLAYER)
		{
			if (!EntityUtils.IsPlayer(ent))
				return false;
		}
		if (m_sOwnerFactionKey.IsEmpty())
			return true;	//if faction is not specified, any faction can (de)activate the trigger

		FactionAffiliationComponent pFaciliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (!pFaciliation)
			return false;

		return pFaciliation.GetAffiliatedFaction() == m_OwnerFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnActivate(IEntity ent)
	{
		if (!IsMaster())
			return;
			
		m_iEntitiesInside++;
		if (m_eActivationPresence == SCR_EScenarioFrameworkTriggerActivation.PLAYER || m_eActivationPresence == SCR_EScenarioFrameworkTriggerActivation.ANY_CHARACTER)
		{
			m_OnActivate.Invoke(ent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnDeactivate(IEntity ent)
	{
		m_iEntitiesInside--;
		m_OnDeactivate.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_OwnerFaction = factionManager.GetFactionByKey(m_sOwnerFactionKey);
	}
};
