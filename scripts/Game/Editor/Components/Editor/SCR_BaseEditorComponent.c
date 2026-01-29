[ComponentEditorProps(category: "GameScripted/Editor", insertable: false)]
class SCR_BaseEditorComponentClass: ScriptComponentClass
{
	[Attribute(category: "Effects")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsActivate;
	
	[Attribute(category: "Effects")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsDeactivate;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_BaseEditorEffect> GetEffectsActivate()
	{
		return m_EffectsActivate;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_BaseEditorEffect> GetEffectsDeactivate()
	{
		return m_EffectsDeactivate;
	}

	//------------------------------------------------------------------------------------------------
	//! Get prefab data of local editor component instance attached on SCR_EditorManagerEntity or SCR_EditorModeEntity.
	//! \param[in] type Type of requested component (not its prefab data, i.e., without *Class suffix!)
	//! \param[in] showError True to log a warning message when the component was not found
	//! \return Component prefab data
	static SCR_BaseEditorComponentClass GetInstance(typename type, bool showError)
	{
		SCR_BaseEditorComponent component = SCR_BaseEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(type, showError));
		if (component)
			return SCR_BaseEditorComponentClass.Cast(component.GetEditorComponentData());
		else
			return null;
	}
};

//! @ingroup Editor_Components

//! Base class for editor components.
//! - Editor components handle actual functionality of the editor, like GUI, camera or entity management.
//! - They must inherit from this class and be attached to SCR_EditorBaseEntity, otherwise they won't initialize.
//! - The base class offers several pre-made functions from which custom editor components can inherit, e.g., \ref EOnEditorOpen or \ref EOnEditorActivate.
//!
//! # Execution Order
//! \dot
//! digraph SCR_BaseEditorComponent_Order
//! {
//! 	compound=true;
//! 	newrank=true;
//! 	fontname="sans-serif";
//! 	fontsize=12;
//! 	splines=ortho;
//! 	style=filled;
//! 	node [shape=box, fontname="sans-serif", height=0.25, fontsize=10, style=filled, fillcolor=white];
//! 	edge [fontname="sans-serif", fontsize=9];
//!
//! 	EOnEditorInit [label="{ EOnEditorInit | EOnEditorInitServer }", shape=record, URL="\ref EOnEditorInit"];
//! 	EOnEditorRequest1 [URL="\ref EOnEditorRequest"];
//! 	EOnEditorClose [label="{ EOnEditorClose | EOnEditorCloseServer }", shape=record, URL="\ref EOnEditorClose"];
//! 	EOnEditorDelete [label="{ EOnEditorDelete | EOnEditorDeleteServer }", shape=record, URL="\ref EOnEditorDelete"];
//!
//! 	subgraph cluster_Opened
//! 	{
//! 		label="Editor Open";
//! 		margin=24;
//! 		color=gray95;
//! 		fontcolor=gray50;
//! 		fontsize=16;
//!
//! 		EOnEditorOpen [label="{ EOnEditorOpen | EOnEditorOpenServer }", shape=record, URL="\ref EOnEditorOpen"];
//! 		EOnEditorPreActivate [URL="\ref EOnEditorPreActivate"];
//!
//! 		subgraph cluster_Mode
//! 		{
//! 			label="Editor Mode Active";
//! 			margin=24;
//! 			color=gray85;
//!
//! 			EOnEditorActivate [URL="\ref EOnEditorActivate"];
//! 			EOnEditorPostActivate [URL="\ref EOnEditorPostActivate"];
//! 			EOnEditorDeactivate [URL="\ref EOnEditorDeactivate"];
//!
//! 			subgraph cluster_Frame
//! 			{
//! 				label="Each Frame";
//! 				margin=24;
//! 				color=gray75;
//!
//! 				EOnFrame [URL="\ref EOnFrame"];
//! 				EOnEditorDebug [URL="\ref EOnEditorDebug", style="dashed, filled"];
//! 			}
//! 		}
//! 		EOnEditorRequest2 [URL="\ref EOnEditorRequest"];
//! 	}
//!
//! 	EOnEditorInit -> EOnEditorRequest1 -> EOnEditorOpen -> EOnEditorPreActivate -> EOnEditorActivate -> EOnEditorPostActivate ->
//! 	EOnFrame -> EOnEditorDebug -> EOnEditorDeactivate -> EOnEditorRequest2 -> EOnEditorClose -> EOnEditorDelete;
//!
//! 	EOnEditorDebug -> EOnFrame;
//! 	EOnEditorDeactivate -> EOnEditorPreActivate;
//! 	EOnEditorClose -> EOnEditorOpen;
//!
//! 	{rank=same; EOnEditorInit;}
//! 	{rank=same; EOnEditorRequest1;}
//! 	{rank=same; EOnEditorOpen;}
//! 	{rank=same; EOnEditorPreActivate;}
//! 	{rank=same; EOnEditorActivate;}
//! 	{rank=same; EOnEditorPostActivate;}
//! 	{rank=same; EOnFrame;}
//! 	{rank=same; EOnEditorDebug;}
//! 	{rank=same; EOnEditorDeactivate;}
//! 	{rank=same; EOnEditorRequest2;}
//! 	{rank=same; EOnEditorClose;}
//! 	{rank=same; EOnEditorDelete;}
//! }
//! \enddot
class SCR_BaseEditorComponent : ScriptComponent
{	
	protected SCR_EditorBaseEntity m_Owner;
	protected SCR_EditorManagerEntity m_Manager;
	protected SCR_BaseEditorComponent m_Parent;
	protected RplComponent m_RplComponent;
	private bool m_bOpen;
	
	private ref ScriptInvoker m_OnEffect;
	
	//! @name Template Methods (Owner)
	//! Methods to be overridden by child classes, called where the editor is **local**.
	//!
	//! @{
	
	//------------------------------------------------------------------------------------------------
	//! When the entity is created
	protected void EOnEditorInit();

	//------------------------------------------------------------------------------------------------
	//! When the entity is destroyed
	protected void EOnEditorDelete();

	//------------------------------------------------------------------------------------------------
	//! When opening/closing request is sent to server
	protected void EOnEditorRequest(bool isOpen);

	//------------------------------------------------------------------------------------------------
	//! When the editor is opened
	protected void EOnEditorOpen();

	//------------------------------------------------------------------------------------------------
	//! When the editor is closed
	protected void EOnEditorClose();
	
	//------------------------------------------------------------------------------------------------
	//! Before the component is activated.
	//! - When attached on SCR_EditorManagerEntity, this is executed together with EOnEditorOpen()
	//! - When attached on SCR_EditorModeEntity, it's executed before previous mode is deactivated, so values can be read from it
	protected void EOnEditorPreActivate();

	//------------------------------------------------------------------------------------------------
	//! When the component is activated.
	//! - When attached on SCR_EditorManagerEntity, this is executed together with EOnEditorOpen()
	//! - When attached on SCR_EditorModeEntity, it's executed when the mode is activated
	protected void EOnEditorActivate();

	//------------------------------------------------------------------------------------------------
	//! When the component is activated. Asynchronous, can take multiple frames in case of complex operations.
	//! \param[in] attempt Number of previous attempts to activate the component.
	//! \return True if the component is activated, false if it isn't (another attempt will follow in the next frame).
	protected bool EOnEditorActivateAsync(int attempt)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! When the component is activated, but after EOnEditorActivate() function was called in all components
	protected void EOnEditorPostActivate();

	//------------------------------------------------------------------------------------------------
	//! When the component is deactivated.
	//! - When attached on SCR_EditorManagerEntity, this is executed together with EOnEditorClose()
	//! - When attached on SCR_EditorModeEntity, it's executed when the mode is deactivated
	protected void EOnEditorDeactivate();

	//------------------------------------------------------------------------------------------------
	//! When the component is deactivated. Asynchronous, can take multiple frames in case of complex operations.
	//! \param[in] attempt Number of previous attempts to deactivate the component.
	//! \return true if the component is activated, false if it isn't (another attempt will follow in the next frame).
	protected bool EOnEditorDeactivateAsync(int attempt)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! When the component is deactivated, but after the next mode is activated
	protected void EOnEditorPostDeactivate();

	//------------------------------------------------------------------------------------------------
	//! Every frame while the editor is opened and debug menu is shown
	protected void EOnEditorDebug(array<string> debugTexts);
	
	//------------------------------------------------------------------------------------------------
	//! When an effect is created in reaction an an event
	protected void EOnEffect(SCR_BaseEditorEffect effect);
	//! @}
	
	//! @name Template Methods (Server)
	//! Methods to be overridden by child classes, called on **server**.
	//!
	//! @{
	
	//------------------------------------------------------------------------------------------------
	//! When the entity is created (called on server)
	protected void EOnEditorInitServer();

	//------------------------------------------------------------------------------------------------
	//! When the entity is destroyed (called on server)
	protected void EOnEditorDeleteServer();

	//------------------------------------------------------------------------------------------------
	//! When the editor is opened (called on server)
	protected void EOnEditorOpenServer();

	//------------------------------------------------------------------------------------------------
	//! When the editor is opened (called on server after async loading finished on client)
	protected void EOnEditorOpenServerCallback();

	//------------------------------------------------------------------------------------------------
	//! When the editor is closed (called on server)
	protected void EOnEditorCloseServer();

	//------------------------------------------------------------------------------------------------
	//! When the editor is closed (called on server after async loading finished on client)
	protected void EOnEditorCloseServerCallback();

	//------------------------------------------------------------------------------------------------
	//! When the component is activated (called on server)
	//! - When attached on SCR_EditorManagerEntity, this is executed together with EOnEditorOpen()
	//! - When attached on SCR_EditorModeEntity, it's executed when the mode is activated
	protected void EOnEditorActivateServer();

	//------------------------------------------------------------------------------------------------
	//! When the component is deactivated (called on server)
	//! - When attached on SCR_EditorManagerEntity, this is executed together with EOnEditorClose()
	//! - When attached on SCR_EditorModeEntity, it's executed when the mode is deactivated
	protected void EOnEditorDeactivateServer();
	//! @}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Event handlers	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	sealed void OnInitBase()
	{
		EOnEditorInit();
	}

	//------------------------------------------------------------------------------------------------
	sealed void OnDeleteBase()
	{
		EOnEditorDelete();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] isOpen
	sealed void OnRequestBase(bool isOpen)
	{
		EOnEditorRequest(isOpen);
	}

	//------------------------------------------------------------------------------------------------
	sealed void OnOpenedBase()
	{
		m_bOpen = true;
		EOnEditorOpen();
	}

	//------------------------------------------------------------------------------------------------
	sealed void OnClosedBase()
	{
		m_bOpen = false;
		//OnDeactivateBase();
		EOnEditorClose();
	}

	//------------------------------------------------------------------------------------------------
	sealed void OnPreActivateBase()
	{
		EOnEditorPreActivate();
	}

	//------------------------------------------------------------------------------------------------
	sealed void OnActivateBase()
	{
		Activate(m_Owner);
		//SetEventMask(m_Owner, EntityEvent.FRAME);
		EOnEditorActivate();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] attempt
	//! \return
	sealed bool OnActivateAsyncBase(int attempt)
	{
		return EOnEditorActivateAsync(attempt);
	}

	//------------------------------------------------------------------------------------------------
	sealed void OnPostActivateBase()
	{
		EOnEditorPostActivate();
		
		SCR_BaseEditorComponentClass prefabData = SCR_BaseEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData)
			SCR_BaseEditorEffect.Activate(prefabData.GetEffectsActivate(), this);
	}

	//------------------------------------------------------------------------------------------------
	sealed void OnDeactivateBase()
	{
		if (!IsActive())
			return;
		
		//--- Deactivate only when it's not being deleted, so that proper cleanup can be arranged in destructor
		if (!GetOwner().IsDeleted())
			Deactivate(m_Owner);
		
		EOnEditorDeactivate();
		
		SCR_BaseEditorComponentClass prefabData = SCR_BaseEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData)
			SCR_BaseEditorEffect.Activate(prefabData.GetEffectsDeactivate(), this);
	}

