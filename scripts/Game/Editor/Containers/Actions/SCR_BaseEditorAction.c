enum EEditorActionType
{
	ACTION,
	TOGGLE,
	DYNAMIC
};
enum EEditorActionGroup
{
	NONE,
	SIMULATION,
	SAVING,
	TOOLS,
	DYNAMIC,
	COMMAND_WAYPOINT, ///< Waypoints in command bar
	COMMAND_TASK, ///< Tasks in command bar
	COMMAND_SPAWN_POINT, ///< Spawn points in command bar
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot:true)]
/*!
Base container class for all editor action types (ContextActions, WaypointActions, AbiliityActions)
*/
class SCR_BaseEditorAction
{
	[Attribute("1", desc: "When disabled, the action will not appear in action list.\nUse to temporary hide actions without losing their configuration.")]
	protected bool m_bEnabled;
	
	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	
	[Attribute(SCR_Enum.GetDefault(EEditorActionType.ACTION), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorActionType))]
	protected EEditorActionType m_ActionType;
	
	[Attribute(SCR_Enum.GetDefault(EEditorActionGroup.NONE), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorActionGroup))]
	protected EEditorActionGroup m_ActionGroup;
	
	[Attribute(desc: "Lower number is higher priority in the order")]
	protected int m_iOrder;
	
	[Attribute()]
	protected string m_sShortcut;
	
	[Attribute(defvalue: "true", desc: "If false will not execute the shortcut via toolbar when pressed")]
	protected bool m_bEnableShortcutLogics;

	[Attribute(category: "Effects")]
	protected ref array<ref SCR_BaseEditorEffect> m_Effects;
	
	protected SCR_BaseActionsEditorComponent m_ActionsManager;
	protected string m_sShortcutRef;
	
	[Attribute("0", desc: "Cooldown to prevent spamming of action. Leave zero or less to ignore, time in seconds.", params: "0, inf, 0.05")]
	protected float m_fCooldownTime;
	
	protected float m_fCurrentCooldownTime;
	
	//~ In mili seconds what is the update freq for the cooldown update. If changed updated the step param for m_fCooldownTime (m_fCooldownUpdateFreq / 1000)
	protected float m_fCooldownUpdateFreq = 50; 
	
	[Attribute("1", desc: "If true will show notification if trying to perform the action if the cooldown is active.")]
	protected bool m_bShowOnCooldownNotification;
	
	protected bool m_bIsOnCooldown; 
	
	/*!
	\return True if the action is enabled
	*/
	bool IsEnabled()
	{
		return m_bEnabled;
	}
	
	/*!
	Determine if action can be shown with passed hovered entity and selected entitites
	*/
	bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		Print(string.Format("CanBeShown condition method not overridden for action %1, action won't show", Type()), LogLevel.WARNING);
		return false;
	}
	
	/*!
	Determine if action can be performed with passed hovered entity and selected entitites
	*/
	bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	/*!
	Perform action
	*/
	void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		
	}
	
	/*
	Local execution only, eg. effects or UI	
	*/
	void PerformOwner(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		
	}
	
	/*
	Get UI info representing the action.
	\return UI info
	*/
	SCR_UIInfo GetInfo()
	{
		//if (!m_Info)
		//	m_Info = SCR_UIInfo.CreatePlaceholderInfo(SCR_UIInfo);
		
		return m_Info;
	}
	
	EEditorActionType GetActionType()
	{
		return m_ActionType;
	}
	EEditorActionGroup GetActionGroup()
	{
		return m_ActionGroup;
	}
	
	/*!
	Get action shortcut that triggers this action.
	\return Action name
	*/
	string GetShortcut()
	{
		if (m_sShortcutRef)
			return m_sShortcutRef;
		else
			return m_sShortcut;
	}
	/*!
	Set shortcut reference - it's shown in GUI, but not actually listened to.
	\param shortcutRef Action name
	*/
	void SetShortcutRef(string shortcutRef)
	{
		m_sShortcutRef = shortcutRef;
	}
	
	/*!
	Get order of this entity, can be any integer value including negative.	
	*/
	int GetOrder()
	{
		return m_iOrder;
	}
	
	/*!
	Should action be sent to server
	\return True if action is handled on server
	*/
	bool IsServer()
	{
		return true;
	}
	
	/*!
	Get custom param which can be used to pass extra information to performed action.
	Called on the machine of whoever called the action, with the editor opened.
	Sent to Perform function, even when it's called on server.
	\return Param value
	*/
	int GetParam()
	{
		return -1;
	}
	
	/*!
	Get effects tied to the action
	\return Array of effects
	*/
	array<ref SCR_BaseEditorEffect> GetEffects()
	{
		return m_Effects;
	}
	
	protected void OnShortcut()
	{
		if (m_ActionsManager)
			m_ActionsManager.ActionPerformInstantly(this);
	}
	void AddShortcut(SCR_BaseActionsEditorComponent actionsManager)
	{
		if (m_sShortcut.IsEmpty() || m_bEnableShortcutLogics == false)
			return;
		
		m_ActionsManager = actionsManager;
		GetGame().GetInputManager().AddActionListener(m_sShortcut, EActionTrigger.DOWN, OnShortcut);
	}
	void RemoveShortcut()
	{
		if (m_sShortcut.IsEmpty() || m_bEnableShortcutLogics == false)
			return;
		
		m_ActionsManager = null;
		GetGame().GetInputManager().RemoveActionListener(m_sShortcut, EActionTrigger.DOWN, OnShortcut);
	}
	
	/*!
	Check if action is on cooldown and if not set action on cooldown
	The cooldown will never be set if cooldownTime is equal or less than 0
	\return Returns true if currently on cooldown
	*/
	bool CheckAndSetCooldown()
	{
		//~ No Cooldown
		if (m_fCooldownTime <= 0)
			return false;
		
		//~ Is On Cooldown
		if (IsOnCooldown())
		{
			//~ Send cooldown notification
			if (m_bShowOnCooldownNotification)
				SCR_NotificationsComponent.SendLocal(ENotification.ACTION_ON_COOLDOWN, m_fCurrentCooldownTime * 100);
			
			return true;
		}
			
		//~ Activate cooldown
		ActivateCooldown();
		return false;
	}
	
	/*!
	Check if action is on cooldown
	\return Returns true if currently on cooldown
	*/
	bool IsOnCooldown()
	{
		return m_bIsOnCooldown;
	}
	
	//~ Start Cooldown
	protected void ActivateCooldown()
	{
		if (m_fCooldownTime <= 0)
			return;
		
		m_fCurrentCooldownTime = m_fCooldownTime;
		
		m_bIsOnCooldown = true;
		
		//~ Add to callqueue to remove cooldown
		GetGame().GetCallqueue().CallLater(UpdateCooldown, m_fCooldownUpdateFreq, true);
	}
	
	//~ Update current cooldown
	protected void UpdateCooldown()
	{
		m_fCurrentCooldownTime -= m_fCooldownUpdateFreq / 1000; 
		
		if (m_fCurrentCooldownTime <= 0)
			OnCooldownDone();
	}
	
	//~ Cooldown done
	protected void OnCooldownDone()
	{
		GetGame().GetCallqueue().Remove(UpdateCooldown);
		m_bIsOnCooldown = false;
	}
	
	void ~SCR_BaseEditorAction()
	{
		//~ Remove cooldown from Call Queue
		if (m_bIsOnCooldown)
			OnCooldownDone();
	}
};

