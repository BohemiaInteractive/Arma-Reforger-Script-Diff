// Script File

class SCR_DebugShootComponentClass: DebugShootComponentClass
{
};

class SCR_DebugShootComponent : DebugShootComponent
{
	[Attribute("{609E216CBF8D0B68}Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VL.et", UIWidgets.ResourceNamePicker, "Rocket", "et")]
	ResourceName m_rocketResource;
		
	InputManager m_InputManager;
	IEntity m_owner;
	
	override void OnPostInit(IEntity owner)
	{
		
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.FRAME);
		int mask = GetEventMask();		
		m_InputManager = GetGame().GetInputManager();
		m_InputManager.AddActionListener("MouseLeft", EActionTrigger.DOWN, CallbackFunc);	
		m_owner = owner;
	}
	

	void CallbackFunc()
	{
		Resource prefab = Resource.Load(m_rocketResource);
		
		ChimeraGame game = GetGame();

		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;

		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return;
		
		IEntity rocket = GetGame().SpawnEntityPrefab(prefab, game.GetWorld());
		
		//transform the rocket 
		
		vector rot[4];		
		camera.GetWorldTransform(rot);
		rot[3] = vector.Zero;
		
		vector forward = vector.Forward;	
		forward = forward.Multiply3(rot);
		
		vector mat[4];
		camera.GetWorldTransform(mat);
		mat[3] = mat[3] + forward;	
		
		rocket.SetWorldTransform(mat);
		
		Fire(forward, mat, rocket, m_owner);
		
	}
	
	void Cleanup()
	{
		m_InputManager.RemoveActionListener("MouseLeft", EActionTrigger.DOWN, CallbackFunc);	
	}
};