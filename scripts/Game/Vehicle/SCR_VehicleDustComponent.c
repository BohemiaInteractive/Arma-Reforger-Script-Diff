// Spawns dust particle effect that drags with the vehicle during ride

[ComponentEditorProps(category: "GameScripted/Test", description:"SCR_VehicleDust")]
class SCR_VehicleDustClass: ScriptComponentClass
{
};

class SCR_VehicleDust : ScriptComponent
{
	// LIST OF PARTICLE SOURCES
	static string particle_attribute_descriptions = "Select particle effect for driving on this surface type.";
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Grass;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Gravel;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Dirt;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Default;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Wood;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Asphalt;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Snow;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Sand;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Metal;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Concrete;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Soil;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Stone;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Rubber;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Flesh;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Glass;
	[Attribute( "", UIWidgets.ResourceNamePicker, particle_attribute_descriptions )]
	ResourceName m_Plastic;
	
	[Attribute( "", UIWidgets.Coords, "Orientation of the effect relative to the vehicle" )]
	vector 	m_vLocalOrientation; // Local rotation of the particle
	
	[Attribute( "", UIWidgets.Coords, "Position of the effect relative to the object" )]
	vector 	m_vLocalOffset; // Local offset of the particle
	
	[Attribute( "", UIWidgets.EditBox, "Minimal speed (km/h) which triggers the dust effect and its interpolation from 0%" )]
	float	m_fDustStartSpeed;
	
	[Attribute( "", UIWidgets.EditBox, "Speed (km/h) which tops the effect's interpolation at 100% values" )]
	float	m_fDustTopSpeed;
	VehicleWheeledSimulation 	m_VehicleWheeledSimulation;
	VehicleBodyEffectBase		m_Effect;
	SignalsManagerComponent 	m_SignalManagerComp;
	ref map<int, ResourceName> surface_particles = new map<int, ResourceName>();
	int 						m_iCurrentSurfaceID;
	int							m_iSurfaceSignalID = -1;
	
