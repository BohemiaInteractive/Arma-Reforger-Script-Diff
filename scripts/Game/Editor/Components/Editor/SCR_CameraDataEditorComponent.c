[ComponentEditorProps(category: "GameScripted/Editor", description: "Camera data per session. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CameraDataEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
class SCR_CameraDataEditorComponent : SCR_BaseEditorComponent
{
	protected ref array<ref SCR_ManualCameraComponentSave> m_aSavedData;

	protected CameraBase m_ReturnCamera;
	protected SCR_PlayerController m_ReturnPlayerController;
	protected bool m_bRestoreCameraOnClose;

	//------------------------------------------------------------------------------------------------
	// Set whether this close may restore the camera saved on editor entry
	void SetRestoreCameraOnClose(bool restoreCamera)
	{
		m_bRestoreCameraOnClose = restoreCamera;
	}

	//------------------------------------------------------------------------------------------------
	// Restore the entry camera only while closing editor still owns the view
	[Friend(SCR_CameraEditorComponent)]
	protected bool TryRestoreCamera(SCR_ManualCamera editorCamera)
	{
		if (!m_bRestoreCameraOnClose || !editorCamera || !m_ReturnCamera || m_ReturnCamera.IsDeleted() || PlayerCamera.Cast(m_ReturnCamera))
			return false;

		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager || cameraManager.CurrentCamera() != editorCamera || m_ReturnCamera == editorCamera)
			return false;

		array<CameraBase> cameras = {};
		cameraManager.GetCamerasList(cameras);
		if (!cameras.Contains(m_ReturnCamera))
			return false;

		return cameraManager.SetCamera(m_ReturnCamera);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		ClearReturnCamera();

		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;

		// Manager state survives mode switches and replacement of the editor camera.
		m_ReturnCamera = cameraManager.CurrentCamera();
		m_ReturnPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (m_ReturnPlayerController)
		{
			m_ReturnPlayerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
			m_ReturnPlayerController.m_OnDestroyed.Insert(OnPlayerDestroyed);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		if (from != to)
			m_ReturnCamera = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDestroyed(Instigator killer, IEntity killerEntity)
	{
		m_ReturnCamera = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearReturnCamera()
	{
		if (m_ReturnPlayerController)
		{
			m_ReturnPlayerController.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);
			m_ReturnPlayerController.m_OnDestroyed.Remove(OnPlayerDestroyed);
		}

		m_ReturnPlayerController = null;
		m_ReturnCamera = null;
		m_bRestoreCameraOnClose = false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorClose()
	{
		ClearReturnCamera();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorDelete()
	{
		ClearReturnCamera();
	}
	
	void SaveComponents(SCR_ManualCamera camera)
	{
		if (camera)
		{
			if (!m_aSavedData)
				m_aSavedData = {};
			
			camera.SaveComponents(m_aSavedData);
		}
	}
	void LoadComponents(SCR_ManualCamera camera)
	{
		if (m_aSavedData)
			camera.LoadComponents(m_aSavedData);
	}
	bool IsSave()
	{
		return m_aSavedData != null;
	}
};