//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Gadgets", description: "Wristwatch gadget")]
class SCR_WristwatchComponentClass : SCR_GadgetComponentClass
{
	[Attribute("0", UIWidgets.ComboBox, "Set wristwatch type", "", ParamEnumArray.FromEnum(EWristwatchType), category: "Wristwatch")]
	int m_iWristwatchType;
	
	[Attribute("{6ECDF523E1035A0F}Prefabs/Items/Equipment/Watches/Watch_SandY184A_Map.et", desc: "Wristwatch prefab used for display within 2D map", category: "Wristwatch")]
	ResourceName m_sMapResource;
	
	bool m_bSignalInit = false;
	int m_iSignalHour = -1;
	int m_iSignalMinute = -1;
	int m_iSignalSecond = -1;
	int m_iSignalDay = -1;
	
	//------------------------------------------------------------------------------------------------
	//! Cache procedural animation signals
	void InitSignals(IEntity owner)
	{	
		SignalsManagerComponent signalMgr = SignalsManagerComponent.Cast( owner.FindComponent( SignalsManagerComponent ) );
		if (!signalMgr)
			return;

		// cache signals
		m_iSignalHour = signalMgr.FindSignal("Hour");
		m_iSignalMinute = signalMgr.FindSignal("Minute");
		m_iSignalSecond = signalMgr.FindSignal("Second");
		
		if (m_iWristwatchType == EWristwatchType.VOSTOK)
			m_iSignalDay = signalMgr.FindSignal("Day");
		
		if (m_iSignalHour != -1 && m_iSignalMinute != -1 && m_iSignalSecond != -1)
			m_bSignalInit = true;
	}
}

//------------------------------------------------------------------------------------------------
//! Wristwatch type 
enum EWristwatchType
{
	SandY184A,	// US
	VOSTOK		// Soviet
}

//------------------------------------------------------------------------------------------------
class SCR_WristwatchComponent : SCR_GadgetComponent
{
	protected int m_iSeconds;
	protected int m_iMinutes;
	protected int m_iHours;
	protected int m_iDay;
	
	protected SCR_WristwatchComponentClass m_PrefabData;
	protected SignalsManagerComponent m_SignalManager;
	protected TimeAndWeatherManagerEntity m_TimeMgr;
		
	//------------------------------------------------------------------------------------------------
	//! Get 2D map prefab resource name
	//! \return returns prefab ResourceName
	ResourceName GetMapPrefabResource()
	{					
		return m_PrefabData.m_sMapResource;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void UpdateTime()
	{		
		if (!m_TimeMgr)
		{
			ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
			if (world)
				m_TimeMgr = world.GetTimeAndWeatherManager();
			
			return;
		}
		
		m_TimeMgr.GetHoursMinutesSeconds(m_iHours, m_iMinutes, m_iSeconds);
		if (m_iHours >= 12)
			m_iHours -= 12;
		
		m_iHours *= 10;
		m_iHours += (m_iMinutes/6);
		
		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalHour, m_iHours);
		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalMinute, m_iMinutes);
		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalSecond, m_iSeconds);
		
		m_iDay = m_TimeMgr.GetDay();
		
		m_SignalManager.SetSignalValue(m_PrefabData.m_iSignalDay, m_iDay);
	}
			
	//------------------------------------------------------------------------------------------------
	//! Update state of wristwatch -> active/inactive
	protected void UpdateWristwatchState()
	{
		if (System.IsConsoleApp())
			return;
		
		if (m_bActivated)
			ActivateGadgetUpdate();
		else 
			DeactivateGadgetUpdate();
		
		if (!m_PrefabData.m_bSignalInit)
			m_PrefabData.InitSignals(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate in a map UI mode 
	void SetMapMode()
	{
		m_iMode = EGadgetMode.IN_HAND;
		m_bActivated = true;
		UpdateWristwatchState();
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (world)
			m_TimeMgr = world.GetTimeAndWeatherManager();
		
		m_PrefabData.InitSignals(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);
		
		if (mode == EGadgetMode.IN_HAND)
		{
			m_bActivated = true;
			UpdateWristwatchState();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{				
		if (mode == EGadgetMode.IN_HAND)
		{
			m_bActivated = false;
			UpdateWristwatchState();
		}
			
		super.ModeClear(mode);
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
		return EGadgetType.WRISTWATCH;
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
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_PrefabData = SCR_WristwatchComponentClass.Cast( GetComponentData(owner) );
		m_SignalManager = SignalsManagerComponent.Cast( owner.FindComponent( SignalsManagerComponent ) );
		
		UpdateWristwatchState();
	}
		
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{					
		UpdateTime();
	}
}
