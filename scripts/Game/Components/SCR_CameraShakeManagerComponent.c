[EntityEditorProps(category: "GameScripted/Camera", description: "Manager for camera shake.")]
class SCR_CameraShakeManagerComponentClass : ScriptComponentClass
{
}

//! This manager allows adding camera shake that is updated automatically and can be applied to any transform via provided API.
class SCR_CameraShakeManagerComponent : ScriptComponent
{
	//! Immediate shake transformation matrix
	protected vector m_mShakeMatrix[4];
	protected float m_fFovScale;

	//! Array of pre-cached generic shakes
	const int CAMERA_SHAKE_INSTANCES = 16;
	ref SCR_NoisyCameraShakeProgress m_aShakeInstances[CAMERA_SHAKE_INSTANCES];
	
	//! Array of custom shakes
	ref array<ref SCR_BaseCameraShakeProgress> m_aAdditionalInstances = {};

	// Current instance of manager or null if none
	private static SCR_CameraShakeManagerComponent s_Instance;

	//------------------------------------------------------------------------------------------------
	//! Adds camera shake.
	//! \param[in] linearMagnitude Magnitude of linear (positional change) shake
	//! \param[in] angularMagnitude Magnitude of angular (rotational change) shake
	//! \param[in] inTime Blend in time duration before shake reached peak value (in seconds)
	//! \param[in] sustainTime Time duration for which the shake stays at peak value (in seconds)
	//! \param[in] outTime Blend out time duration before shake clears out from peak value (in seconds)
	static void AddCameraShake(float linearMagnitude = 1.0, float angularMagnitude = 1.0, float inTime = 0.01, float sustainTime = 0.1, float outTime = 0.24)
	{
		if (s_Instance)
			s_Instance.DoAddCameraShake(linearMagnitude, angularMagnitude, inTime, sustainTime, outTime);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds camera shake.
	//! \param[in] linearMagnitude Magnitude of linear (positional change) shake
	//! \param[in] angularMagnitude Magnitude of angular (rotational change) shake
	//! \param[in] inTime Blend in time duration before shake reached peak value (in seconds)
	//! \param[in] sustainTime Time duration for which the shake stays at peak value (in seconds)
	//! \param[in] outTime Blend out time duration before shake clears out from peak value (in seconds)
	void DoAddCameraShake(float linearMagnitude = 1.0, float angularMagnitude = 1.0, float inTime = 0.01, float sustainTime = 0.1, float outTime = 0.24)
	{
		for (int i = 0; i < CAMERA_SHAKE_INSTANCES; i++)
		{
			// Find first free instance that can be used
			SCR_NoisyCameraShakeProgress shake = m_aShakeInstances[i];
			if (shake.IsFinished())
			{
				shake.SetParams(linearMagnitude, angularMagnitude, inTime, sustainTime, outTime);
				shake.Start();
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds custom scripted camera shake that will be progressed, applied and then withdrawn from the manager.
	//! \param[in] instance The shake to add
	static void AddCameraShake(SCR_BaseCameraShakeProgress instance)
	{
		if (s_Instance)
			s_Instance.DoAddCameraShake(instance);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds custom scripted camera shake that will be progressed, applied and then withdrawn from the manager.
	//! \param[in] instance The shake to add
	void DoAddCameraShake(SCR_BaseCameraShakeProgress instance)
	{
		if (!instance)
		{
			Print("Invalid camera shake instance passed to manager!", LogLevel.ERROR);
			return;
		}
		
		m_aAdditionalInstances.Insert(instance);
		instance.Start();
	}

	//------------------------------------------------------------------------------------------------
	//! Applies currently calculated shake to desired camera.
	static void ApplyCameraShake(inout vector matrix[4], inout float fov)
	{
		if (s_Instance)
			s_Instance.DoApplyCameraShake(matrix, fov);
	}

	//------------------------------------------------------------------------------------------------
	//! Applies currently calculated shake to desired camera.
	//! \param[in,out] matrix
	//! \param[in,out] fov
	void DoApplyCameraShake(inout vector matrix[4], inout float fov)
	{
		#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_ADDITIONAL_CAMERASHAKE_DISABLE))
			return;
		#endif

		Math3D.MatrixMultiply4( matrix, m_mShakeMatrix, matrix );
		fov *= m_fFovScale;
	}

	//------------------------------------------------------------------------------------------------
	//! Clears all existing camera shake progress(es).
	static void ClearCameraShake()
	{
		if (s_Instance)
			s_Instance.DoClearCameraShake();
	}

	//------------------------------------------------------------------------------------------------
	//! Clears all existing camera shake progress(es).
	void DoClearCameraShake()
	{
		for (int i = 0; i < CAMERA_SHAKE_INSTANCES; i++)
		{
			SCR_BaseCameraShakeProgress shake = m_aShakeInstances[i];
			if (!shake.IsFinished())
				shake.Clear();
		}
		
		m_aAdditionalInstances.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		Math3D.MatrixIdentity4(m_mShakeMatrix);
		
		// Skip the initialization of shake manager on headless
		// clients and dedicated server, as the visual can be ommitted completely.
		if (System.IsConsoleApp())
			return;
		
		s_Instance = this;
		SetEventMask(owner, EntityEvent.FRAME);
		GenericEntity.Cast(owner).Activate();
		
		// Pre-cache instances
		for (int i = 0; i < CAMERA_SHAKE_INSTANCES; i++)
		{
			m_aShakeInstances[i] = new SCR_NoisyCameraShakeProgress();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Clear out matrix and FOV
		Math3D.MatrixIdentity4(m_mShakeMatrix);
		m_fFovScale = 1.0;

		// Update generic shakes
		for (int i = 0; i < CAMERA_SHAKE_INSTANCES; i++)
		{
			SCR_BaseCameraShakeProgress shake = m_aShakeInstances[i];
			if (!shake.IsFinished())
			{
				shake.Update(owner, timeSlice);
				shake.Apply(m_mShakeMatrix, m_fFovScale);
			}
		}
		
		// Update custom instances
		for (int i = 0; i < m_aAdditionalInstances.Count();)
		{
			SCR_BaseCameraShakeProgress shake = m_aAdditionalInstances[i];
			if (!shake || shake.IsFinished())
			{
				m_aAdditionalInstances.Remove(i);
				continue;
			}
			
			++i;
			
			shake.Update(owner, timeSlice);
			shake.Apply(m_mShakeMatrix, m_fFovScale);
		} 

		super.EOnFrame(owner, timeSlice);

		// Draw diag window
		#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_CAMERASHAKE_TEST_WINDOW))
		{
			DbgUI.Begin("Camera Shake");
			{
				float linear = 1, angular = 1, inTime = 0.01, sustainTime = 0.1, outTime = 1.0;
				DbgUI.Text("New shake: ");
				DbgUI.InputFloat("Linear: ", linear);
				DbgUI.InputFloat("Angular: ", angular);
				DbgUI.InputFloat("InTime: ", inTime);
				DbgUI.InputFloat("SustainTime: ", sustainTime);
				DbgUI.InputFloat("OutTime: ", outTime);

				if (DbgUI.Button("Add new shake"))
				{
					AddCameraShake(linear, angular, inTime, sustainTime, outTime);
				}
				
				if (DbgUI.Button("Add custom shake"))
				{
					// Just for testing purposes
					SCR_NoisyCameraShakeProgress progress = new SCR_NoisyCameraShakeProgress();
					progress.SetParams(linear, angular, inTime, sustainTime, outTime);
					
					AddCameraShake(progress);
				}

				DbgUI.Spacer(16);

				if (DbgUI.Button("Clear shake"))
				{
					ClearCameraShake();
				}
			}
			DbgUI.End();
		}
		#endif
	}
}