	//------------------------------------------------------------------------------------------------
	sealed void OnPostDeactivateBase()
	{
		EOnEditorPostDeactivate();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] attempt
	//! \return
	sealed bool OnDeactivateAsyncBase(int attempt)
	{
		return EOnEditorDeactivateAsync(attempt);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] effect
	sealed void OnEffectBase(SCR_BaseEditorEffect effect)
	{
		EOnEffect(effect);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Custom functions
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//------------------------------------------------------------------------------------------------
	//! Get the local instance of a component attached on editor manager (SCR_EditorManagerEntity).
	//! When not found on editor manager, it will be searched for in current editor mode (SCR_EditorModeEntity).
	//! \param[in] type Requested component type
	//! \param[in] showError True to log a warning message when the component was not found
	//! \param[in] modeFirst When true, search the component first on editor mode and then on editor manager. By default it's false, the other way around.
	//! \return Component
	static Managed GetInstance(typename type, bool showError = false, bool modeFirst = false)
	{
		//--- Find the component in Editor Manager entity
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
		{
			if (showError)
				Print(string.Format("Cannot find editor component '%1', local instance of editor manager not found!", type), LogLevel.ERROR);
			return null;
		}
		
		Managed component = editorManager.FindComponent(type);
		if (component && !modeFirst)
			return component;
		
		//--- Find the component in Editor Mode entity
		SCR_EditorModeEntity editorMode = SCR_EditorModeEntity.GetInstance();
		if (editorMode)
		{
			Managed componentMode = editorMode.FindComponent(type);
			if (componentMode)
			{
				return componentMode;
			}
			else if (showError)
			{
				Print(string.Format("Cannot find editor component '%1' on local instance of editor manager or on the curent editor mode!", type), LogLevel.ERROR);
			}
		}
		else if (showError)
		{
			Print(string.Format("Cannot find editor component '%1' on local instance of editor manager, and no current editor mode exists!", type), LogLevel.ERROR);
		}
		return component;
	}

	//------------------------------------------------------------------------------------------------
	//! Get instance of a component attached on specific editor manager / mode (SCR_EditorBaseEntity).
	//! When not found on editor manager, it will be searched for in current editor mode (SCR_EditorModeEntity).
	//! \param[in] editorManager Editor manager or mode which is searched
	//! \param[in] type Requested component type
	//! \param[in] showError True to log a warning message when the component was not found (useful when the component is required by a script)
	//! \return Component
	static Managed GetInstance(SCR_EditorBaseEntity editorManager, typename type, bool showError = false)
	{
		Managed component = editorManager.FindComponent(type);
		if (component)
			return component;
		
		SCR_EditorModeEntity editorMode = SCR_EditorModeEntity.Cast(editorManager);
		if (!editorMode)
			return null;
		
		editorManager = editorMode.GetManager();
		if (!editorManager)
			return null;
		
		return editorManager.FindComponent(type);
	}

	//------------------------------------------------------------------------------------------------
	//! Get component instances on editor manager (SCR_EditorBaseEntity) and all its modes (SCR_EditorModeEntity).
	//! \param[in] type Requested component type
	//! \param[out] outComponents Array to be filled with components
	//! \return Number of found components
	static int GetAllInstances(typename type, out notnull array<Managed> outComponents)
	{
		outComponents.Clear();

		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
			return 0;
		
		Managed component = editorManager.FindComponent(type);
		if (component)
			outComponents.Insert(component);
		
		array<SCR_EditorModeEntity> modeEntities = {};
		int modesCount = editorManager.GetModeEntities(modeEntities);
		for (int i = 0; i < modesCount; i++)
		{
			component = modeEntities[i].FindComponent(type);
			if (component)
				outComponents.Insert(component);
		}
		return outComponents.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get editor manager this component belongs to.
	//! \return Editor manager
	SCR_EditorManagerEntity GetManager()
	{
		if (!m_Owner)
			return null;

		SCR_EditorManagerEntity manager = SCR_EditorManagerEntity.Cast(m_Owner);
		if (!manager)
			manager = m_Owner.GetManager();

		return manager;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the editor manager to which this component belongs.
	//! \param[in] type
	//! \param[in] showError
	//! \param[in] modeFirst
	//! \return Editor manager
	SCR_BaseEditorComponent FindEditorComponent(typename type, bool showError = false, bool modeFirst = false)
	{
		//--- Find the component in Editor Manager entity
		SCR_EditorManagerEntity editorManager = GetManager();
		if (!editorManager)
		{
			if (showError)
				Print(string.Format("Cannot find editor component '%1', local instance of editor manager not found!", type), LogLevel.ERROR);
			return null;
		}
		
		SCR_BaseEditorComponent component = SCR_BaseEditorComponent.Cast(editorManager.FindComponent(type));
		if (component && !modeFirst)
			return component;
		
		//--- Find the component in Editor Mode entity
		SCR_EditorModeEntity editorMode = editorManager.GetCurrentModeEntity();
		if (editorMode)
		{
			SCR_BaseEditorComponent componentMode = SCR_BaseEditorComponent.Cast(editorMode.FindComponent(type));
			if (componentMode)
			{
				component = componentMode;
			}
			else if (showError && !component)
			{
				Print(string.Format("Cannot find editor component '%1' on local instance of editor manager or on the curent editor mode!", type), LogLevel.ERROR);
			}
		}
		else if (showError && !component)
		{
			Print(string.Format("Cannot find editor component '%1' on local instance of editor manager, and no current editor mode exists!", type), LogLevel.ERROR);
		}
		return component;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get prefab data of this component.
	//! \return Prefab data
	EntityComponentPrefabData GetEditorComponentData()
	{
		if (m_Owner)
			return GetComponentData(m_Owner);
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the component is being deleted.
	//! \return True if entity is ready to be deleted
	//!
	bool IsRemoved()
	{
		return !m_Owner || m_Owner.IsDeleted();
	}

	//------------------------------------------------------------------------------------------------
	//! Reset values controlled by the component.
	//! To be overridden by child classes.
	//!
	void ResetEditorComponent();

	//------------------------------------------------------------------------------------------------
	//! Get event called when an effect is activated.
	//! \return Script invoker
	ScriptInvoker GetOnEffect()
	{
		if (!m_OnEffect)
			m_OnEffect = new ScriptInvoker();

		return m_OnEffect;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsOnEditorManager()
	{
		return m_Owner && m_Owner.IsInherited(SCR_EditorManagerEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsOwner()
	{
		return m_RplComponent && m_RplComponent.IsOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return m_RplComponent && m_RplComponent.IsProxy();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsMaster()
	{
		return m_RplComponent && m_RplComponent.IsMaster();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsAdmin()
	{
		//--- ToDo: Proper admin detection once admin feature is implemented
		return SCR_Global.IsAdmin(GetManager().GetPlayerID()) && Replication.IsRunning();
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_BaseEditorComponent GetParentComponent()
	{
		return m_Parent;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetParentComponent(SCR_BaseEditorComponent parent)
	{
		if (m_Parent)
			return;

		m_Parent = parent;
	}
	
	//------------------------------------------------------------------------------------------------
	//If target Entity is given then location is used to target position unless: m_bSetLocationOnce is true in the data found in m_aNotificationDisplayInfos (SCR_NotificationManagerEditorComponent on the EditorManager)
	protected void SendNotification(ENotification notificationID, int selfID = 0, int targetID = 0, vector position = vector.Zero)
	{
		//Send notification
		SCR_NotificationsComponent.SendLocal(notificationID, position, selfID, targetID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void InitServer()
	{
		if (!m_Owner)
			return;
		
		if (m_Owner.GetOnOpenedServer())
			m_Owner.GetOnOpenedServer().Insert(EOnEditorOpenServer);

		if (m_Owner.GetOnOpenedServerCallback())
			m_Owner.GetOnOpenedServerCallback().Insert(EOnEditorOpenServerCallback);

		if (m_Owner.GetOnActivateServer())
			m_Owner.GetOnActivateServer().Insert(EOnEditorActivateServer);

		if (m_Owner.GetOnDeactivateServer())
			m_Owner.GetOnDeactivateServer().Insert(EOnEditorDeactivateServer);

		if (m_Owner.GetOnClosedServer())
			m_Owner.GetOnClosedServer().Insert(EOnEditorCloseServer);

		if (m_Owner.GetOnClosedServerCallback())
			m_Owner.GetOnClosedServerCallback().Insert(EOnEditorCloseServerCallback);
		
		EOnEditorInitServer();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void InitOwner()
	{
		if (!m_Owner)
			return;

		//if (m_Owner.GetOnInit()) m_Owner.GetOnInit().Insert(OnInitBase);
		//if (m_Owner.GetOnRequest()) m_Owner.GetOnRequest().Insert(OnRequestBase);
		//if (m_Owner.GetOnOpened()) m_Owner.GetOnOpened().Insert(OnOpenedBase);
		//if (m_Owner.GetOnPreActivate()) m_Owner.GetOnPreActivate().Insert(OnPreActivateBase);
		//if (m_Owner.GetOnActivate()) m_Owner.GetOnActivate().Insert(OnActivateBase);
		//if (m_Owner.GetOnPostActivate()) m_Owner.GetOnPostActivate().Insert(OnPostActivateBase);
		//if (m_Owner.GetOnDeactivate()) m_Owner.GetOnDeactivate().Insert(OnDeactivateBase);
		//if (m_Owner.GetOnClosed()) m_Owner.GetOnClosed().Insert(OnClosedBase);
		if (m_Owner.GetOnDebug())
			m_Owner.GetOnDebug().Insert(EOnEditorDebug);
		
		m_Manager = SCR_EditorManagerEntity.Cast(m_Owner);
		if (!m_Manager)
			m_Manager = m_Owner.GetManager();
		
//		if (m_Owner.IsOpened())
//		{
//			OnInitBase();
//			OnOpenedBase();
//		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!m_Owner)
			return;

		m_RplComponent = RplComponent.Cast(m_Owner.FindComponent(RplComponent)); //--- Must be here, it's too early in the constructor
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_BaseEditorComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{	
		if (SCR_Global.IsEditMode(ent)) //--- Run-time only
			return;
		
		m_Owner = SCR_EditorBaseEntity.Cast(ent);
		if (!m_Owner)
		{
			Print("SCR_BaseEditorComponent must be attached to SCR_EditorBaseEntity!", LogLevel.ERROR);
			return;
		}
		
		Deactivate(null);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_BaseEditorComponent()
	{
		if (Replication.IsServer())
			EOnEditorDeleteServer();
	}
}
