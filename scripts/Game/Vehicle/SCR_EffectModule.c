//------------------------------------------------------------------------------------------------
//Base generic effect class
[BaseContainerProps()]
class SCR_EffectModule
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Particle effect", params: "ptc")]
	protected ResourceName m_sParticle;														//the particle resource
	[Attribute(desc: "Effect position")]
	protected ref PointInfo		m_effectPosition;											//the position of the m_effectPosition
	[Attribute(desc: "If not checked, the modul is activated and deactivated only with physics event. If checked, the effect is always active")]
	protected bool m_bTickOnFrame;															//if false, effect is activate only with EOnPhysic
	[Attribute("", desc: "Play the effect in Workbench", params: "Debug")]
	protected bool m_bShowEffectInWB;
	protected IEntity m_owner;													
	protected SCR_BaseEffectManagerComponent m_Manager;
	protected ParticleEffectEntity m_sParticleEffect;
	protected SignalsManagerComponent m_SignalManager;
	
	
	[Attribute(desc: "Particle modifier action")]
	protected ref array<ref SCR_BaseEffectParticleAction> m_aEffectActions;					//array of actions which can modify the particle based on the signal value
	

	//------------------------------------------------------------------------------------------------
	bool GetTickOnFrame()
	{
		return m_bTickOnFrame;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetOwner()
	{
		return m_Manager.GetOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	void TurnOn()
	{
		if (!m_sParticleEffect)
			CreateEmitter();
		if (!m_sParticleEffect)
			return;
		m_sParticleEffect.Play();
	}

	//------------------------------------------------------------------------------------------------
	void TurnOff()
	{
		if (!m_sParticleEffect)
			return;
		m_sParticleEffect.Stop();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateEmitter(bool PlayOnSpawn = false)
	{
		if (!m_sParticle)
			return;
		
		if (!m_Manager)
		{
			if(!m_owner)
				return;
		}
		else
		{
			m_owner = m_Manager.GetOwner();
		}
		
		vector transform[4];
		m_effectPosition.GetLocalTransform(transform);
		
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.UseFrameEvent = true;
		spawnParams.Parent = m_owner;
		spawnParams.PivotID = m_effectPosition.GetNodeId();
		spawnParams.Transform = transform;
		
		m_sParticleEffect = ParticleEffectEntity.SpawnParticleEffect(m_sParticle, spawnParams);
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected void WB_PlayEffect()
	{
		CreateEmitter(true);
		//TurnOn();
	}
#endif
			
	//------------------------------------------------------------------------------------------------
	ParticleEffectEntity GetEmitter()
	{
		return m_sParticleEffect;
	}
	
	//------------------------------------------------------------------------------------------------
	SignalsManagerComponent GetSignalManager()
	{
		return m_SignalManager;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitActions()
	{
		foreach (SCR_BaseEffectParticleAction action : m_aEffectActions)
		{
			if (!action)
				continue;
			action.Init(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DeInitActions()
	{
		foreach (SCR_BaseEffectParticleAction action : m_aEffectActions)
		{
			if (!action)
				continue;
			action.DeInit();
		}
		m_aEffectActions.Clear();
	}
	
	
	//------------------------------------------------------------------------------------------------
	void Init(SCR_BaseEffectManagerComponent manager)
	{
		m_Manager = manager;
		if (manager)
			m_SignalManager = SignalsManagerComponent.Cast(manager.GetOwner().FindComponent(SignalsManagerComponent));
		
		m_effectPosition.Init(manager.GetOwner());
		
		InitActions();
	}
	
	//------------------------------------------------------------------------------------------------
	void DeInit()
	{
		m_SignalManager = null;
		m_Manager = null;
		DeInitActions();
	}

	//------------------------------------------------------------------------------------------------
	void Update(IEntity owner, float timeSlice)
	{
		foreach (SCR_BaseEffectParticleAction action : m_aEffectActions)
		{
			if (!action)
				continue;
			action.Update();
		}
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	void WB_Update(IEntity owner, float timeSlice)
	{
		if (m_bShowEffectInWB && m_sParticleEffect)
			//m_sParticleEffect.WB_Update(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	void WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		TurnOff();
		m_owner = owner;
		WB_PlayEffect();
	}
#endif	
};

//------------------------------------------------------------------------------------------------
//Helicopter specific effect class
[BaseContainerProps()]
class SCR_EffectModuleHelicopterExhaust : SCR_EffectModule
{
	[Attribute("10", UIWidgets.Auto, desc: "Time to play startup particle\n[s]")]
	protected float m_fStartupTime;

	protected SCR_HelicopterControllerComponent m_HelicopterController;

	//------------------------------------------------------------------------------------------------
	override void Init(SCR_BaseEffectManagerComponent manager)
	{
		super.Init(manager)
		//do the stuff here
	}

	//------------------------------------------------------------------------------------------------
	override void Update(IEntity owner, float timeSlice)
	{
		super.Update(owner, timeSlice);
	}
};

enum SCR_EmitterParam
{
	TURN_ON,
	TURN_OFF,
	BIRTHRATE,
	LIFETIME,
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_BaseEffectParticleAction
{
	[Attribute("0", UIWidgets.ComboBox, "Type of parameter to change", "", ParamEnumArray.FromEnum(SCR_EmitterParam))]
	protected SCR_EmitterParam m_Parameter;
	[Attribute(desc: "New value of the parameter")]
	protected float m_fValue;
	
	protected SCR_EffectModule m_Module;
	protected bool m_bPerformed = false;

	//------------------------------------------------------------------------------------------------
	void Init(notnull SCR_EffectModule module)
	{
		m_Module = module;
	}
	
	//------------------------------------------------------------------------------------------------
	void DeInit()
	{
		m_Module = null;
	}

	//------------------------------------------------------------------------------------------------
	void Update();

	//------------------------------------------------------------------------------------------------
	protected void PerformAction()
	{
		if (m_Parameter == SCR_EmitterParam.TURN_ON)
		{
			m_Module.TurnOn();
		}		
		else if (m_Parameter == SCR_EmitterParam.TURN_OFF)
		{
			m_Module.TurnOff();
		}		
		else if (m_Parameter == SCR_EmitterParam.BIRTHRATE)
		{
			m_Module.GetEmitter().GetParticles().SetParam(-1, EmitterParam.BIRTH_RATE, m_fValue);
		}
		else if (m_Parameter == SCR_EmitterParam.LIFETIME)
		{
		
		}
	}
}



//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_BaseEffectParticleSignalAction: SCR_BaseEffectParticleAction
{
	[Attribute(desc: "Signal which drives the effect")]
	protected string m_sSignal;
	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_ComparerOperator.GREATER_THAN_OR_EQUAL), UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;
	[Attribute(desc: "Value of signal when the action will be executed")]
	protected float m_fSignalValue;
	protected int m_iSignalID;
	SignalsManagerComponent m_SignalManager;
	
	//------------------------------------------------------------------------------------------------
	override void Init(notnull SCR_EffectModule module)
	{
		super.Init(module);
		m_SignalManager = module.GetSignalManager();
		if (!m_SignalManager)
			return;
		m_iSignalID = m_SignalManager.FindSignal(m_sSignal);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void DeInit()
	{
		m_SignalManager = null;
		m_iSignalID = -1;
		super.DeInit();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update()
	{
		if (m_bPerformed)
		{
			if (m_SignalManager.GetSignalValue(m_iSignalID) == 0.0)
				m_bPerformed = false;	//if signal returns to 0.0, reset the flag
			return;
		}
			
		//add option to do this only once until the next change
		PrintFormat("Signal value = %1", m_SignalManager.GetSignalValue(m_iSignalID));
		
		if (SCR_Comparer<float>.Compare(m_eOperator, (float)m_SignalManager.GetSignalValue(m_iSignalID), (float)m_fSignalValue ))
		{
			m_bPerformed = true;
			PerformAction();
		}
	}
}


enum EVehicleEvent
{
	ON_ENGINE_START,
	ON_ENGINE_STOP,
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_BaseEffectParticleHelicopterEventAction: SCR_BaseEffectParticleAction
{
	//TODO(zozo): this could be done better - some dynamic event getter would be useful
	[Attribute(defvalue: "0", UIWidgets.ComboBox, "Events", "", ParamEnumArray.FromEnum(EVehicleEvent))]
	protected EVehicleEvent m_eEvent;
	
	[Attribute(defvalue: "0", desc: "Perform the action only once")]
	protected bool m_bOnce;
	protected SCR_HelicopterControllerComponent m_VehicleController;
	protected ScriptInvokerVoid m_EventHandler;
	
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterEvent()
	{
		switch(m_eEvent)
		{
			case EVehicleEvent.ON_ENGINE_START:
			{
				m_EventHandler = m_VehicleController.GetOnEngineStart();
				break;
			};
			case EVehicleEvent.ON_ENGINE_STOP:
			{
				m_EventHandler = m_VehicleController.GetOnEngineStop();
				break;
			};
		}
		
		if (m_EventHandler)
			m_EventHandler.Insert(PerformAction);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(notnull SCR_EffectModule module)
	{
		super.Init(module);
		IEntity owner = module.GetOwner();
		if (!owner)	//this would be weird, but doublecheck it
			return;	
		m_VehicleController = SCR_HelicopterControllerComponent.Cast(owner.FindComponent(SCR_HelicopterControllerComponent));
		if (!m_VehicleController)
		{
			Print("SCR_HelicopterControllerComponent controller not found. Action won't work", LogLevel.WARNING);
			return;
		}
		RegisterEvent();
	}	
	
	//------------------------------------------------------------------------------------------------
	override void DeInit()
	{
		m_VehicleController = null;
		m_EventHandler.Clear();
		super.DeInit();
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction()
	{
		super.PerformAction();
		if (m_bOnce)
			m_EventHandler.Remove(PerformAction);	
	}
	
}

//------------------------------------------------------------------------------------------------
//! Specific action which controls the particles from the rotor wash. 
[BaseContainerProps()]
class SCR_BaseEffectParticleHelicopterRotorControlAction: SCR_BaseEffectParticleAction
{
	[Attribute( "", UIWidgets.EditBox, "Speed in m/s which tops the effect's interpolation from 100 to 0% lifetime value" )]
	float m_ExhaustEndSpeedInM;
	
	const string SIGNAL_RPM = "MainRotorRPM";
	const int RPM_THRESHOLD = 50;	//when to enable or disable the rotor wash effect
	protected int m_iSignalID;
	SignalsManagerComponent m_SignalManager;
	ParticleEffectEntity m_EffectEntity;
		
	//------------------------------------------------------------------------------------------------
	override void Init(notnull SCR_EffectModule module)
	{
		super.Init(module);
		m_SignalManager = module.GetSignalManager();
		if (!m_SignalManager)
			return;

		m_iSignalID = m_SignalManager.FindSignal(SIGNAL_RPM);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void Update()
	{
		//PrintFormat("Signal value = %1", m_SignalManager.GetSignalValue(m_iSignalID));

		Particles particles;
		if (m_EffectEntity)
			particles = m_EffectEntity.GetParticles();

		if (!particles && m_SignalManager.GetSignalValue(m_iSignalID) > RPM_THRESHOLD)	// engine is started, but effect hasn't been yet created
		{
			m_Module.TurnOn();
		}
		else
		if (particles && m_SignalManager.GetSignalValue(m_iSignalID) < RPM_THRESHOLD)	// engine was already running. Now is turned off and RPM went down
		{
			m_Module.TurnOff();	
			m_EffectEntity = null;
			particles = null;
		}

		if (!particles && m_Module.GetEmitter())
		{
			m_EffectEntity = m_Module.GetEmitter();
			if (m_EffectEntity)
				particles = m_EffectEntity.GetParticles(); // get the particles
		}

		if (!particles)
			return;

		if (m_ExhaustEndSpeedInM <= 0)
			return;
		
		Physics physics = m_Module.GetOwner().GetPhysics();
		if (!physics)
			return;

		float speed_m_per_s = physics.GetVelocity().Length();
		float lifetime_scale = Math.Clamp(1 - (speed_m_per_s / m_ExhaustEndSpeedInM), 0, 1);

		particles.MultParam(-1, EmitterParam.LIFETIME,		lifetime_scale);
		particles.MultParam(-1, EmitterParam.LIFETIME_RND,	lifetime_scale);
	}
}
