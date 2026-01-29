[ComponentEditorProps(category: "GameScripted/Sound", description: "THIS IS THE SCRIPT DESCRIPTION.", color: "0 0 255 255")]
class SCR_ShellSoundComponentClass: ProjectileSoundComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ShellSoundComponent : ProjectileSoundComponent
{
	protected ProjectileMoveComponent m_ProjectileMoveComponent;
	protected SignalsManagerComponent m_SignalsManagerComponent;
	
	protected int m_iSignalIdxSpeed = -1;
	protected int m_iSignalIdxSpeedVertical = -1;
	protected int m_iSignalIdxDistanceToClosestPoint = -1;
	protected int m_iSignalIdxCosAngleProjectileToListener = -1;
	
	//------------------------------------------------------------------------------------------------		
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		// Check component availability
		if (!m_SignalsManagerComponent || !m_ProjectileMoveComponent)
			return;

		// Get projectile velocity and set speed-related signals
		vector velocity = m_ProjectileMoveComponent.GetVelocity();
		float speed = velocity.Length();
		m_SignalsManagerComponent.SetSignalValue(m_iSignalIdxSpeed, speed);
		m_SignalsManagerComponent.SetSignalValue(m_iSignalIdxSpeedVertical, velocity[1]);	
			
		// Calculate a rough "Closest Point to camera position" based on simple linear projectile trajectory estimation
		vector position = owner.GetOrigin();
		velocity.Normalize(); // Turn velocity into direction vector
		vector cameraPosition = GetCameraPosition();
		vector dirProjectileToListener = vector.Direction(position,cameraPosition);
	    vector closestPoint = position + velocity * vector.Dot(dirProjectileToListener, velocity);		
		float distanceToClosestPoint = vector.Distance(closestPoint, position);
		m_SignalsManagerComponent.SetSignalValue(m_iSignalIdxDistanceToClosestPoint, distanceToClosestPoint);
		
		// Get cosinus of angle from projectile velocity to listener
		dirProjectileToListener.Normalize();
		float cosAngleProjectileToListener = vector.Dot(dirProjectileToListener,velocity);
		m_SignalsManagerComponent.SetSignalValue(m_iSignalIdxCosAngleProjectileToListener, cosAngleProjectileToListener);
	}
		
	//------------------------------------------------------------------------------------------------
	private vector GetCameraPosition()
	{		
		CameraBase playerCamera = GetGame().GetCameraManager().CurrentCamera();
		if (!playerCamera)
			return vector.Zero;
		
		return playerCamera.GetOrigin();
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		// Find SignalsManager and BaseMove Components
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		m_ProjectileMoveComponent = ProjectileMoveComponent.Cast(owner.FindComponent(ProjectileMoveComponent));
		
		// Warn for missing components
		if (!m_SignalsManagerComponent)
			Print("Warning: SCR_ShellSoundComponent requires SignalsManagerComponent!");
		
		if (!m_ProjectileMoveComponent)
			Print("Warning: SCR_ShellSoundComponent requires ProjectileMoveComponent (or something inheriting from it)!");
		
		// Get and set indices of signals
		m_iSignalIdxSpeed 							= m_SignalsManagerComponent.AddOrFindSignal("Speed");
		m_iSignalIdxSpeedVertical 					= m_SignalsManagerComponent.AddOrFindSignal("SpeedVertical");	
		m_iSignalIdxDistanceToClosestPoint 			= m_SignalsManagerComponent.AddOrFindSignal("DistanceToClosestPoint", 1000);
		m_iSignalIdxCosAngleProjectileToListener 	= m_SignalsManagerComponent.AddOrFindSignal("CosAngleProjectileToListener");
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ShellSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);	
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_ShellSoundComponent()
	{
	}
};