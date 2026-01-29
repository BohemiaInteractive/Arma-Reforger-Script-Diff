[ComponentEditorProps(category: "GameScripted/Callsign", description: "")]
class SCR_CallsignBaseComponentClass : ScriptComponentClass
{
}

//~ ScriptInvokers
void SCR_CallsignBaseComponent_OnCallsignChanged(int companyIndex, int platoonIndex, int squadIndex, int characterNumber, ERoleCallsign characterRole);
typedef func SCR_CallsignBaseComponent_OnCallsignChanged;

//! Component of assigning and storing squad names
class SCR_CallsignBaseComponent : ScriptComponent
{
	protected Faction m_Faction;
	protected SCR_FactionCallsignInfo m_CallsignInfo;
	
	//Callsign IDs(Broadcast)
	protected int m_iCompanyCallsign = -1;
	protected int m_iPlatoonCallsign = -1;
	protected int m_iSquadCallsign = -1;
	
	//State
	protected bool m_bIsServer;
	
	//Ref
	protected SCR_CallsignManagerComponent m_CallsignManager;
	
	//Will always return int Company, int Platoon, int squad, int character number and character role (See GetCallsignIndexes() comments for more information). Also called when assigning callsigns for first time
	protected ref ScriptInvokerBase<SCR_CallsignBaseComponent_OnCallsignChanged> Event_OnCallsignChanged = new ScriptInvokerBase<SCR_CallsignBaseComponent_OnCallsignChanged>();

	//------------------------------------------------------------------------------------------------
	//! Get the callsign names assigned
	//! \param[out] company Company name
	//! \param[out] platoon Platoon name
	//! \param[out] squad Squad name
	//! \param[out] character Character name (Optional if callsign assigned to a character). Will return Role name if any role is assigned to the character
	//! \param[out] format Format of callsign
	//! \return returns true if names are succesfully found
	bool GetCallsignNames(out string company, out string platoon, out string squad, out string character, out string format)
	{	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the callsign indexes assigned
	//! \param[out] companyIndex company index. Eg: Company Alpha is index 0 and Beta is index 1
	//! \param[out] platoonIndex platoon index. Is an index similar to how Company works.
	//! \param[out] squadIndex squad index. Is an index similar to how Company works.
	//! \param[out] characterNumber character callsign number. This is not an index. Soldier 1 will have character 1. The second soldier in the group will have character 2. Will return -1 if component is on a group
	//! \param[out] characterRole Character role number, roles can be checked with ERoleCallsign eg: characterRole == ERoleCallsign.SQUAD_LEADER will let you know if the character is a squad leader etc. Will return -1 if component is on a group
	//! \return returns false if indexes are not assigned
	bool GetCallsignIndexes(out int companyIndex, out int platoonIndex, out int squadIndex, out int characterNumber = -1, out ERoleCallsign characterRole = ERoleCallsign.NONE)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearCallsigns()
	{
		m_CallsignInfo = null;
		
		m_iCompanyCallsign = -1;
		m_iPlatoonCallsign = -1;
		m_iSquadCallsign = -1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool SetCallsignInfo()
	{
		Faction currentFaction;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(GetOwner());
		if (group && group.IsPlayable())
				currentFaction = group.GetFaction();
		else
		{
			SCR_EditableEntityComponent editableEntityComponent = SCR_EditableEntityComponent.GetEditableEntity(GetOwner());
			if (editableEntityComponent)
				currentFaction = editableEntityComponent.GetFaction();
		}
		
		if (m_Faction != currentFaction && currentFaction != null)
		{
			m_Faction = currentFaction;
			m_CallsignInfo = null;
		}
		else 
		{
			if (m_CallsignInfo)
				return true;
		}
		
		SCR_Faction scrFaction = SCR_Faction.Cast(m_Faction);
		
		if (!scrFaction)
			return false;
		
		m_CallsignInfo = scrFaction.GetCallsignInfo();
		return m_CallsignInfo != null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerBase<SCR_CallsignBaseComponent_OnCallsignChanged> GetOnCallsignChanged()
	{
		return Event_OnCallsignChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitOnServer(IEntity owner)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		m_CallsignManager = SCR_CallsignManagerComponent.Cast(gameMode.FindComponent(SCR_CallsignManagerComponent));
		if (!m_CallsignManager)
			return;
		
		SCR_EditableEntityComponent editableEntityComponent = SCR_EditableEntityComponent.GetEditableEntity(owner);
		if (editableEntityComponent)
			m_Faction = editableEntityComponent.GetFaction();
		m_bIsServer = Replication.IsServer();
		
		if (!m_bIsServer)
			return;
		
		GetGame().GetCallqueue().CallLater(InitOnServer, 1, false, owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
}
