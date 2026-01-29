[EntityEditorProps(category: "GameScripted/Sound", description: "Testing component")]
class SCR_VehicleSoundComponentClass : VehicleSoundComponentClass
{
	[Attribute("", UIWidgets.Auto, "HitZone State Signals")]
	ref array<ref SCR_HitZoneStateSignalData> m_aHitZoneStateSignalData;
}

class SCR_VehicleSoundComponent : VehicleSoundComponent
{		
	protected ref array<ref SCR_HitZoneStateSignal> m_aHitZoneStateSignal = {};
	
	//------------------------------------------------------------------------------------------------
	protected array<ref SCR_HitZoneStateSignalData> GetHitZoneStateSignalData()
	{
		SCR_VehicleSoundComponentClass prefabData = SCR_VehicleSoundComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
		{
			return prefabData.m_aHitZoneStateSignalData;
		}
		
		return null;	
	}
				
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		
		RegisterHitZoneSignals(owner);
	}
		
	//------------------------------------------------------------------------------------------------
	protected void RegisterHitZoneSignals(IEntity owner)
	{	
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent)	);	
		if (!signalsManagerComponent)
			return;
		
		SCR_DamageManagerComponent hitZoneContainerComponent = SCR_DamageManagerComponent.Cast(GetOwner().FindComponent(SCR_DamageManagerComponent));
		if (!hitZoneContainerComponent)
			return;
		
		array<ref SCR_HitZoneStateSignalData> hitZoneStateSignalData = GetHitZoneStateSignalData();
		if (!hitZoneStateSignalData)
			return;
					
		foreach(SCR_HitZoneStateSignalData data : hitZoneStateSignalData)
		{		
			SCR_HitZoneStateSignal hitZoneSignal = new SCR_HitZoneStateSignal;			
			if (hitZoneSignal.RegisterSignal(hitZoneContainerComponent, data, signalsManagerComponent))
				m_aHitZoneStateSignal.Insert(hitZoneSignal);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnregisterHitZoneSignals()
	{				
		foreach(SCR_HitZoneStateSignal hitZoneStateSignal : m_aHitZoneStateSignal)
		{
			hitZoneStateSignal.UnregisterSignal();
		}
		
		m_aHitZoneStateSignal.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_VehicleSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_VehicleSoundComponent()
	{
		UnregisterHitZoneSignals();
	}
}