class SCR_EditorActionData
{
	private SCR_BaseEditorAction m_EditorAction;
	private bool m_CanBePerformed;
	
	SCR_BaseEditorAction GetAction()
	{
		return m_EditorAction;
	}
	
	bool GetCanBePerformed()
	{
		return m_CanBePerformed;
	}
	
	void SCR_EditorActionData(SCR_BaseEditorAction action, bool canPerform)
	{
		m_EditorAction = action;
		m_CanBePerformed = canPerform;
	}
};


/* TESTING MULTIPLAYER SHENANIGANS
class SCR_BaseEditorActionParameters
{	
	int m_iTestInt;
	
	void SCR_BaseEditorActionParameters()
	{
		
	}
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{	
		return lhs.CompareSnapshots(rhs, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_BaseEditorActionParameters prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return  snapshot.Compare(prop.m_iTestInt, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_BaseEditorActionParameters prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(prop.m_iTestInt, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_BaseEditorActionParameters prop) 
	{
		snapshot.SerializeBytes(prop.m_iTestInt, 4);		
		return true;
	}
};

class SCR_ContextActionParameters : SCR_BaseEditorActionParameters
{
	vector m_CursorWorldPosition = "100 100 100";
	
	void SCR_ContextActionParameters(vector cursorWorldPosition)
	{
		m_CursorWorldPosition = cursorWorldPosition;
	}
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static override void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 16);
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 16);
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{
		return lhs.CompareSnapshots(rhs, 16);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_ContextActionParameters prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return SCR_BaseEditorActionParameters.PropCompare(prop, snapshot, ctx)
		&& snapshot.Compare(prop.m_CursorWorldPosition[0], 4)
		&& snapshot.Compare(prop.m_CursorWorldPosition[1], 4)
		&& snapshot.Compare(prop.m_CursorWorldPosition[2], 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_ContextActionParameters prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		SCR_BaseEditorActionParameters.Extract(prop, ctx, snapshot);
		snapshot.SerializeBytes(prop.m_CursorWorldPosition[0], 4);
		snapshot.SerializeBytes(prop.m_CursorWorldPosition[1], 4);
		snapshot.SerializeBytes(prop.m_CursorWorldPosition[2], 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_ContextActionParameters prop) 
	{
		SCR_BaseEditorActionParameters.Inject(snapshot, ctx, prop);
		float temp;
		prop.m_CursorWorldPosition = vector.Zero;
		snapshot.SerializeBytes(temp, 4);
		prop.m_CursorWorldPosition[0] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_CursorWorldPosition[1] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_CursorWorldPosition[2] = temp;
		return true;
	}
};

class SCR_WaypointActionParameters : SCR_ContextActionParameters
{
	
};

class SCR_ToolbarActionParameters : SCR_ContextActionParameters
{
	int m_iPlacedEntityId;
	bool m_bShortcutTriggered;
	
	void SCR_ToolbarActionParameters(vector cursorWorldPosition, bool shortcutTriggered = false, int placedEntityId = -1)
	{
		m_CursorWorldPosition = cursorWorldPosition;
		m_iPlacedEntityId = placedEntityId;
		m_bShortcutTriggered = shortcutTriggered;
	}
	
		//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static override void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 20);
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 20);
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{
		return lhs.CompareSnapshots(rhs, 20);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_ToolbarActionParameters prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return SCR_ContextActionParameters.PropCompare(prop, snapshot, ctx)
		&& snapshot.Compare(prop.m_iPlacedEntityId, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_ToolbarActionParameters prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		SCR_ContextActionParameters.Extract(prop, ctx, snapshot);
		snapshot.SerializeBytes(prop.m_iPlacedEntityId, 4);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_ToolbarActionParameters prop) 
	{
		SCR_ContextActionParameters.Inject(snapshot, ctx, prop);
		snapshot.SerializeBytes(prop.m_iPlacedEntityId, 4);
		return true;
	}
};
*/