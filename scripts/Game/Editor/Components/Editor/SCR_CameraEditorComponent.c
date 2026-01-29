[ComponentEditorProps(category: "GameScripted/Editor", description: "Camera for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CameraEditorComponentClass: SCR_BaseEditorComponentClass
{
};

//~ Script Invokers
void SCR_CameraEditorComponent_OnCameraCreated(SCR_ManualCamera manualCamera);
typedef func SCR_CameraEditorComponent_OnCameraCreated;


/** @ingroup Editor_Components
*/
class SCR_CameraEditorComponent : SCR_BaseEditorComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab of class SCR_ManualCamera", "et", category: "Camera")]
	protected ResourceName m_CameraPrefab;
	
	[Attribute(defvalue: "1", category: "Init", desc: "Should the camera move above player when it initializes?")]
	protected bool m_bMoveUpOnInit;
	
	[Attribute(defvalue: "10", category: "Init", desc: "Initial camera distance.")]
	protected float m_fDistance;
	
	[Attribute(defvalue: "0 50 0")]
	protected vector m_vDefaultOffset;
	
	[Attribute(defvalue: "0 -45 0")]
	protected vector m_vDefaultAngle;
	
	protected CameraManager m_CameraManager;
	protected SCR_ManualCamera m_Camera;
	protected SCR_CameraDataEditorComponent m_CameraData;
	protected vector m_vInitCameraTransform[4];
	protected vector m_vPreActivateCameraTransform[4];
	protected IEntity m_PreActivateControlledEntity;
	protected bool m_bIsReplacingCamera;	
	protected ref ScriptInvokerBase<SCR_CameraEditorComponent_OnCameraCreated> m_OnCameraCreate = new ScriptInvokerBase<SCR_CameraEditorComponent_OnCameraCreated>();
	private SCR_EditorManagerEntity editorManagerEntity;
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public Functions
	/*!
	Get the camera entity
	\return Camera
	*/
	SCR_ManualCamera GetCamera()
	{
		return m_Camera;
	}
	/*!
	Get the camera entity
	\param[out] camera Variable to be filled with the camera entity
	\return True if the camera exists
	*/
	bool GetCamera(out SCR_ManualCamera camera)
	{
		if (!m_Camera) return false;
		camera = m_Camera;
		return true;
	}
	/*!
	Get camera entity instance
	\return Camera
	*/
	static SCR_ManualCamera GetCameraInstance()
	{
		SCR_CameraEditorComponent cameraManager = SCR_CameraEditorComponent.Cast(GetInstance(SCR_CameraEditorComponent));
		if (cameraManager) return cameraManager.GetCamera();
		return null;
	}
	/*!
	Get invoker called when new manual camera is created.
	\return Script invoker
	*/
	ScriptInvokerBase<SCR_CameraEditorComponent_OnCameraCreated> GetOnCameraCreate()
	{
		return m_OnCameraCreate;
	}
	/*!
	Set initial camera transformation.
	To be used before camera is created.
	\param transform Tranaformation matrix
	*/
	void SetInitTransform(vector transform[4])
	{
		m_vInitCameraTransform = transform;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Protected Functions
	protected void InitCameraTransform()
	{
		if (!m_CameraManager)
			return;
		
		//--- If a camera exists, base the view on it. If there is no camera, use terrain default.
		CameraBase currentCamera = m_CameraManager.CurrentCamera();
		if (!currentCamera)
		{
			//--- Use one of pre-defined defaults (if available)
			SCR_EditorCameraDefaultsComponent cameraDefaults = SCR_EditorCameraDefaultsComponent.GetInstance();
			if (cameraDefaults)
			{
				if (cameraDefaults.GetRandomPosition(m_vInitCameraTransform))
					return;
			}
			
			//--- Use world center
			BaseWorld world = GetGame().GetWorld();
			if (!world) return;
			
			vector mins, maxs;
			world.GetBoundBox(mins, maxs);
			vector center = mins + (maxs - mins) / 2;
			center[1] = world.GetSurfaceY(center[0], center[2]);
			center += m_vDefaultOffset;
			m_vInitCameraTransform[3] = center;
			Math3D.AnglesToMatrix(m_vDefaultAngle, m_vInitCameraTransform);
			return;
		}
		
		if (m_PreActivateControlledEntity && PlayerCamera.Cast(currentCamera) && !currentCamera.GetParent())
			return; // Wait for player camera to be assigned to inital entity before grabbing the transform
		
		currentCamera.GetWorldTransform(m_vInitCameraTransform);		
	}

	protected void TryCreateCamera()
	{
		if ((!m_CameraData || !m_CameraData.IsSave()) && m_vInitCameraTransform[3] == vector.Zero)
		{
			InitCameraTransform();
		}
		else
		{
			CreateCamera();
		}
	}
	protected bool TryForceCamera()
	{
		if (!m_Camera || !m_CameraManager)
			return false;
		
		if (m_Camera != m_CameraManager.CurrentCamera())
		{
			array<CameraBase> cameras = {};
			m_CameraManager.GetCamerasList(cameras);
			if (cameras.Contains(m_Camera))
				m_CameraManager.SetCamera(m_Camera);
		}
		
		return true;
	}
	protected void ReplaceCamera()
	{
		m_bIsReplacingCamera = true;
		if (m_CameraData)
			m_CameraData.SaveComponents(m_Camera);
		CreateCamera();
	}
	protected void CreateCamera()
	{
		delete m_Camera;
		
		if (GetOwner().IsDeleted())
			return;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Transform = m_vInitCameraTransform;
		
		ResourceName prefab = GetCameraPrefab();
		if (prefab.IsEmpty())
		{
			m_Camera = SCR_ManualCamera.Cast(GetGame().SpawnEntity(SCR_ManualCamera, GetGame().GetWorld(), spawnParams));
		}
		else
		{
			m_Camera = SCR_ManualCamera.Cast(GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams));
		}
		
		m_Camera.GetOnCameraDeactivate().Insert(OnCameraDectivate);
		Math3D.MatrixIdentity4(m_vInitCameraTransform);
		
		if (m_CameraData)
			m_CameraData.LoadComponents(m_Camera);
		
		m_OnCameraCreate.Invoke(m_Camera);
		
		//--- When opening the editor, move the camera above player
		if (m_bMoveUpOnInit && !m_bIsReplacingCamera)
		{
			if (m_PreActivateControlledEntity)
			{
				SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(m_Camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
				if (teleportComponent)
				{
					if (m_vPreActivateCameraTransform[3] == vector.Zero)
						m_Camera.GetWorldTransform(m_vPreActivateCameraTransform);
					
					//--- Reset roll
					vector angles = Math3D.MatrixToAngles(m_vPreActivateCameraTransform);
					angles[2] = 0;
					Math3D.AnglesToMatrix(angles, m_vPreActivateCameraTransform);
					
					m_Camera.SetTransform(m_vPreActivateCameraTransform);
					bool disableInterruption = !GetGame().GetInputManager().IsUsingMouseAndKeyboard();
					teleportComponent.TeleportCamera(m_vPreActivateCameraTransform[3], true, true, true, disableInterruption, m_fDistance, true);
				}
			}
		}
		m_bIsReplacingCamera = false;
		m_PreActivateControlledEntity = null;
	}
	protected void OnCameraDectivate()
	{
		//--- Force camera
		if (m_CameraManager && m_Camera) m_CameraManager.SetCamera(m_Camera);
	}
	protected ResourceName GetCameraPrefab()
	{
		return m_CameraPrefab;
	}

	SCR_EditorManagerEntity GetEditorManagerEntity()
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.Cast(GetOwner().GetParent());		
		//NOTE @JK: Maybe I could do SCR_EditorManagerEntity.GetInstance() but I'm not 100% sure if it would be the right thing to do.
		return editorManagerEntity;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	override void ResetEditorComponent()
	{
		if (m_Camera)
			m_Camera.ResetComponents();
	}
	override protected void EOnEditorInit()
	{
		m_CameraManager = GetGame().GetCameraManager();
		m_CameraData = SCR_CameraDataEditorComponent.Cast(SCR_CameraDataEditorComponent.GetInstance(SCR_CameraDataEditorComponent, true));

		SCR_EditorManagerEntity editorManagerEntity = GetEditorManagerEntity();
		if (editorManagerEntity)
			editorManagerEntity.EnableCameraNwkSimulation(false);
	}
	override protected void EOnEditorPreActivate()
	{
		if (!m_CameraManager)
			return;
		
		CameraBase currentCamera = m_CameraManager.CurrentCamera();
		m_bIsReplacingCamera = currentCamera && currentCamera.IsInherited(SCR_ManualCamera);
		m_PreActivateControlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		GetGame().GetWorld().GetCurrentCamera(m_vPreActivateCameraTransform);
	}
	override protected void EOnEditorPostActivate()
	{
		SetEventMask(GetOwner(), EntityEvent.FRAME);
		
		SCR_EditorManagerCore editorCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (editorCore)
			editorManagerEntity = editorCore.GetEditorManager(GetGame().GetPlayerController().GetPlayerId());
		
		SCR_EditorManagerEntity editorManagerEntity = GetEditorManagerEntity();
		if (editorManagerEntity)
			editorManagerEntity.EnableCameraNwkSimulation(true);

		//TryCreateCamera(); //--- Don't try to create camera just yet, wait for GUI to initialize in its own EOnEditorPostActivate
	}
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Tell server where camera is located by moving the EditorManagerEntity
		SCR_EditorManagerEntity editorManagerEntity = GetEditorManagerEntity();		
		if(!editorManagerEntity)
			return;

		//if editor isn't open we shouldn't worry about it
		if(!editorManagerEntity.IsOpened())
			return;
	
		//if(editorManagerEntity)
		if (!m_Camera)
			TryCreateCamera();
		
		TryForceCamera();
		
		if (!m_Camera)
			return;
		
		vector mat[4];
		m_Camera.GetWorldTransform(mat);
		editorManagerEntity.SetWorldTransform(mat);
	}
	override protected void EOnEditorDeactivate()
	{
		if (m_CameraData)
			m_CameraData.SaveComponents(m_Camera);
		
		if (m_Camera)
			m_Camera.Terminate();

		SCR_EditorManagerEntity editorManagerEntity = GetEditorManagerEntity();
		if (editorManagerEntity)
			editorManagerEntity.EnableCameraNwkSimulation(false);
	}
	override protected void EOnEditorPostDeactivate()
	{
		if (GetOwner())
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
		
		if (m_Camera)
		{
			m_Camera.TrySwitchToControlledEntityCamera();
			delete m_Camera;
		}
	}
};