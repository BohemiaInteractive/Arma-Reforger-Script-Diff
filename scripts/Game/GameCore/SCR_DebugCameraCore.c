//! @ingroup GameCore

//! Debug manual camera
[BaseContainerProps(configRoot: true)]
class SCR_DebugCameraCore : SCR_GameCoreBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "SCR_ManualCamera prefab", "et")]
	private ResourceName m_CameraPrefab;

	private CameraManager m_CameraManager;
	private IEntity m_Camera;

	//------------------------------------------------------------------------------------------------
	//! Toggle camera, i.e., close it when it's opened and vice versa.
	void ToggleCamera()
	{
		if (m_Camera)
			DeleteCamera();
		else
			CreateCamera();
	}

	//------------------------------------------------------------------------------------------------
	//! Create debug camera with given transformation.
	//! \param transform Initial camera transformation
	void CreateCamera(vector transform[4])
	{
		if (m_Camera)
			return;

		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.Transform = transform;
		m_Camera = GetGame().SpawnEntityPrefab(Resource.Load(m_CameraPrefab), GetGame().GetWorld(), spawnParams);

		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_FREE, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Create debug camera on position of the current camera.
	void CreateCamera()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		vector transform[4];
		world.GetCurrentCamera(transform);
		CreateCamera(transform);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete debug camera.
	void DeleteCamera()
	{
		if (!m_Camera)
			return;

		delete m_Camera;

		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_FREE, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Get camera entity.
	//! \return Camera entity
	IEntity GetCamera()
	{
		return m_Camera;
	}

	//------------------------------------------------------------------------------------------------
	//! Get camera entity.
	//! \return Camera entity
	static IEntity GetCameraInstance()
	{
		SCR_DebugCameraCore core = SCR_DebugCameraCore.Cast(SCR_DebugCameraCore.GetInstance(SCR_DebugCameraCore));
		if (core)
			return core.GetCamera();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanCreate()
	{
		if (m_CameraPrefab.IsEmpty())
		{
			Print("Cannot initialize SCR_DebugCameraCore, m_CameraPrefab is empty!", LogLevel.ERROR);
			return false;
		}
		typename type = SCR_BaseContainerTools.GetContainerClassName(m_CameraPrefab).ToType();
		if (!type.IsInherited(SCR_ManualCamera))
		{
			Print(string.Format("Cannot initialize SCR_DebugCameraCore, prefab '%1' is %2, not SCR_ManualCamera!", m_CameraPrefab.GetPath(), type), LogLevel.ERROR);
			return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_FREE))
		{
			//--- Check if in-game editor is opened
			if (SCR_EditorManagerEntity.IsOpenedInstance())
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_FREE, 0);
				Print("Attempting to start debug camera when the editor is opened. There is no need for that.", LogLevel.NORMAL);
				return;
			}

			//--- Create new camera
			CreateCamera();

			//--- Set camera as current if some other camera stole focus
			if (m_CameraManager && m_CameraManager.CurrentCamera() != m_Camera)
			{
				m_CameraManager.SetCamera(CameraBase.Cast(m_Camera));
			}
		}
		else
		{
			DeleteCamera();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameStart()
	{
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_FREE, "lctrl+lalt+c", "Debug manual camera", "Cheats");
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_FREE, 0);

		m_CameraManager = GetGame().GetCameraManager();

#ifdef PLATFORM_WINDOWS
		//--- Delete obsolete settings file. ToDo: Remove
		FileIO.DeleteFile("$profile:.EditorSettings.conf");
#endif
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_FREE, 0);
		delete m_Camera;
	}
}
