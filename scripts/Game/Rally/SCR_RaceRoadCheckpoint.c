class SCR_RaceRoadCheckpointClass : SCR_RaceCheckpointClass
{
}

class SCR_RaceRoadCheckpoint : SCR_RaceCheckpoint
{
	[Attribute(defvalue: "0", UIWidgets.EditBox, desc: "Unique ID of the checkpoint. This ID affects the order of checkpoints!"), SortAttribute()]
	protected int m_iCheckpointID;

	//------------------------------------------------------------------------------------------------
	override void OnCheckpointReached()
	{
		super.OnCheckpointReached();
		DisableCheckpoint();

		SCR_RaceEntity race = GetRaceEntity();
		if (!race)
		{
			Print("SCR_RaceCheckpoint:: Checkpoint has no parent SCR_RaceEntity!", LogLevel.WARNING);
			return;
		}

		SCR_RaceCheckpoint nextCheckpoint = race.FindNextCheckpoint(this);
		if (!nextCheckpoint)
			return;

		nextCheckpoint.EnableCheckpoint();
	}

	//------------------------------------------------------------------------------------------------
	override int GetCheckpointID()
	{
		return m_iCheckpointID;
	}
}
