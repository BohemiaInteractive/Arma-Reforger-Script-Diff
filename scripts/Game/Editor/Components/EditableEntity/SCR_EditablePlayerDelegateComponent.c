[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditablePlayerDelegateComponentClass : SCR_EditableEntityComponentClass
{
}

//! @ingroup Editable_Entities

//! Editable entity representing a player.
//! Used to guarantee player representation even when controlled entity is not present (e.g., player is dead)
class SCR_EditablePlayerDelegateComponent : SCR_EditableEntityComponent
{
	[RplProp(onRplName: "OnRplPlayerID")]
	protected int m_iPlayerID = 0;
	
	[RplProp(onRplName: "OnRplLimitedEditor")]
	protected bool m_bHasLimitedEditor;
	
	protected SCR_EditableEntityComponent m_ControlledEntity; //--- Don't use RplProp(), controlled entity cannot be found on client using Replication.FindItem() right after being spawned
	
	protected ref ScriptInvoker m_OnUIReset = new ScriptInvoker();
	protected ref ScriptInvoker m_OnLimitedEditorChanged = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//! Initialise the delegate.
	//! Can be called only once.
	//! \param[in] editorManager
	//! \param[in] playerID Player ID
	void InitPlayerDelegate(SCR_EditorManagerEntity editorManager, int playerID)
	{
		if (m_iPlayerID != 0) return;
		
		m_iPlayerID = playerID;
		OnRplPlayerID();
		
		m_bHasLimitedEditor = editorManager.IsLimited();
		editorManager.GetOnLimitedChange().Insert(OnEditorLimitedChange); //--- No Remove() later needed - delegate is removed when the manager is
		
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Get ID of player this delegates represents.
	//! \return Player ID
	override int GetPlayerID()
	{
		return m_iPlayerID;
	}

	//------------------------------------------------------------------------------------------------
	//! Get script invoker called when editor belonging to this player become limited (e.g., only photo mode, without any editing capabilities), or vice versa.
	//! \return ScriptInvoker
	ScriptInvoker GetOnLimitedEditorChanged()
	{
		return m_OnLimitedEditorChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the player has access only to limited editor (e.g., only photo mode, without any editing capabilities).
	//! \return True when limited
	bool HasLimitedEditor()
	{
		return m_bHasLimitedEditor;
	}

	//------------------------------------------------------------------------------------------------
	//! Update reference to editable entity controlled by the player this delegate represents.
	//! \param[in] controlledEntity New controlled entity
	void SetControlledEntity(IEntity controlledEntity)
	{
		RplId entityID = Replication.FindId(SCR_EditableEntityComponent.GetEditableEntity(controlledEntity));
		SetControlledEntityOwner(entityID);
		Rpc(SetControlledEntityOwner, entityID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetControlledEntityOwner(RplId entityID)
	{
		m_ControlledEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		m_OnUIReset.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! Get reference to editable entity controlled by the player this delegate represents.
	//! \return Controlled entity
	SCR_EditableEntityComponent GetControlledEntity()
	{
		return m_ControlledEntity;
	}
	
	//--- OnRpl methods

	//------------------------------------------------------------------------------------------------
	protected void OnRplPlayerID()
	{
		if (!SCR_EditorManagerEntity.GetInstance())
		{
			//--- Initialized before the editor - wait
			SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorManagerCore) editorManagerCore.Event_OnEditorManagerInitOwner.Insert(OnRplPlayerID);
			return;
		}
		else
		{
			SCR_EditorManagerCore editorManagerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (editorManagerCore) editorManagerCore.Event_OnEditorManagerInitOwner.Remove(OnRplPlayerID);
		}
		
		SCR_PlayerDelegateEditorComponent delegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent, true));
		if (delegateManager)
		delegateManager.RegisterDelegate(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRplLimitedEditor()
	{
		m_OnLimitedEditorChanged.Invoke(m_iPlayerID, m_bHasLimitedEditor);
	}
	
	//--- Invokers

	//------------------------------------------------------------------------------------------------
	protected void OnEditorLimitedChange(bool isLimited)
	{
		m_bHasLimitedEditor = isLimited;
		OnRplLimitedEditor();
		Replication.BumpMe();
	}
	
	//--- Overrides
	override EEditableEntityType GetEntityType(IEntity owner = null)
	{
		if (m_ControlledEntity)
			return m_ControlledEntity.GetEntityType(owner);
		else
			return super.GetEntityType(owner);
	}

	//------------------------------------------------------------------------------------------------
	override SCR_UIInfo GetInfo(IEntity owner = null)
	{
		if (m_ControlledEntity)
		{
			SCR_UIInfo info = m_ControlledEntity.GetInfo(owner);
			return info;
		}
		else
		{
			return super.GetInfo();
		}
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnUIReset()
	{
		return m_OnUIReset;
	}

	//------------------------------------------------------------------------------------------------
	override Faction GetFaction()
	{
		if (m_ControlledEntity)
		{
			Faction faction = m_ControlledEntity.GetFaction();
			return faction;
		}
		else
		{
			return null;
		}
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetAIGroup()
	{
		if (m_ControlledEntity)
			return m_ControlledEntity.GetAIGroup();
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetVehicle()
	{
		if (m_ControlledEntity)
			return m_ControlledEntity.GetVehicle();
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override float GetHealth()
	{
		if (m_ControlledEntity)
			return m_ControlledEntity.GetHealth();
		else
			return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetEntityBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets, IEntity owner = null)
	{
		// Return true and empty cost array, avoid fallback entityType cost
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetPos(out vector pos)
	{
		if (m_ControlledEntity)
			return m_ControlledEntity.GetPos(pos);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	override string GetLogText(string prefix = "")
	{
		return super.GetLogText(prefix) + string.Format(", ctrl: %1", m_ControlledEntity);
	}

	//--- JIP on server

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;
		
		RplId controlledEntityRplID = Replication.FindId(m_ControlledEntity);
		writer.WriteRplId(controlledEntityRplID);
		
		return true;
	}

	//--- JIP on client

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;
		
		RplId controlledEntityRplID;
		reader.ReadRplId(controlledEntityRplID);
		
		SetControlledEntityOwner(controlledEntityRplID);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		owner.SetFlags(EntityFlags.NO_LINK, true);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_EditablePlayerDelegateComponent()
	{
		SCR_PlayerDelegateEditorComponent delegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
		if (delegateManager)
			delegateManager.UnegisterDelegate(this);
	}
}
