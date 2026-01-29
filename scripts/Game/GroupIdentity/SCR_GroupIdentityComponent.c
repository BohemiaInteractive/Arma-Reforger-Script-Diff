void SCR_GroupIdentityComponent_OnChange(SCR_MilitarySymbol symbol, LocalizedString name);
typedef func SCR_GroupIdentityComponent_OnChange;

[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_GroupIdentityComponentClass : ScriptComponentClass
{
}

class SCR_GroupIdentityComponent : ScriptComponent
{
	[Attribute()]
	protected ref SCR_MilitarySymbol m_MilitarySymbol;
	
	[Attribute(desc: "When enabled, the group will use the identity defined in 'Military Symbol' attribute and ignore automatic evaluation.")]
	protected bool m_bForced;
	
	protected SCR_AIGroup m_Group;
	protected int m_iNameID = -1;
	protected LocalizedString m_sName;
	protected ref ScriptInvokerBase<SCR_GroupIdentityComponent_OnChange> m_OnIdentityChange = new ScriptInvokerBase<SCR_GroupIdentityComponent_OnChange>();
	
	//------------------------------------------------------------------------------------------------
	//! Get group's symbol data.
	//! \return Symbol data
	SCR_MilitarySymbol GetMilitarySymbol()
	{
		return m_MilitarySymbol;
	}

	//------------------------------------------------------------------------------------------------
	//! Get group's dynamic name.
	//! \return Name
	LocalizedString GetGroupName()
	{
		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return invoker called when group identity changes.
	ScriptInvokerBase<SCR_GroupIdentityComponent_OnChange> GetOnIdentityChange()
	{
		return m_OnIdentityChange;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update group identity based on its current state.
	void UpdateIdentity()
	{
		//--- Server only
		if (Replication.IsClient())
			return;
		
		//--- Save the current symbol for later comparison
		SCR_MilitarySymbol original = new SCR_MilitarySymbol();
		original.CopyFrom(m_MilitarySymbol);
		
		//--- Update military symbol
		SCR_GroupIdentityCore core = SCR_GroupIdentityCore.Cast(SCR_GroupIdentityCore.GetInstance(SCR_GroupIdentityCore));
		SCR_MilitarySymbolRuleSet ruleSet = core.GetSymbolRuleSet();
		ruleSet.UpdateSymbol(m_MilitarySymbol, m_Group);
		
		//--- Broadcast the symbol if it changed
		if (!m_MilitarySymbol.IsEqual(original) || m_iNameID == -1)
		{
			//--- Get name based on the symbol
			SCR_GroupNameConfig nameManager = core.GetNames();
			int nameID = nameManager.GetGroupNameID(m_MilitarySymbol);
			
			//--- Apply changes
			UpdateIdentityBroadcast(nameID, m_MilitarySymbol);
			Rpc(UpdateIdentityBroadcast, nameID, m_MilitarySymbol);
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void UpdateIdentityBroadcast(int nameID, SCR_MilitarySymbol symbol)
	{
		SCR_GroupIdentityCore core = SCR_GroupIdentityCore.Cast(SCR_GroupIdentityCore.GetInstance(SCR_GroupIdentityCore));
		SCR_GroupNameConfig nameManager = core.GetNames();
		
		m_MilitarySymbol = symbol;
		m_iNameID = nameID;
		m_sName = nameManager.GetGroupName(m_iNameID);
		
		m_OnIdentityChange.Invoke(m_MilitarySymbol, m_sName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGroupInit(SCR_AIGroup aiGroup)
	{
		UpdateIdentity();
		m_Group.GetOnAgentAdded().Insert(OnAgentAdded);
		m_Group.GetOnAgentRemoved().Insert(OnAgentRemoved);
		m_Group.GetOnAgentRemoved().Remove(OnAgentRemoved);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAgentAdded()
	{
		UpdateIdentity();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAgentRemoved()
	{
		UpdateIdentity();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		m_MilitarySymbol.OnRplSave(writer);
		writer.WriteInt(m_iNameID);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//--- JIP on client
	override bool RplLoad(ScriptBitReader reader)
	{
		m_MilitarySymbol.OnRplLoad(reader);
		reader.ReadInt(m_iNameID);
		
		UpdateIdentityBroadcast(m_iNameID, m_MilitarySymbol);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;
		
		m_Group = SCR_AIGroup.Cast(owner);
		if (!m_Group)
		{
			Print("SCR_GroupIdentityComponent must be attached to SCR_AIGroup!", LogLevel.WARNING);
			return;
		}
		
		if (!m_bForced)
			m_Group.GetOnInit().Insert(OnGroupInit);
		//--- ToDo: When member gets in/out of vehicle
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (!m_bForced && m_Group)
		{
			m_Group.GetOnInit().Remove(OnGroupInit);
			m_Group.GetOnAgentAdded().Remove(OnAgentAdded);
			m_Group.GetOnAgentRemoved().Remove(OnAgentRemoved);
		}
	}
}
