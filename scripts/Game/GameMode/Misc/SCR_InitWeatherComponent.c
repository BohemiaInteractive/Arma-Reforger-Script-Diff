[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_InitWeatherComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_InitWeatherComponent : SCR_BaseGameModeComponent
{
	[Attribute(defvalue: "", desc: "Weather IDs are the same as used in the TimeAndWeatherManager. This sets the weather as soon as the game has been started. This weather will loop and never change automatically. If this is empty then the automated weather system will run instead")]
	protected string m_sInitialStartingWeatherId;
	
	[Attribute(defvalue: "0", desc: "If true it will make sure that the set weather will stay that weather state until the GM changes this")]
	protected bool m_iSetWeatherConstant;
	
	[Attribute(defvalue: "0.8", desc: "Takes float up to 24 (hours) and is used to randomize the inital weather duration. Must be lower then m_initialWeatherDurationMax. Is ignored if weather is constant.", params: "0.1 24")]
	protected float m_initialWeatherDurationMin; 
	
	[Attribute(defvalue: "1.2", desc: "Takes float up to 24 (hours) and is used to randomize the inital weather duration. Must be higher then m_initialWeatherDurationMin. Is ignored if weather is constant.", params: "0.1 24")]
	protected float m_initialWeatherDurationMax; 
	
	//------------------------------------------------------------------------------------------------
	//! Sets initial weather state when server is first loaded. This state will loop and never change automatically
	protected void InitWeatherServer()
	{
		if (!Replication.IsServer())
			return;
		
		if (m_sInitialStartingWeatherId.IsEmpty())
			return;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if (!world) 
		{
			Print("SCR_InitWeatherComponent is not inside a ChimeraWorld", LogLevel.WARNING);
			return;
		}
		
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
		{
			Print("SCR_InitWeatherComponent could not find TimeAndWeatherManagerEntity", LogLevel.WARNING);
			return;
		}
		
		float weatherDuration = 0;
		if (!m_iSetWeatherConstant)
		{
			if (m_initialWeatherDurationMin > m_initialWeatherDurationMax)
				m_initialWeatherDurationMin = m_initialWeatherDurationMax;
			
			weatherDuration = Math.RandomFloat(m_initialWeatherDurationMin, m_initialWeatherDurationMax);
		}
		
		weatherManager.ForceWeatherTo(m_iSetWeatherConstant, m_sInitialStartingWeatherId, weatherDuration * 0.5, weatherDuration * 0.5);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		InitWeatherServer();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
}
