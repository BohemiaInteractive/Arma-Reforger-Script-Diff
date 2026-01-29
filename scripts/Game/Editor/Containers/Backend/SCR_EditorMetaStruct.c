/*!
@ingroup Editor_Containers_Backend

Saved data for editor-specific information, e.g., camera position
*/
[Obsolete("Only used for backwards compatiblity for GM saves. Will be removed entirely.")]
class SCR_EditorMetaStruct: JsonApiStruct
{
	protected float pX; //--- Camera pos X
	protected float pY; //--- Camera pos Y
	protected float pZ; //--- Camera pos Z
	protected float qX; //--- Camera quaternion X
	protected float qY; //--- Camera quaternion Y
	protected float qZ; //--- Camera quaternion Z
	protected float qW; //--- Camera quaternion W
	
	void Serialize()
	{
		SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
		if (camera)
		{
			vector transform[4];
			camera.GetWorldTransform(transform);
			
			pX = transform[3][0];
			pY = transform[3][1];
			pZ = transform[3][2];
			
			float quat[4];
			Math3D.MatrixToQuat(transform, quat);
			qX = quat[0];
			qY = quat[1];
			qZ = quat[2];
			qW = quat[3];
		}
	}
	void Deserialize()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			//--- Editor manager exists, load camera directly
			OnEditorManagerCreated(editorManager);
		}
		else
		{
			//--- Editor manager does not exist yet (e.g., on mission start), wait for it to be created
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
				core.Event_OnEditorManagerInitOwner.Insert(OnEditorManagerCreated);
		}
	}
	void ClearCache()
	{
		pX = 0;
		pY = 0;
		pZ = 0;
		qX = 0;
		qY = 0;
		qZ = 0;
		qW = 0;
	}
	protected void OnEditorManagerCreated(SCR_EditorManagerEntity editorManager)
	{
		//--- Wait one frame for editor modes to be initialized
		GetGame().GetCallqueue().CallLater(OnEditorManagerCreatedDelayed, 1, false);
	}	
	protected void OnEditorManagerCreatedDelayed()
	{
		SCR_CameraEditorComponent editorCameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		if (!editorCameraManager)
			return;
		
		SCR_ManualCamera camera = editorCameraManager.GetCamera();
		if (camera)
			//--- Camera exists, load it directly
			OnCameraCreated(camera);
		else
			//--- Camera does not exist (e.g., editor not opened yet), wait for it to be created
			editorCameraManager.GetOnCameraCreate().Insert(OnCameraCreated);
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
			core.Event_OnEditorManagerInitOwner.Remove(OnEditorManagerCreated);
	}
	protected void OnCameraCreated(SCR_ManualCamera camera)
	{
		vector transform[4];
		transform[3] = Vector(pX, pY, pZ);
		
		if (transform[3] != vector.Zero)
		{
			float quat[4];
			quat[0] = qX;
			quat[1] = qY;
			quat[2] = qZ;
			quat[3] = qW;
			Math3D.QuatToMatrix(quat, transform);
			
			camera.SetTransform(transform);
		}
		
		SCR_CameraEditorComponent editorCameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		if (editorCameraManager)
			editorCameraManager.GetOnCameraCreate().Remove(OnCameraCreated);
	}
	void Log()
	{
		float quat[4];
		quat[0] = qX;
		quat[1] = qY;
		quat[2] = qZ;
		quat[3] = qW;
		vector angles = Math3D.QuatToAngles(quat);
		
		PrintFormat("  SCR_EditorMetaStruct: cam pos: %1 | cam ang: %2", Vector(pX, pY, pZ), angles);
	}
	void SCR_EditorMetaStruct()
	{
		RegV("pX");
		RegV("pY");
		RegV("pZ");
		RegV("qX");
		RegV("qY");
		RegV("qZ");
		RegV("qW");
	}
}