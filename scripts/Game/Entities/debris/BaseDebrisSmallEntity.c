#define DEBUG
#define ENABLE_BASE_DESTRUCTION
[EntityEditorProps(category: "GameScripted/Debris", description: "Entity used to represent small chunks of debris. Automatically managed.", dynamicBox: true)]
class SCR_BaseDebrisSmallEntityClass: GenericEntityClass
{
}

//! Entity used to represent small debris in the world. Is managed automatically and will never
//! allow the user to exceed the specified maximum amount. If new spawn is requested, priority
//! is evaluated. If the new debris has higher priority than any of the debris spawned before,
//! it will be replaced by the new one.
class SCR_BaseDebrisSmallEntity : GenericEntity
{
#ifdef ENABLE_BASE_DESTRUCTION	
	//! Whether this debris has reached end of its lifetime and should be deleted
	bool m_bDelete = false;

	//! The lifetime in seconds. 
	float m_fLifeTime;

	//! Entity age in seconds. After this time is bigger thatn m_fLifeTime, debris will despawn.
	float m_fAgeTime;
	
	//! Minimal distance from last played sound
	protected static const float MINIMAL_DISTANCE_SQ = 0.25;

	//! Minimum entity lifetime to play sound
	protected static const float MINIMAL_AGE = 0.25;
	
	//! Sound triggered is kinetic energy is above the threshold
	protected static const float KINETIC_ENERGY_THRESHOLD = 15;
		
#ifdef ENABLE_DIAG
	//! Impact contact velocity change
	float m_fdVelocity;

	//! World space text object
	protected ref DebugTextWorldSpace m_Text;

	//! Peak value
	protected float m_fdVelocityPeak;

	//! Peak value age
	protected float m_fdVelocityPeakAgeTime;
#endif // ENABLE_DIAG
		
	//------------------------------------------------------------------------------------------------
	//! Get whether game is in play state or not.
	//! \return true if play mode, false otherwise
	protected bool IsGamePlaying()
	{		
		return GetGame().GetWorldEntity() != null;
	}
			
	//------------------------------------------------------------------------------------------------
	protected static float GetDistanceToCamera(BaseWorld world, vector position)
	{
		vector cameraMat[4];
		world.GetCurrentCamera(cameraMat);
		return vector.Distance(cameraMat[3], position);
	}
	
	//------------------------------------------------------------------------------------------------	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Get debris age
		m_fAgeTime += timeSlice;
		
		// Delete this debris. (automatically unregisters)
		if (m_fAgeTime >= m_fLifeTime)
			DeleteDebris();
				
		// If debris should be deleted then delete it.
		if (m_bDelete)
		{
			Physics entityPhysics = GetPhysics();
			if (entityPhysics)
				entityPhysics.Destroy();
			
			delete this;
		}
	}
			
	//------------------------------------------------------------------------------------------------
#ifdef ENABLE_DIAG
	void SoundDebugOnFrame(float soundThreshold)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_MPDESTRUCTION_SHOW_IMPULSEVALUES))
		{
			// Get center of entity
			vector minsDebug;
			vector maxsDebug;
			GetWorldBounds(minsDebug, maxsDebug);			
			const vector centerDebug = vector.Lerp(minsDebug, maxsDebug, 0.5);
			
			// Hold peak value for 1s
			if (m_fAgeTime - m_fdVelocityPeakAgeTime > 1 || m_fdVelocity > m_fdVelocityPeak)
			{
				m_Text = DebugTextWorldSpace.Create(GetWorld(), m_fdVelocity.ToString(1, 2) + "/" + soundThreshold.ToString(1, 2) + "/" + GetPhysics().GetMass().ToString(), DebugTextFlags.FACE_CAMERA, centerDebug[0], centerDebug[1], centerDebug[2], 20);
				m_fdVelocityPeak = m_fdVelocity;
				m_fdVelocityPeakAgeTime = m_fAgeTime;
			}
		}
	}
	
	void SoundDebugPlaySound(float soundThreshold, float dVelocity, vector pos)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_MPDESTRUCTION_SHOW_IMPULSEVALUES))
		{
			m_Text = DebugTextWorldSpace.Create(GetWorld(), dVelocity.ToString(1, 2) + "/" + soundThreshold.ToString(1, 2) + "/" + GetPhysics().GetMass().ToString(), DebugTextFlags.FACE_CAMERA, pos[0], pos[1], pos[2], 20, COLOR_BLUE);
			m_fdVelocityPeakAgeTime = m_fAgeTime + 1;
		}
	}

#endif // ENABLE_DIAG
	
	//------------------------------------------------------------------------------------------------
	//! Delete debris - unregisters it from the list and makes it scale down and delete.
	void DeleteDebris()
	{
		m_bDelete = true;
		ClearEventMask(EntityEvent.CONTACT);
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_BaseDebrisSmallEntity(IEntitySource src, IEntity parent)
	{	
		SetEventMask(EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);		
	}
	
#endif // ENABLE_BASE_DESTRUCTION
}
