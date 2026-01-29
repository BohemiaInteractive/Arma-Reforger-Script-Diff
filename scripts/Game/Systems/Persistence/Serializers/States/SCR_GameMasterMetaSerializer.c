//! Dummy state proxy to access game master data
class SCR_GameMasterMetaData : PersistentState
{
}

class SCR_GameMasterMetaDataSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_GameMasterMetaData;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		if (System.IsConsoleApp())
			return ESerializeResult.DEFAULT; // Do not save on any headless host, no camera there.

		vector camPos, camYPR;
		const SCR_ManualCamera editorCamera = SCR_CameraEditorComponent.GetCameraInstance();
		if (editorCamera)
		{
			camPos = editorCamera.GetOrigin();
			camYPR = editorCamera.GetYawPitchRoll();
		}

		if (camPos == vector.Zero && camYPR == vector.Zero)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(camPos, vector.Zero);
		context.WriteDefault(camYPR, vector.Zero);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		int version;
		context.Read(version);

		SCR_GameMasterMetaDataLoader loader();
		context.ReadValueDefault("camPos", loader.m_vPosition, Vector(6, 6, 6));
		context.ReadValueDefault("camYPR", loader.m_vYawPitchRoll, Vector(9, 9, 9));
		loader.Apply();

		return true;
	}
}

class SCR_GameMasterMetaDataLoader
{
	protected static ref SCR_GameMasterMetaDataLoader s_Self;

	vector m_vPosition;
	vector m_vYawPitchRoll;

	//------------------------------------------------------------------------------------------------
	void Apply()
	{
		// Ensure the self ref is cleared during system exit
		SCR_PersistenceSystem.GetScriptedInstance().GetOnStateChanged().Insert(OnShutdown);
		s_Self = this;

		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			//--- Editor manager exists, load camera directly
			OnEditorManagerCreated(editorManager);
			return;
		}

		//--- Editor manager does not exist yet (e.g., on mission start), wait for it to be created
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
			core.Event_OnEditorManagerInitOwner.Insert(OnEditorManagerCreated);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnShutdown(EPersistenceSystemState oldState, EPersistenceSystemState newState)
	{
		if (newState == EPersistenceSystemState.SHUTDOWN)
			s_Self = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorManagerCreated(SCR_EditorManagerEntity editorManager)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
			core.Event_OnEditorManagerInitOwner.Remove(OnEditorManagerCreated);

		//--- Wait one frame for editor modes to be initialized
		GetGame().GetCallqueue().Call(OnEditorManagerCreatedDelayed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorManagerCreatedDelayed()
	{
		SCR_CameraEditorComponent editorCameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		if (!editorCameraManager)
		{
			s_Self = null;
			return;
		}

		SCR_ManualCamera camera = editorCameraManager.GetCamera();
		if (camera)
		{
			//--- Camera exists, load it directly
			OnCameraCreated(camera);
			return;
		}

		//--- Camera does not exist (e.g., editor not opened yet), wait for it to be created
		editorCameraManager.GetOnCameraCreate().Insert(OnCameraCreated);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCameraCreated(SCR_ManualCamera camera)
	{
		SCR_CameraEditorComponent editorCameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		editorCameraManager.GetOnCameraCreate().Remove(OnCameraCreated);

		s_Self = null;
		vector transform[4];
		Math3D.AnglesToMatrix(m_vYawPitchRoll, transform);
		transform[3] = m_vPosition;
		camera.SetTransform(transform);
	}
}