	// Optimization timers
	float 				m_fUpdateDelay; // Desired delay between each particle calculations in seconds. 0 means per frame.
	float 				m_fCurrentUpdateDelay; // The time of current delay
	const float 		TICK_TIME_DELAY_MAX = 2; // Max acceptable delay in seconds
	const float 		TICK_DELAY_RANGE_START = 15; // Starting range at which the delay begins to increase (from 0 to TICK_TIME_DELAY_MAX)
	const float 		TICK_DELAY_RANGE_END = 100; // End range at which delay reaches its maximum (TICK_TIME_DELAY_MAX)
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		// Don't run this component on console app
		if (System.IsConsoleApp())
		{
			Deactivate(owner);
			return;
		}
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}
    
	//------------------------------------------------------------------------------------------------  
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		// Initialize the list of surface types, but only once.
		if (surface_particles.Count() == 0)
			RegisterSurfaceTypes();
		
		// Get data
		GenericEntity generic_entity = GenericEntity.Cast(owner);
		
		if (generic_entity)
		{
			m_SignalManagerComp = SignalsManagerComponent.Cast(generic_entity.FindComponent(SignalsManagerComponent));
			m_VehicleWheeledSimulation = VehicleWheeledSimulation.Cast(generic_entity.FindComponent(VehicleWheeledSimulation));
		}
		
		// Prep eff
		if (!m_Effect)
		{
			m_Effect = VehicleBodyEffectBase.Cast(GetGame().SpawnEntity(VehicleBodyEffectBase, owner.GetWorld()));
			if (m_Effect)
			{
				vector mat[4];
				owner.GetTransform(mat);
				m_Effect.SetTransform(mat);
				owner.AddChild(m_Effect, 0);
			}

		}
	}
	
	// Assigns particle files to surface IDs. This should be called only once
	void RegisterSurfaceTypes()
	{
		// DO NOT CHANGE ORDER OF THESE SURFACE TYPES! THEY ARE DEFINED IN THE ENGINE!
		surface_particles.Insert(0, m_Default);
		surface_particles.Insert(31, m_Grass);
		surface_particles.Insert(21, m_Dirt);
		surface_particles.Insert(25, m_Gravel);
		surface_particles.Insert(51, m_Wood);
		surface_particles.Insert(11, m_Asphalt);
		surface_particles.Insert(41, m_Snow);
		surface_particles.Insert(28, m_Sand);
		surface_particles.Insert(61, m_Metal);
		surface_particles.Insert(12, m_Concrete);
		surface_particles.Insert(23, m_Soil);
		surface_particles.Insert(15, m_Stone);
		surface_particles.Insert(27, m_Stone);
		surface_particles.Insert(72, m_Rubber);
		surface_particles.Insert(91, m_Flesh);
		surface_particles.Insert(71, m_Glass);
		surface_particles.Insert(73, m_Plastic);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fCurrentUpdateDelay += timeSlice;
		
		// Calculate distance between owner and camera
		vector camera_transform[4];
		GetGame().GetWorld().GetCurrentCamera(camera_transform);
		vector camera_world_pos = camera_transform[3];
		vector relative_pos = camera_world_pos - owner.GetOrigin();
		float distance = relative_pos.Length();
		
		// Prolong update interval based on the distance between camera and effect
		if (distance > TICK_DELAY_RANGE_START)
		{
			m_fUpdateDelay = TICK_TIME_DELAY_MAX * ( Math.Clamp( (distance - TICK_DELAY_RANGE_START)  /  TICK_DELAY_RANGE_END , 0, 1) );
		}
		else
		{
			m_fUpdateDelay = 0;
		}
		
		// Check if delay treshold was crossed. This prevents calling intense calculations per frame for vehicles which are far away from camera as they have low priority
		if (m_fCurrentUpdateDelay > m_fUpdateDelay) 
		{
			m_fCurrentUpdateDelay = 0;
			UpdateEffect(owner);
		}
	}
    
	// Called when the effects needs to be updated. This happens per frame if the camera is close to it, but much slower if further away fr optimization purposes.
	void UpdateEffect(IEntity owner)
	{
		Physics physics = owner.GetPhysics();
		
		if (physics  &&  m_SignalManagerComp) // Makre sure that data exists
		{
			float speed = physics.GetVelocity().Length();
			
			// Activate particle effect only within the desired speed
			if (speed >= m_fDustStartSpeed  &&  m_fDustStartSpeed > 0  &&  m_fDustTopSpeed >= m_fDustStartSpeed)
			{
				int previousSurfaceID = m_iCurrentSurfaceID;
				
				// Only create the signal when it's necessary, otherwise use the ID
				if (m_iSurfaceSignalID < 0)
					m_iSurfaceSignalID = m_SignalManagerComp.AddOrFindSignal("surface");
				
				m_iCurrentSurfaceID = m_SignalManagerComp.GetSignalValue(m_iSurfaceSignalID);
				
				if (!surface_particles)
					RegisterSurfaceTypes();
				
				
				// get and set surface material
				ResourceName surface_type = m_Default;
				m_iCurrentSurfaceID = m_iCurrentSurfaceID/100;
				if (m_iCurrentSurfaceID >= 0 && surface_particles.Contains(m_iCurrentSurfaceID))
					surface_type = surface_particles.Get(m_iCurrentSurfaceID);

				
				// Vehicle is moving fast enough to generate dust
				if (m_Effect  &&  previousSurfaceID == m_iCurrentSurfaceID)
				{
					m_Effect.Play();
					m_Effect.UpdateVehicleDustEffect(speed, m_fDustStartSpeed, m_fDustTopSpeed);
				}
				else
				{
					if (m_Effect) 
						m_Effect.StopEmission();
					
					if (surface_type.Length() > 0)
					{
						ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
						spawnParams.Type = VehicleBodyEffectBase;
						Math3D.AnglesToMatrix(m_vLocalOrientation, spawnParams.Transform);
						spawnParams.Transform[3] = m_vLocalOffset;
						spawnParams.Parent = owner;
						spawnParams.UseFrameEvent = true;
						
						m_Effect = VehicleBodyEffectBase.Cast(ParticleEffectEntity.SpawnParticleEffect(surface_type, spawnParams));
						m_Effect.UpdateVehicleDustEffect(speed, m_fDustStartSpeed*3.6, m_fDustTopSpeed);
					}
				}
			}
			// Vehicle is NOT moving fast enough to generate dust
			else if (m_Effect)
			{
				m_Effect.StopEmission(); // Stop dust effect
			}
		}
	}
	
    //------------------------------------------------------------------------------------------------
    void ~SCR_VehicleDust()
    {
		if (m_Effect)
			m_Effect.Stop(); // Clean up
    }
};