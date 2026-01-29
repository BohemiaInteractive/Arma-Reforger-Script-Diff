[EntityEditorProps(category: "GameScripted/Sound", description: "")]
class SCR_HelicopterSoundComponentClass : SCR_VehicleSoundComponentClass
{
}

class SCR_HelicopterSoundComponent : SCR_VehicleSoundComponent
{		 
	// Signal names
	protected const string SPEED_TO_CAMERA_SIGNAL_NAME = "SpeedToCamera";
	protected const string DISTANCE_SIGNAL_NAME = "Distance";	
	protected const string ALTITUDE_AGL = "AltitudeAGL";	
	protected const string MAIN_ROTOR_RPM_SCALED = "MainRotorRPMScaled";	
	protected const string MAIN_ROTOR_HIT_ZONE_NAME = "RotorMain";	
	protected const string DAMAGE_STATE_SIGNAL_NAME = "DamageState";
	protected const string ROTOR_MAIN_DAMAGE_STATE_SIGNAL_NAME = "RotorMainDamageState";

	// Constants
	protected const float UPDATE_TIME = 0.15;
	protected const float WASH_ROTOR_DISTANCE_LIMIT = 100;
	protected const float ALTITUDE_LIMIT = 50;
	protected const float MAIN_ROTOR_RPM_SCALED_THRESHOLD = 0.2;

	// Signal indexes
	protected int m_iDistanceSignalIdx;
	protected int m_iSpeedToCameraSignalIdx;
	protected int m_AltitudeAGLSignalIdx;
	protected int m_MainRotorRPMScaledIdx;

	//
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected float m_fTimer;

	//! Wash rotor audio handle
	protected AudioHandle m_WashRotorAudioHandle = AudioHandle.Invalid;		

	//! Damage state signal
	protected SCR_HelicopterDamageManagerComponent m_HelicopterDamageManagerComponent;

	//! Main rotor damage state
	protected SCR_HitZone m_RotorMainHitZone;

	protected EDamageState m_eRotorMainDamageState;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{	
		super.UpdateSoundJob(owner, timeSlice);
		
		if (!m_SignalsManagerComponent)
			return;
					
		m_fTimer += timeSlice;
		if (m_fTimer < UPDATE_TIME)
			return;
	
		CalculateSpeedToCameraSignal(owner);
		HandleWashRotor(owner);
		m_fTimer = 0;		
	}
		
