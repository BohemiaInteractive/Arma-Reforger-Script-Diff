//------------------------------------------------------------------------------------------------
//! Compass type 
enum SCR_ECompassType
{
	SY183,		// US
	ADRIANOV	// Soviet
}

//! Prefab data class for compass component
[EntityEditorProps(category: "GameScripted/Gadgets", description: "Compass", color: "0 0 255 255")]
class SCR_CompassComponentClass : SCR_GadgetComponentClass
{
	[Attribute(defvalue: "0", UIWidgets.ComboBox, "Set compass type", "", ParamEnumArray.FromEnum(SCR_ECompassType), category: "Compass")]
	int m_iCompassType;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Approximate time (in seconds) for the needle to reach a new direction", params: "0.1 10 0.1", category: "Compass")]
	float m_fDirectionChangeTime; 

	[Attribute(defvalue: "25", uiwidget: UIWidgets.Slider, desc: "Percentage overshoot of the needle when settling", params: "0 95 0.1", category: "Compass")]
	float m_fDirectionOvershootPercentage; 
	
	[Attribute(defvalue: "2", UIWidgets.CheckBox, desc: "Frequency (in hertz) of the vertical shaking of the needle", params: "0.1 5 0.1", category: "Compass")]
	float m_fShakeFrequency;

	[Attribute(defvalue: "15", uiwidget: UIWidgets.Slider, desc: "Percentage overshoot the vertical needle shaking", params: "0 95 0.1", category: "Compass")]
	float m_fShakeOvershootPercentage; 
	
	[Attribute(defvalue: "3", UIWidgets.CheckBox, desc: "Maximum angle (in degrees) of the shaking", params: "0 15 0.1", category: "Compass")]
	float m_fShakeMaximumAngle; 	
	
	[Attribute("0 0.3 0", UIWidgets.Coords, desc: "camera position when displayed in map UI mode", category: "Compass")]
	vector m_vMapCameraPos; 	
	
	[Attribute("{72691366C26BD901}Prefabs/Items/Equipment/Compass/Compass_SY183_Map.et", desc: "Compass prefab used for display within 2D map", category: "Compass")]
	ResourceName m_sMapResource;
	
	// signals
	bool m_bSignalInit = false;
	int m_iSignalNeedle = -1;
	int m_iSignalShake = -1;
	int m_iSignalInHand = -1;
	int m_iSignalInMap = -1;
	int m_iSignalClose = -1;
		
	//------------------------------------------------------------------------------------------------
	//! Cache procedural animation signals
	void InitSignals(IEntity owner)
	{	
		SignalsManagerComponent signalMgr = SignalsManagerComponent.Cast( owner.FindComponent( SignalsManagerComponent ) );
		if (!signalMgr)
			return;

		// cache signals
		m_iSignalNeedle = signalMgr.FindSignal("Needle");
		m_iSignalShake = signalMgr.FindSignal("Shake");
		m_iSignalInHand = signalMgr.FindSignal("InHand");
		m_iSignalInMap = signalMgr.FindSignal("InMap");
		m_iSignalClose = signalMgr.FindSignal("Close");
		
		if (m_iSignalNeedle != -1 && m_iSignalShake != -1 && m_iSignalInHand != -1)
			m_bSignalInit = true;
	}
}

//! Compass component
class SCR_CompassComponent : SCR_GadgetComponent
{			
	static const float E = 2.71828182845904;
	
	// Derived from m_fDirectionChangeTime and m_fDirectionOvershootPercentage:
	protected float m_fNeedleAccelerationConstant = 0;
	protected float m_fNeedleDragConstant = 0;

	// The current position and velocity:
	protected float m_fNeedleAngle = 0;
	protected float m_fNeedleVelocity = 0;

	// Derived from m_fShakeFrequency and m_fShakeOvershootPercentage:
	protected float m_fShakeAccelerationConstant = 0;
	protected float m_fShakeDragConstant = 0;

	// The current shake angle and velocity:
	protected float m_fShakeAngle = 0;
	protected float m_fShakeVelocity = 0;

