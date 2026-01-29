[EntityEditorProps(category: "GameScripted/Editor", description: "Editor mode", color: "251 91 0 255", icon: "WBData/EntityEditorProps/entityEditor.png")]
class SCR_EditorModeEntityClass: SCR_EditorBaseEntityClass
{
};

/** @ingroup Editor_Entities
*/

/*!
Editor mode entity.

Must be a child of SCR_EditorManagerEntity.
- In multiplayer, the entity is **local to the player**.
- Only one mode can be activated at the same time. Together with it, all its components (SCR_BaseEditorComponent) are activated as well.
- A mode is the primary way to control editor functionality, an each mode can behave differently - use different GUI, camera, etc.
- Player can access the editor mode as long as the entity exists.
 + This is for security reasons. If the mode entity always existed, but mode access for disabled in GUI, functions for communication on server could potentially be exploited.
- Mode entity influences if the editor is considered limited. See SCR_EditorManagerEntity description for more details.
- Default editor mode prefabs are defined in SCR_EditorManagerCore (config is Configs/Core folder).

*/
class SCR_EditorModeEntity : SCR_EditorBaseEntity
{
	[Attribute(SCR_Enum.GetDefault(EEditorMode.EDIT), desc: "Mode this entity controls.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorMode), category: "Editor Mode")]
	private EEditorMode m_ModeType;
	
	[Attribute("-1", desc: "Order in which modes are listed. Lower number means earlier place.", category: "Editor Mode")]
	private int m_iOrder;
	
	[Attribute(desc: "Anti-exploit restriction will be in effect when all available modes are marked as limited.", category: "Editor Mode")]
	private bool m_bIsLimited;
	
	[Attribute(desc: "When enabled, removing this mode when it's current will close the editor instead of switching to the next available mode.", category: "Editor Mode")]
	private bool m_bCloseAfterRemoval;
	
	[Attribute(desc: "GUI representation of the mode.", category: "Editor Mode")]
	private ref SCR_UIInfo m_UIInfo;
	
	[Attribute("0", UIWidgets.ComboBox, "Mode add notification", "", ParamEnumArray.FromEnum(ENotification), category: "Notification")]
	protected ENotification m_ModeAddedNotification;
	[Attribute("0", UIWidgets.ComboBox, "Mode remove notification", "", ParamEnumArray.FromEnum(ENotification), category: "Notification")]
	protected ENotification m_ModeRemovedNotification;
	
	[Attribute("0", desc: "If true will only notify the local player when the mode is added or removed, if false it sends to to all Game Masters and the local player", category: "Notification")]
	protected bool m_bSendNotificationLocalOnly;
	
	[Attribute("false", UIWidgets.Auto, "True - Mode can't be closed", category: "Editor Mode")]
	protected bool m_bPreventClose;
	
	private SCR_EditorManagerEntity m_EditorManager;
	
	private ref ScriptInvoker Event_OnInit = new ScriptInvoker();
	private ref ScriptInvoker Event_OnPreActivate = new ScriptInvoker();
	private ref ScriptInvoker Event_OnActivate = new ScriptInvoker();
	private ref ScriptInvoker Event_OnPostActivate = new ScriptInvoker();
	private ref ScriptInvoker Event_OnDeactivate = new ScriptInvoker();
	
	private ref ScriptInvoker Event_OnActivateServer = new ScriptInvoker();
	private ref ScriptInvoker Event_OnDeactivateServer = new ScriptInvoker();

	/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
	//--- Getters
	/*!
	Get type of this mode.
	\return Mode type
	*/
	EEditorMode GetModeType()
	{
		return m_ModeType;
	}
	/*!
	Get order of the mode.
	\return Order
	*/
	int GetOrder()
	{
		return m_iOrder;
	}
	/*!
	Get GUI info representing the mode.
	\return Info class
	*/
	SCR_UIInfo GetInfo()
	{
		return m_UIInfo;
	}
	/*!
	Check if the mode is marked as limited.
	\return True when limited
	*/
	bool IsLimited()
	{
		return m_bIsLimited;
	}
	/*!
	\return When true, removing this mode when it's current will close the editor instead of switching to the next available mode.
	*/
	bool ShouldCloseAfterRemoval()
	{
		return m_bCloseAfterRemoval;
	}
	/*!
	Check if the mode is currently active.
	\return True when active
	*/
	bool IsCurrent()
	{
		return m_EditorManager && m_EditorManager.GetCurrentModeEntity() == this;
	}
	/*!
	Get current editor mode entity.
	\return Editor mode entity
	*/
	static SCR_EditorModeEntity GetInstance()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager) return null;
		
		return editorManager.GetCurrentModeEntity();
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
	//--- Event bases 
	void PreActivateMode()
	{
		if (!m_EditorManager.IsOpened() && !m_EditorManager.IsInTransition())
			return;
		
		Event_OnPreActivate.Invoke();
	}
	void ActivateMode(bool isInit = false)
	{
		if (!m_EditorManager.IsOpened() && !m_EditorManager.IsInTransition())
			return;
		
		SetFlags(EntityFlags.ACTIVE);
		Event_OnActivate.Invoke();
	}
	void PostActivateMode()
	{
		if (!m_EditorManager.IsOpened() && !m_EditorManager.IsInTransition())
			return;
		
		Event_OnPostActivate.Invoke();
	}
	void DeactivateMode()
	{
		if (!m_EditorManager.IsOpened() && !m_EditorManager.IsInTransition())
			return;
		
		Event_OnDeactivate.Invoke();
		ClearFlags(EntityFlags.ACTIVE);
	}
	void ActivateModeServer()
	{
		Event_OnActivateServer.Invoke();
	}
	void DeactivateModeServer()
	{
		Event_OnDeactivateServer.Invoke();
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
	//--- Notification
	/*!
	If true will only notify the local player when the mode is added or removed else it sends it to all GMs and local player
	\return If should send locally
	*/
	bool SendNotificationLocalOnly()
	{
		return m_bSendNotificationLocalOnly;
	}
	
	//----------------------------------------------------------------------------------------
	bool GetPreventClose()
	{
		return m_bPreventClose;
	}
	
	/*!
	Returns enum for on Add notification
	\return ENotification on Add
	*/
	ENotification GetOnAddNotification()
	{
		return m_ModeAddedNotification;
	}
	
	/*!
	Returns enum for on Remove notification
	\return ENotification on Remove
	*/
	ENotification GetOnRemoveNotification()
	{
		return m_ModeRemovedNotification;
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
	//--- Default functions
	override ScriptInvoker GetOnInit()
	{
		return Event_OnInit;
	}
	
	override ref ScriptInvoker GetOnRequest()
	{
		if (!GetManager()) return null;
		ScriptInvoker invoker = GetManager().GetOnRequest();
		return invoker;
	}
	override ref ScriptInvoker GetOnOpened()
	{
		if (!GetManager()) return null;
		ScriptInvoker invoker = GetManager().GetOnOpened();
		return invoker;
	}
	override ref ScriptInvoker GetOnOpenedServer()
	{
		if (!GetManager()) return null;
		ScriptInvoker invoker = GetManager().GetOnOpenedServer();
		return invoker;
	}
	override ref ScriptInvoker GetOnOpenedServerCallback()
	{
		if (!GetManager()) return null;
		ScriptInvoker invoker = GetManager().GetOnOpenedServerCallback();
		return invoker;
	}
	override ScriptInvoker GetOnPreActivate()
	{
		return Event_OnPreActivate;
	}
	override ScriptInvoker GetOnActivate()
	{
		return Event_OnActivate;
	}
	override ScriptInvoker GetOnPostActivate()
	{
		return Event_OnPostActivate;
	}
	override ScriptInvoker GetOnDeactivate()
	{
		return Event_OnDeactivate;
	}
	override ScriptInvoker GetOnClosed()
	{
		if (!GetManager()) return null;
		ScriptInvoker invoker = GetManager().GetOnClosed();
		return invoker;
	}
	override ref ScriptInvoker GetOnClosedServer()
	{
		if (!GetManager()) return null;
		ScriptInvoker invoker = GetManager().GetOnClosedServer();
		return invoker;
	}
	override ref ScriptInvoker GetOnClosedServerCallback()
	{
		if (!GetManager()) return null;
		ScriptInvoker invoker = GetManager().GetOnClosedServerCallback();
		return invoker;
	}
	override ScriptInvoker GetOnActivateServer()
	{
		return Event_OnActivateServer;
	}
	override ScriptInvoker GetOnDeactivateServer()
	{
		return Event_OnDeactivateServer;
	}
	override ScriptInvoker GetOnDebug()
	{
		if (!GetManager()) return null;
		ScriptInvoker invoker = GetManager().GetOnDebug();
		return invoker;
	}
	override bool IsOpened()
	{
		return m_EditorManager && m_EditorManager.IsOpened();
	}
	override SCR_EditorManagerEntity GetManager()
	{
		return m_EditorManager;
	}
	void InitServer(SCR_EditorManagerEntity editorManager)
	{
		RplComponent rplEditor = RplComponent.Cast(editorManager.FindComponent(RplComponent));
		RplComponent rplMode = RplComponent.Cast(FindComponent(RplComponent));
		if (!rplEditor || !rplMode) return;
		
		rplMode.Give(Replication.FindOwner(rplEditor.Id()));
		
		m_EditorManager = editorManager;
		InitComponents(true);
		//Rpc(InitOwner);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void InitOwner()
	{
		//--- Activate to allow events in components
		SetFlags(EntityFlags.ACTIVE);
		
		m_EditorManager = SCR_EditorManagerEntity.GetInstance();
		if (!m_EditorManager)
		{
			Print("Error when initialized editor mode!", LogLevel.ERROR);
			return;
		}
		
		InitComponents(false);
		Event_OnInit.Invoke();
	}
	void SCR_EditorModeEntity(IEntitySource src, IEntity parent)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
		{
			world.RegisterEntityToBeUpdatedWhileGameIsPaused(this);
		}
		
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
	}
	void ~SCR_EditorModeEntity()
	{
		if (m_EditorManager) m_EditorManager.RemoveMode(this, false);
		Event_OnDeactivate.Invoke();
		
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
		{
			world.UnregisterEntityToBeUpdatedWhileGameIsPaused(this);
		}
	}
};