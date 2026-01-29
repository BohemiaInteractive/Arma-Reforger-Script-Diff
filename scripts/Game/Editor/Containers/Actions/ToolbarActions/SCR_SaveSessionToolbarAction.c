[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SaveSessionToolbarAction : SCR_EditorToolbarAction
{
	[Attribute(desc: "When enabled, the operation will always bring up a save dialog.")]
	protected bool m_bSaveAs;

	//------------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		//--- The action opens local UI
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		//--- Disallow in MP
		if (!Replication.IsServer())
			return false;

		//--- Disallow if mission is not configured for saving or currently suppressed
		return GetGame().GetSaveGameManager().IsSavingAllowed();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (m_bSaveAs)
		{
			new SCR_CreateNewSaveDialog();
			return;
		}

		ESaveGameRequestFlags saveFlags;
		if (RplSession.Mode() == RplMode.None)
			saveFlags = ESaveGameRequestFlags.BLOCKING;

		const SaveGameManager manager = GetGame().GetSaveGameManager();
		SaveGame currentSave = manager.GetActiveSave();
		if (currentSave)
		{
			auto rewind = SCR_RewindComponent.GetInstance();
			if (rewind)
			{
				if (rewind.IsRewindPoint(currentSave))
				{
					currentSave = null;
					const int currentPlaythrough = manager.GetCurrentPlaythroughNumber();

					array<SaveGame> saves();
					manager.GetSaves(saves, manager.GetCurrentMissionResource());
					foreach (SaveGame save : saves)
					{
						if (rewind.IsRewindPoint(save))
							continue;
			
						if (save.GetPlaythroughNumber() == currentPlaythrough)
							currentSave = save;
					}
				}
			}
		}

		if (currentSave)
		{
			manager.RequestSavePointOverwrite(currentSave, saveFlags);
			return;
		}

		manager.RequestSavePoint(ESaveGameType.MANUAL, flags: saveFlags);
	}
}
