[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_PlayerDelegateEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
Manager of player delegate - entity representing the player (works even when controlled entity doesn't exist, e.g., when player is waiting for respawn)
*/
class SCR_PlayerDelegateEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(params: "et")]
	protected ResourceName m_PlayerDelegatePrefab;
	
	protected int m_iPlayerID;
	protected SCR_EditablePlayerDelegateComponent m_PlayerDelegate;
	protected ref map<int, SCR_EditablePlayerDelegateComponent> m_Delegates = new map<int, SCR_EditablePlayerDelegateComponent>;
	protected ref ScriptInvoker m_OnLimitedEditorChanged = new ScriptInvoker;
	
	/*!
	Register delegate *locally*
	\param delegate Player delegate
	*/
	void RegisterDelegate(SCR_EditablePlayerDelegateComponent delegate)
	{
		m_Delegates.Set(delegate.GetPlayerID(), delegate);
		
		delegate.GetOnLimitedEditorChanged().Insert(OnLimitedEditorChanged);
		OnLimitedEditorChanged(delegate.GetPlayerID(), delegate.HasLimitedEditor());
	}
	/*!
	Unregister delegate *locally*
	\param delegate Player delegate
	*/
	void UnegisterDelegate(SCR_EditablePlayerDelegateComponent delegate)
	{
		m_Delegates.Remove(delegate.GetPlayerID());
		
		delegate.GetOnLimitedEditorChanged().Remove(OnLimitedEditorChanged);
		OnLimitedEditorChanged(delegate.GetPlayerID(), true);
	}
	/*!
	Get all player delegates.
	\param outDelegates Map in format <player ID, delegate entity> filled by the function
	\return Number of delegates
	*/
	int GetDelegates(out notnull map<int, SCR_EditablePlayerDelegateComponent> outDelegates)
	{
		return outDelegates.Copy(m_Delegates);
	}
	/*!
	Get delegate of player with given ID.
	\param playerID Player ID
	\return Delegate entity
	*/
	SCR_EditablePlayerDelegateComponent GetDelegate(int playerID)
	{
		SCR_EditablePlayerDelegateComponent delegate;
		m_Delegates.Find(playerID, delegate);
		return delegate;
	}
	/*!
	\return True if the current session has at least one player with unlimited editor.
	*/
	bool HasPlayerWithUnlimitedEditor()
	{
		if (!m_Delegates || m_Delegates.IsEmpty())
			return false;
		
		for (int i, count = m_Delegates.Count(); i < count; i++)
		{
			if (!m_Delegates.GetElement(i))
				continue;
			
			if (!m_Delegates.GetElement(i).HasLimitedEditor())
				return true;
		}
		return false;
	}
	/*!
	Get script invoker called when editor belonging to any player become limited (e.g., only photo mode, without any editing capabilities), or vice versa.
	\return ScriptInvoker
	*/
	ScriptInvoker GetOnLimitedEditorChanged()
	{
		return m_OnLimitedEditorChanged;
	}
	
	protected void OnLimitedEditorChanged(int playerID, bool isLimited)
	{
		m_OnLimitedEditorChanged.Invoke(playerID, isLimited);
	}
	
	protected void OnSpawnServer(int playerID, IEntity controlledEntity)
	{
		if (m_PlayerDelegate && m_iPlayerID == playerID) 
			m_PlayerDelegate.SetControlledEntity(controlledEntity);
	}
	protected void OnDeathServer(notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (m_PlayerDelegate && m_iPlayerID == instigatorContextData.GetVictimPlayerID()) 
			m_PlayerDelegate.SetControlledEntity(null);
	}
	
	protected void OnPlayerDeletedServer(int playerID, IEntity controlledEntity)
	{
		OnDeathServer(new SCR_InstigatorContextData(playerID, controlledEntity, null, Instigator.CreateInstigatorGM(), true));
	}
	
	override void EOnEditorInitServer()
	{
		Resource entityResource = Resource.Load(m_PlayerDelegatePrefab);
		IEntityComponentSource componentSource = SCR_EditablePlayerDelegateComponentClass.GetEditableEntitySource(entityResource);
		if (!componentSource)
		{
			Print(string.Format("SCR_EditablePlayerDelegateComponent missing in '%1'!", m_PlayerDelegatePrefab.GetPath()), LogLevel.WARNING);
			return;
		}	
		
		if (!componentSource.GetClassName().ToType().IsInherited(SCR_EditablePlayerDelegateComponent))
		{
			Print(string.Format("'%1' contains editable entity entity of type %2, must be SCR_EditablePlayerDelegateComponent!", m_PlayerDelegatePrefab.GetPath(), componentSource.GetClassName()), LogLevel.WARNING);
			return;
		}
		
		SCR_EditorManagerEntity editorManager = GetManager();
		m_iPlayerID = editorManager.GetPlayerID();
		
		IEntity delegateEntity = GetGame().SpawnEntityPrefab(entityResource);
		m_PlayerDelegate = SCR_EditablePlayerDelegateComponent.Cast(SCR_EditablePlayerDelegateComponent.GetEditableEntity(delegateEntity));
		m_PlayerDelegate.InitPlayerDelegate(editorManager, m_iPlayerID);
		m_PlayerDelegate.RestoreParentEntity();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerSpawned().Insert(OnSpawnServer);
			gameMode.GetOnPlayerKilled().Insert(OnDeathServer);
			gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeletedServer);
			
			m_PlayerDelegate.SetControlledEntity(SCR_PossessingManagerComponent.GetPlayerMainEntity(m_iPlayerID));
		}
		else
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController && playerController.GetPlayerId() == m_iPlayerID)
			{
				m_PlayerDelegate.SetControlledEntity(playerController.GetMainEntity());
			}
		}
	}
	override void EOnEditorDeleteServer()
	{
		if (m_PlayerDelegate)
			m_PlayerDelegate.Delete();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerSpawned().Remove(OnSpawnServer);
			gameMode.GetOnPlayerKilled().Remove(OnDeathServer);
			gameMode.GetOnPlayerDeleted().Remove(OnPlayerDeletedServer);
		}
	}
};