	// The shake is induced by the rotation since the last frame:
	protected vector m_vPreviousFrameAngles;
	
	// Various:
	protected bool m_bIsInMapMode;
	
	protected SignalsManagerComponent m_SignalManager;
	protected ref SCR_CompassComponentClass m_PrefabData;
	
	//------------------------------------------------------------------------------------------------
	//! Get 2D map prefab resource name
	//! \return returns prefab ResourceName
	ResourceName GetMapPrefabResource()
	{					
		return m_PrefabData.m_sMapResource;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates the needle angle around the Y axis, to pointing north
	//! \param[in] timeSlice is OnFrame timeslice
	protected void UpdateNeedleDirection(float timeSlice)
	{
		vector angles = GetOwner().GetYawPitchRoll();
		float northDirection = -angles[0];

		float magneticTorque = Math.Sin((northDirection - m_fNeedleAngle) * Math.DEG2RAD);

		float acceleration = magneticTorque * m_fNeedleAccelerationConstant;
		float drag = m_fNeedleVelocity * m_fNeedleDragConstant;

		float cappedTimeSlice = Math.Min(timeSlice, 1);

		m_fNeedleVelocity += (acceleration - drag) * cappedTimeSlice;
		m_fNeedleAngle += m_fNeedleVelocity * cappedTimeSlice;

		m_fNeedleAngle = SCR_Math.fmod(m_fNeedleAngle, 360);
		if (m_fNeedleAngle > 180) m_fNeedleAngle -= 360;

		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalNeedle, m_fNeedleAngle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Applies shake to the needle based on rotations
	//! \param[in] timeSlice is OnFrame timeslice
	protected void UpdateNeedleShake(float timeSlice)
	{
		vector angles = GetOwner().GetYawPitchRoll();
		vector deltaAngles = (m_vPreviousFrameAngles - angles);

		m_vPreviousFrameAngles = angles;

		float acceleration = -m_fShakeAngle * m_fShakeAccelerationConstant;
		float drag = m_fShakeVelocity * m_fShakeDragConstant;

		float cappedTimeSlice = Math.Min(timeSlice, 1);

		m_fShakeVelocity += (acceleration - drag) * cappedTimeSlice;

		// The pitch change is added to the angle (as if the needle stayed still as the compass was
		// rotated around the X axis) and yaw is added as shake when you spin around quickly:
		m_fShakeAngle += m_fShakeVelocity * cappedTimeSlice + deltaAngles[0] + deltaAngles[1];

		// The needle hit the glass or base below it, and stopped:
		if (m_fShakeAngle > m_PrefabData.m_fShakeMaximumAngle)
		{
			m_fShakeVelocity = 0;
			m_fShakeAngle = m_PrefabData.m_fShakeMaximumAngle;
		}

		if (m_fShakeAngle < -m_PrefabData.m_fShakeMaximumAngle)
		{
			m_fShakeVelocity = 0;
			m_fShakeAngle = -m_PrefabData.m_fShakeMaximumAngle;
		}

		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalShake, m_fShakeAngle);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Init for 2D map compass UI
	void Init2DMapCompass()
	{
		// Perturb the compass a bit:
		m_fShakeAngle = m_PrefabData.m_fShakeMaximumAngle;
		m_fNeedleVelocity = 5;
			
		// Initial set
		vector angles = GetOwner().GetYawPitchRoll();
		m_fNeedleAngle = -angles[0] + Math.RandomInt(-10, 10);
		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalNeedle, m_fNeedleAngle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! MapCompassUI drag activation, simulating movement
	void DragMapCompass()
	{
		// Perturb the compass a bit:
		m_fShakeAngle = m_PrefabData.m_fShakeMaximumAngle;
		m_fNeedleVelocity = 5;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate in a map UI mode 
	void SetMapMode()
	{
		m_iMode = EGadgetMode.IN_HAND;
		m_bActivated = true;
		m_bIsInMapMode = true;
		
		UpdateCompassState();
	}
						
	//------------------------------------------------------------------------------------------------
	//! Get compass in map UI camera position configuration
	vector GetMapCamPosition()
	{			
		return m_PrefabData.m_vMapCameraPos;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update state of compass active/inactive
	protected void UpdateCompassState()
	{		
		if (System.IsConsoleApp())
			return;
		
		if (m_bActivated)
			ActivateGadgetUpdate();
		else 
			DeactivateGadgetUpdate();
				
		if (!m_PrefabData.m_bSignalInit)
			m_PrefabData.InitSignals(GetOwner());
				
		// Map
		if (m_bIsInMapMode)
		{
			m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalInMap, 1);
			m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalInHand, 0);
			
			if (m_PrefabData.m_iCompassType == SCR_ECompassType.SY183)
				m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalClose, 0);
		}
		// Hand
		else if (m_iMode != EGadgetMode.IN_SLOT)
		{
			m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalInHand, 1);
			
			if (m_PrefabData.m_iCompassType == SCR_ECompassType.SY183)
				m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalClose, 0);
		}
		// Ground
		else 
		{
			m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalInHand, 0);
			
			if (m_PrefabData.m_iCompassType == SCR_ECompassType.SY183)
				m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalClose, 1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get damping ratiot from overshoot percentage
	//! \param[in] overshootPercentage is % overshoot of the needle when settling
	protected float GetDampingRatio(float overshootPercentage)
	{
		float logOvershoot = Math.Log2(Math.Max(0.001, overshootPercentage / 100.0)) / Math.Log2(E);
		float zeta = -logOvershoot / Math.Sqrt(Math.PI * Math.PI + logOvershoot * logOvershoot);

		return zeta;
	}

	//------------------------------------------------------------------------------------------------
	//! Init configurables
	protected void CalculateConstants()
	{
		// Calculate the needle constants:
		float zeta = GetDampingRatio(m_PrefabData.m_fDirectionOvershootPercentage);
		float naturalFrequency = Math.PI / Math.Max(0.1, m_PrefabData.m_fDirectionChangeTime);

		m_fNeedleAccelerationConstant = naturalFrequency * naturalFrequency * Math.RAD2DEG;
		m_fNeedleDragConstant = 2 * naturalFrequency * zeta;

		// Calculate the shake constants:
		zeta = GetDampingRatio(m_PrefabData.m_fShakeOvershootPercentage);
		naturalFrequency = m_PrefabData.m_fShakeFrequency;

		m_fShakeAccelerationConstant = naturalFrequency * naturalFrequency * Math.RAD2DEG;
		m_fShakeDragConstant = 2 * naturalFrequency * zeta;
	}
		
	
	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);
		