	//------------------------------------------------------------------------------------------------
	//! Updates SpeedToCamera signal based on owner velocity towards the camera
	//! \param owner owner entity - cannot be null
	protected void CalculateSpeedToCameraSignal(IEntity owner)
	{
		Physics physics = owner.GetPhysics();
		if (!physics)
			return;
		
		vector cameraTransform[4];
		GetGame().GetWorld().GetCurrentCamera(cameraTransform);
		
		vector entityPos = owner.GetOrigin();	
		vector entityVelocity = physics.GetVelocity();
				
		vector directionCameraEntity = (cameraTransform[3] - entityPos).Normalized();		
		float speed = vector.Dot(entityVelocity, directionCameraEntity);
						
		// Set SpeedToCamera signal
		m_SignalsManagerComponent.SetSignalValue(m_iSpeedToCameraSignalIdx, speed);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles wash rotor sound. Triggers the sound, when conditions are met and updates sounds position based on owner's altitude.
	//! \param[in] owner cannot be null
	protected void HandleWashRotor(IEntity owner)
	{
		if (m_eRotorMainDamageState == EDamageState.DESTROYED || m_SignalsManagerComponent.GetSignalValue(m_iDistanceSignalIdx) > WASH_ROTOR_DISTANCE_LIMIT || m_SignalsManagerComponent.GetSignalValue(m_MainRotorRPMScaledIdx) < MAIN_ROTOR_RPM_SCALED_THRESHOLD)
		{
			Terminate(m_WashRotorAudioHandle);
			return;
		}
				
		float altitudeAGL = m_SignalsManagerComponent.GetSignalValue(m_AltitudeAGLSignalIdx);
		if (altitudeAGL > ALTITUDE_LIMIT)
		{
			Terminate(m_WashRotorAudioHandle);
			return;
		}
		
		vector mat[4];
		owner.GetTransform(mat);
		mat[3][1] = mat[3][1] - altitudeAGL;
		
		if (IsFinishedPlaying(m_WashRotorAudioHandle))
			m_WashRotorAudioHandle = SoundEvent(SCR_SoundEvent.SOUND_ROTOR_WASH_LP);
		
		SetSoundTransformation(m_WashRotorAudioHandle, mat);
	}
				
	//------------------------------------------------------------------------------------------------	
	protected void RegisterOnDamageChanged()
	{	
		m_HelicopterDamageManagerComponent = SCR_HelicopterDamageManagerComponent.Cast(GetOwner().FindComponent(SCR_HelicopterDamageManagerComponent));
		if (!m_HelicopterDamageManagerComponent)
			return;
		
		m_HelicopterDamageManagerComponent.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void  UnregisterOnDamageChanged()
	{
		if (m_HelicopterDamageManagerComponent)
			m_HelicopterDamageManagerComponent.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDamageStateChanged(EDamageState state)
	{		
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal(DAMAGE_STATE_SIGNAL_NAME), state);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void RegisterRotorMainOnDamageChanged()
	{
		SCR_DamageManagerComponent hitZoneContainerComponent = SCR_DamageManagerComponent.Cast(GetOwner().FindComponent(SCR_DamageManagerComponent));
		if (!hitZoneContainerComponent)
			return;
								
		HitZone hitZone = hitZoneContainerComponent.GetHitZoneByName(MAIN_ROTOR_HIT_ZONE_NAME);
		m_RotorMainHitZone = SCR_HitZone.Cast(hitZone);
		if (m_RotorMainHitZone)
			m_RotorMainHitZone.GetOnDamageStateChanged().Insert(RotorMainOnStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void UnregisterRotorMainOnDamageChanged()
	{
		SCR_DamageManagerComponent hitZoneContainerComponent = SCR_DamageManagerComponent.Cast(GetOwner().FindComponent(SCR_DamageManagerComponent));
		if (!hitZoneContainerComponent)
			return;
								
		HitZone hitZone = hitZoneContainerComponent.GetHitZoneByName(MAIN_ROTOR_HIT_ZONE_NAME);
		m_RotorMainHitZone = SCR_HitZone.Cast(hitZone);
		if (m_RotorMainHitZone)
			m_RotorMainHitZone.GetOnDamageStateChanged().Remove(RotorMainOnStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RotorMainOnStateChanged()
	{
		m_eRotorMainDamageState = m_RotorMainHitZone.GetDamageState();
		
		if (!m_SignalsManagerComponent)
			return;
		
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal(ROTOR_MAIN_DAMAGE_STATE_SIGNAL_NAME), SCR_HitZoneStateSignal.DamageStateToSignalValue(m_eRotorMainDamageState))
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (!m_SignalsManagerComponent)
			return;
			
		m_iDistanceSignalIdx = m_SignalsManagerComponent.AddOrFindSignal(DISTANCE_SIGNAL_NAME);
		m_iSpeedToCameraSignalIdx = m_SignalsManagerComponent.AddOrFindSignal(SPEED_TO_CAMERA_SIGNAL_NAME);
		m_AltitudeAGLSignalIdx = m_SignalsManagerComponent.AddOrFindSignal(ALTITUDE_AGL);
		m_MainRotorRPMScaledIdx = m_SignalsManagerComponent.AddOrFindSignal(MAIN_ROTOR_RPM_SCALED);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		RegisterOnDamageChanged();
		RegisterRotorMainOnDamageChanged();
	}
				
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_HelicopterSoundComponent()
	{
		UnregisterOnDamageChanged();
		UnregisterRotorMainOnDamageChanged();
	}	
}
