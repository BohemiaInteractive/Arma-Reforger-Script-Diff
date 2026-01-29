void SCR_PersistenceSystem_OnStateChanged(EPersistenceSystemState oldState, EPersistenceSystemState newState);
typedef func SCR_PersistenceSystem_OnStateChanged;

void SCR_PersistenceSystem_OnBeforeSave(ESaveGameType saveType);
typedef func SCR_PersistenceSystem_OnBeforeSave;

void SCR_PersistenceSystem_OnAfterSave(ESaveGameType saveType, bool success);
typedef func SCR_PersistenceSystem_OnAfterSave;

class SCR_PersistenceSystem : PersistenceSystem
{
	protected ref ScriptInvokerBase<SCR_PersistenceSystem_OnStateChanged> m_OnStateChanged = new ScriptInvokerBase<SCR_PersistenceSystem_OnStateChanged>();
	protected ref ScriptInvokerBase<SCR_PersistenceSystem_OnBeforeSave> m_OnBeforeSave = new ScriptInvokerBase<SCR_PersistenceSystem_OnBeforeSave>();
	protected ref ScriptInvokerBase<SCR_PersistenceSystem_OnAfterSave> m_OnAfterSave = new ScriptInvokerBase<SCR_PersistenceSystem_OnAfterSave>();

	//------------------------------------------------------------------------------------------------
	override protected void OnStateChanged(EPersistenceSystemState oldState, EPersistenceSystemState newState)
	{
		m_OnStateChanged.Invoke(oldState, newState);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnBeforeSave(ESaveGameType saveType)
	{
		m_OnBeforeSave.Invoke(saveType);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnAfterSave(ESaveGameType saveType, bool success)
	{
		m_OnAfterSave.Invoke(saveType, success);

		if (success)
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_SESSION_SAVE_SUCCESS);
		else
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_SESSION_SAVE_FAIL);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnAfterLoad(bool success)
	{
		if (System.IsConsoleApp())
			return;

		if (!success)
		{
			GetGame().GetCallqueue().Call(DeferredNotifcation, ENotification.EDITOR_SESSION_LOAD_FAIL);
			return;
		}

		const SaveGameManager manager = GetGame().GetSaveGameManager();
		if (manager && manager.GetActiveSave())
			GetGame().GetCallqueue().Call(DeferredNotifcation, ENotification.EDITOR_SESSION_LOAD_SUCCESS);
	}

	//------------------------------------------------------------------------------------------------
	override protected void HandleDelete(IEntity entity)
	{
		SCR_EntityHelper.DeleteBuilding(entity);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeferredNotifcation(ENotification notification)
	{
		SCR_NotificationsComponent.SendLocal(notification);
	}

	//------------------------------------------------------------------------------------------------
	sealed ScriptInvokerBase<SCR_PersistenceSystem_OnStateChanged> GetOnStateChanged()
	{
		return m_OnStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	sealed ScriptInvokerBase<SCR_PersistenceSystem_OnBeforeSave> GetOnBeforeSave()
	{
		return m_OnBeforeSave;
	}

	//------------------------------------------------------------------------------------------------
	sealed ScriptInvokerBase<SCR_PersistenceSystem_OnAfterSave> GetOnAfterSave()
	{
		return m_OnAfterSave;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Utility method to suppress unwanted UI or sound events during load of saves.
		\param msSinceLoad Will report as loading for the specified time in milliseconds after the world load completed to catch events that fire on the first few frames.
	*/
	sealed static bool IsLoadInProgress(float msSinceLoad = 1000.0)
	{
		// TODO: We need to know if and what data to load ahead of even world systems init. Add pre world load phase.
		if (!GetGame().GetSaveGameManager().GetActiveSave())
			return false;

		const PersistenceSystem instance = GetInstance();
		if (!instance)
			return false;

		const EPersistenceSystemState state = instance.GetState();
		if (instance.GetState() < EPersistenceSystemState.ACTIVE)
			return true;

		return instance.WasDataLoaded() && (GetGame().GetWorld().GetWorldTime() < msSinceLoad);
	}

	//------------------------------------------------------------------------------------------------
	sealed static SCR_PersistenceSystem GetScriptedInstance()
	{
		return SCR_PersistenceSystem.Cast(GetInstance());
	}

	//------------------------------------------------------------------------------------------------
	sealed static SCR_PersistenceSystem GetByCurrentWorld()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return null;

		return SCR_PersistenceSystem.Cast(world.FindSystem(SCR_PersistenceSystem));
	}

	//------------------------------------------------------------------------------------------------
	sealed static SCR_PersistenceSystem GetByEntityWorld(IEntity entity)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(entity.GetWorld());
		if (!world)
			return null;

		return SCR_PersistenceSystem.Cast(world.FindSystem(SCR_PersistenceSystem));
	}
}
