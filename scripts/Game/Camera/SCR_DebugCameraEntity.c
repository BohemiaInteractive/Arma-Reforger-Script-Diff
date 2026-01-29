[EntityEditorProps(category: "GameScripted/Camera", description: "Manual camera", color: "0 255 255 255")]
class SCR_DebugCameraEntityClass : SCR_CameraBaseClass
{
}

//! Debug camera spawned on given position.
class SCR_DebugCameraEntity : GenericEntity
{
	//------------------------------------------------------------------------------------------------
	protected void CreateDebugCamera()
	{
		SCR_DebugCameraCore core = SCR_DebugCameraCore.Cast(SCR_DebugCameraCore.GetInstance(SCR_DebugCameraCore));
		if (!core)
		{
			Print("Config SCR_DebugCameraCore not found!", LogLevel.WARNING);
			return;
		}

		vector transform[4];
		GetWorldTransform(transform);
		core.CreateCamera(transform);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param src
	//! \param parent
	void SCR_DebugCameraEntity(IEntitySource src, IEntity parent)
	{
		GetGame().GetCallqueue().CallLater(CreateDebugCamera, 1); //--- Make sure it's called after OnGameStart
	}
}
