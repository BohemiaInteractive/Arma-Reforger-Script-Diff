[EntityEditorProps(category: "GameLib/Scripted", description:"Script camera")]
class ScriptCameraClass: GenericEntityClass
{

}

class ScriptCamera: GenericEntity
{
	[Attribute("60", UIWidgets.Slider, "Field of view", "0 180 1")]
	float FOV;
	[Attribute("1", UIWidgets.EditBox, "Near plane clip")]
	float NearPlane;
	[Attribute("4000", UIWidgets.EditBox, "Far plane clip")]
	float FarPlane;
	
	[Attribute("1", UIWidgets.ComboBox, "Projection type", "", enumType: CameraType )]
	int Type;
	[Attribute("5", UIWidgets.Slider, "Camera speed", "0 20 1")]
	float Speed;
	[Attribute("1", UIWidgets.CheckBox, "Free Fly", "" )]
	bool FreeFly;
	[Attribute("0", UIWidgets.CheckBox, "Invert vertical", "")]
	bool Inverted;
	[Attribute("0", UIWidgets.Slider, "Camera index", "0 31 1")]
	int Index;
	[Attribute("0", UIWidgets.CheckBox, "Enable preload", "")]
	bool EnablePreload;
	float m_MouseSensitivity = 0.001; // should be somewhere else.
	float m_GamepadSensitivity = 0.2; // should be somewhere else.
	int m_GamepadFreeFly;

	// debug variables
	int m_DbgListSelection = 0;
	ref array<string> m_DbgOptions = {"Perspective", "Orthographic"};

	void ScriptCamera(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(EntityEvent.POSTFRAME);

		BaseWorld world = GetWorld();
		world.SetCameraVerticalFOV(Index, FOV);
		world.SetCameraFarPlane(Index, FarPlane);
		world.SetCameraNearPlane(Index, NearPlane);
		world.SetCameraType(Index, Type);
		m_DbgListSelection = Type - 1;
		world.SetCamera(Index, GetOrigin(), GetYawPitchRoll());
		vector camMat[4];
		GetTransform(camMat);
		world.SetCameraEx(Index, camMat);
		if (EnablePreload)
			world.SchedulePreload(GetOrigin(), 500);
		m_GamepadFreeFly = FreeFly;
	}

	override protected void EOnPostFrame(IEntity owner, float timeSlice) //EntityEvent.FRAME
	{
		g_Game.GetInputManager().ActivateContext("ScriptCameraContext");
		
		if (g_Game.GetInputManager().GetActionTriggered("CamFreeFly"))
		{
			FreeFly = !FreeFly;
		}
		
		if (FreeFly)
		{
			FreeFly(timeSlice);
		}
		else
		{
			vector camMat[4]; // matrix can be set outside the class
			GetWorldTransform(camMat);
			GetWorld().SetCameraEx(Index, camMat);
		}
		
		// DebugInfo();
	}

	protected void FreeFly(float timeSlice)
	{
		vector camPosition = GetOrigin();
		vector angles = GetYawPitchRoll();
		vector camMat[4];
		GetTransform(camMat);	
		InputManager imanager = g_Game.GetInputManager();
		imanager.ActivateContext("ScriptCameraFreeFlyContext");
		
		// get input
		float turnX 		= imanager.GetActionValue("CamTurnRight") * 20.0 * timeSlice;
		float turnY 		= imanager.GetActionValue("CamTurnUp") * 20.0 * timeSlice;
		float turnZ			= imanager.GetActionValue("CamRotate") * 20.0 * timeSlice;
		float moveForward	= imanager.GetActionValue("CamForward");
		float moveRight 	= imanager.GetActionValue("CamRight");
		float moveAscend 	= imanager.GetActionValue("CamAscend");
		float speedDelta   	= imanager.GetActionValue("CamSpeedDelta") * timeSlice;
		bool speedBoostHigh   	= imanager.GetActionTriggered("CamSpeedBoostHigh");
		bool speedBoostLow  = imanager.GetActionTriggered("CamSpeedBoostLow");

		Speed = Math.Clamp(Speed + speedDelta * Speed * 0.25, 0.1, 1000.0);
		
		float finalSpeed = Speed;
		if (speedBoostHigh)
			finalSpeed *= 25;
		else if (speedBoostLow)
			finalSpeed *= 5;

		// rotation
		angles[0] = turnX + angles[0];
		if (Inverted)
			angles[1] = turnY + angles[1];
		else
			angles[1] = -turnY + angles[1];

		angles[2] = turnZ + angles[2];
		
		// movement
		vector move = vector.Zero;
		vector forward = camMat[2];
		vector up = camMat[1];
		vector side = camMat[0];
		
		move += forward * moveForward;
		move += side    * moveRight;
		move += up      * moveAscend;
		
		// ------------		
		camPosition = (move * timeSlice * finalSpeed) + camPosition;
		
		Math3D.AnglesToMatrix(angles, camMat);
		camMat[3] = camPosition;
		SetTransform(camMat);
		GetWorld().SetCameraEx(Index, camMat);
	}

	protected void DebugInfo()
	{
		InputManager imanager = g_Game.GetInputManager();
		DbgUI.Begin(String("Camera #" + Index.ToString()), 0, Index * 300);

		DbgUI.Text(String("Position : " + GetOrigin().ToString()));
		DbgUI.Text(String("Orientation (Y, P, R): " + GetYawPitchRoll().ToString()));
		DbgUI.Text(String("Speed : " + Speed.ToString()));
		DbgUI.Text(String("Mouse sensitivity : " + (2000 - (1 / m_MouseSensitivity)).ToString()));
		DbgUI.Check("Select Free fly", FreeFly);
		DbgUI.List("Camera type", m_DbgListSelection, m_DbgOptions);
		if (m_DbgListSelection + 1 != Type)
		{
			Type = m_DbgListSelection + 1;
			GetWorld().SetCameraType(Index, Type);
		}
		
		float sensitivity = 2000 - (1 / m_MouseSensitivity);
		DbgUI.SliderFloat("Mouse sensitivity", sensitivity, 1, 1999);
		m_MouseSensitivity = 1 / (2000 - sensitivity);
		
		DbgUI.Text("CamTurnRight: " + imanager.GetActionValue("CamTurnRight"));
		DbgUI.Text("CamTurnUp: " + imanager.GetActionValue("CamTurnUp"));
		DbgUI.Text("CamSpeedDelta: " + imanager.GetActionValue("CamSpeedDelta"));
		DbgUI.Text("CamForward: " + imanager.GetActionValue("CamForward"));
		DbgUI.Text("CamRight: " +imanager.GetActionValue("CamRight"));
		DbgUI.Text("CamAscend: " + imanager.GetActionValue("CamAscend"));
		DbgUI.Text("CamSpeedBoostHigh: " + imanager.GetActionTriggered("CamSpeedBoostHigh"));
		DbgUI.Text("CamSpeedBoostLow:" + imanager.GetActionTriggered("CamSpeedBoostLow"));		
		
		DbgUI.End();
	}
}
