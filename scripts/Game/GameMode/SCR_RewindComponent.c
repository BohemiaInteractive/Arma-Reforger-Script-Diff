[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "")]
class SCR_RewindComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_RewindComponent : SCR_BaseGameModeComponent
{
	protected ref ScriptInvokerVoid m_OnRewindPointChanged;
	protected const static string s_RewindPointName = "Rewind";
	protected SaveGame m_RewindPoint;
	protected bool m_bWasRewinded;

	//------------------------------------------------------------------------------------------------
	//! \return Local instance of this component
	static SCR_RewindComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;

		return SCR_RewindComponent.Cast(gameMode.FindComponent(SCR_RewindComponent));
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnRewindPointChanged()
	{
		if (!m_OnRewindPointChanged)
			m_OnRewindPointChanged = new ScriptInvokerVoid();

		return m_OnRewindPointChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Check if a rewind point exists for this session.
	bool CanRewind()
	{
		return GetGame().GetSaveGameManager().IsSavingPossible();
	}

	//------------------------------------------------------------------------------------------------
	//! \return Check if a rewind point exists for this session.
	bool HasRewindPoint()
	{
		return m_RewindPoint != null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Check if the current session was rewinded to from previous session.
	bool WasRewinded()
	{
		return m_bWasRewinded;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a rewind point for this session.
	void CreateRewindPoint()
	{
		if (!Replication.IsServer() || HasRewindPoint())
			return;

		GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.AUTO, s_RewindPointName, ESaveGameRequestFlags.BLOCKING, new SaveGameOperationCb(OnRewindPointCreated));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRewindPointCreated(bool success)
	{
		if (!success)
			return;

		m_RewindPoint = GetGame().GetSaveGameManager().GetActiveSave();
		m_OnRewindPointChanged.Invoke();

		RemoveUnusedRewindPoints();
	}

	//------------------------------------------------------------------------------------------------
	//! Remove existing rewind point for this session.
	void DeleteRewindPoint()
	{
		if (!m_RewindPoint)
			return;

		GetGame().GetSaveGameManager().Delete(m_RewindPoint);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRewindDeleted()
	{
		if (!m_RewindPoint)
			return;

		m_RewindPoint = null;
		m_OnRewindPointChanged.Invoke();

		RemoveUnusedRewindPoints();
	}

	//------------------------------------------------------------------------------------------------
	//! Rewind to existing rewind point of this session.
	void Rewind()
	{
		if (!m_RewindPoint)
			return;

		GetGame().GetSaveGameManager().Load(m_RewindPoint);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;

		const SaveGameManager manager = GetGame().GetSaveGameManager();
		EventProvider.ConnectEvent(manager.OnSaveCreated, OnSavedCreated);
		EventProvider.ConnectEvent(manager.OnSaveDeleted, OnSaveDeleted);

		const ResourceName currentMission = manager.GetCurrentMissionResource();
		manager.RetrieveSaveGameInfo({currentMission}, new SaveGameOperationCb(OnSavesLoaded));
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return;

		EventProvider.DisconnectEvent(GetGame().GetSaveGameManager().OnSaveCreated, OnSavedCreated);
		EventProvider.DisconnectEvent(GetGame().GetSaveGameManager().OnSaveDeleted, OnSaveDeleted);
		RemoveUnusedRewindPoints();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSavesLoaded(bool success)
	{
		// When loading a rewind we remember it to pause game time later, but delete it anyway. If user hits play again it will make a new rewind point
		const SaveGame save = GetGame().GetSaveGameManager().GetActiveSave();
		m_bWasRewinded = save && IsRewindPoint(save);
		RemoveUnusedRewindPoints();
	}

	//------------------------------------------------------------------------------------------------
	//! Remove any rewind points that were not cleaned up from last playthrough.
	protected void RemoveUnusedRewindPoints()
	{
		const SaveGameManager manager = GetGame().GetSaveGameManager();
		const int currentPlaythrough = manager.GetCurrentPlaythroughNumber();
		const SaveGame activeSave = manager.GetActiveSave();

		array<SaveGame> saves();
		manager.GetSaves(saves, manager.GetCurrentMissionResource());
		foreach (SaveGame save : saves)
		{
			if (save != activeSave && (save.GetPlaythroughNumber() == currentPlaythrough) && IsRewindPoint(save))
				manager.Delete(save);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsRewindPoint(notnull const SaveGame save)
	{
		return (save == m_RewindPoint) || (save.GetType() == ESaveGameType.AUTO && save.GetSavePointName() == s_RewindPointName);
	}

	//------------------------------------------------------------------------------------------------
	[ReceiverAttribute()]
	protected void OnSavedCreated(SaveGame save)
	{
		if (save && save.GetType() == ESaveGameType.MANUAL)
			DeleteRewindPoint();
	}

	//------------------------------------------------------------------------------------------------
	[ReceiverAttribute()]
	protected void OnSaveDeleted(SaveGame save)
	{
		if (save == m_RewindPoint)
			OnRewindDeleted();
	}
}

class SCR_RewindDialog : SCR_ConfigurableDialogUi
{
	//------------------------------------------------------------------------------------------------
	void SCR_RewindDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "rewind", this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		SCR_RewindComponent.GetInstance().Rewind();
	}
}