		if (mode == EGadgetMode.IN_HAND)
		{
			m_bActivated = true;
			UpdateCompassState();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{
		super.ModeClear(mode);
		
		if (mode == EGadgetMode.IN_HAND)
		{
			m_bActivated = false;
			UpdateCompassState();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void ActivateGadgetUpdate()
	{
		super.ActivateGadgetUpdate();
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.ActivateOwner(true);	// required for the procedural animations to function
	}
	
	//------------------------------------------------------------------------------------------------
	override void DeactivateGadgetUpdate()
	{
		super.DeactivateGadgetUpdate();
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.ActivateOwner(false);
	}
		
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.COMPASS;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeRaised()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsUsingADSControls()
	{
		return true;
	}
		
	//------------------------------------------------------------------------------------------------	
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;
		
		writer.WriteBool(m_bActivated);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;
		
		reader.ReadBool(m_bActivated);
			
		UpdateCompassState();
		
		return true;
	}
							
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{						
		UpdateNeedleDirection(timeSlice);
		UpdateNeedleShake(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_PrefabData = SCR_CompassComponentClass.Cast( GetComponentData(owner) );
		m_SignalManager = SignalsManagerComponent.Cast( owner.FindComponent( SignalsManagerComponent ) );
			
		CalculateConstants();
		UpdateCompassState();
	}
}
