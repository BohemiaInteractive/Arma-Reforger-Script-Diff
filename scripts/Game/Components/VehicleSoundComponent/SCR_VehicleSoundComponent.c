[BaseContainerProps(configRoot: true)]
class SCR_NormalizedSignalData
{
	[Attribute("", UIWidgets.Auto, "Source signal name")]
	string m_sSignalName;
	
	[Attribute("", UIWidgets.Auto, "Normalized signal name")]
	string m_sNormalizedSignalName;
	
	[Attribute("", UIWidgets.Auto, "")]
	float m_fMinimum;
	
	[Attribute("", UIWidgets.Auto, "")]
	float m_fMaximum;
	
	[Attribute("1", UIWidgets.Auto, "0.1 10 0.001")]
	float m_fExponent;
	
	//------------------------------------------------------------------------------------------------
	float GetNormalizedValue(float in)
	{
		if (m_fMinimum < m_fMaximum)
		{
			if (in < m_fMinimum)
			{
				return 0.0;
			}
			else if (in > m_fMaximum)
			{
				return 1.0;
			}
			else
			{
				return Math.Pow((in - m_fMinimum) / (m_fMaximum - m_fMinimum), m_fExponent);
			}
		}
		else if (m_fMinimum > m_fMaximum)
		{
			if (in < m_fMaximum)
			{
				return 0.0;
			}
			else if (in > m_fMinimum)
			{
				return 1.0;
			}
			else
			{
				return Math.Pow((in - m_fMaximum) / (m_fMinimum - m_fMaximum), m_fExponent);
			}
		}
		else
		{
			return 0.0;
		}
	}
}

class SCR_NormalizedSignal
{
	int m_signalIndex = -1;
	int m_normalizedSignalIndex = -1;
	
	void SCR_NormalizedSignal(int signalIndex, int normalizedSignalIndex)
	{
		m_signalIndex = signalIndex;
		m_normalizedSignalIndex = normalizedSignalIndex;
	}
}

[EntityEditorProps(category: "GameScripted/Sound", description: "Testing component")]
class SCR_VehicleSoundComponentClass : VehicleSoundComponentClass
{
	[Attribute("", UIWidgets.Auto, "HitZone State Signals")]
	ref array<ref SCR_HitZoneStateSignalData> m_aHitZoneStateSignalData;
	
	[Attribute("", UIWidgets.Auto, "Normalized signals")]
	ref array<ref SCR_NormalizedSignalData> m_aNormalizeSignalData;
}

class SCR_VehicleSoundComponent : VehicleSoundComponent
{		
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected ref array<ref SCR_HitZoneStateSignal> m_aHitZoneStateSignal;
	protected ref array<ref SCR_NormalizedSignal> m_aNormalizedSignal;
	
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
	protected array<ref SCR_NormalizedSignalData> GetNormalizedSignalData()
	{
		SCR_VehicleSoundComponentClass prefabData = SCR_VehicleSoundComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
		{
			return prefabData.m_aNormalizeSignalData;
		}
		
		return null;	
	}
				
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
				
		if (m_SignalsManagerComponent)
		{
			RegisterHitZoneSignals(owner);
			RegisterNormalizedSignals(owner);
			
			// Dont call UpdateSoundJob() if normalized signals definition is empty
			if (!m_aNormalizedSignal || m_aNormalizedSignal.IsEmpty())
			{
				SetScriptedMethodsCall(false);
			}
		}
		else
		{
			// Dont call UpdateSoundJob() if signals cannot be updated
			SetScriptedMethodsCall(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void UpdateSoundJob(IEntity owner, float timeSlice)
	{	
		super.UpdateSoundJob(owner, timeSlice);
		
		array<ref SCR_NormalizedSignalData> normalizedSignalData = GetNormalizedSignalData();
		if (!normalizedSignalData)
			return;
				
		foreach (int i, SCR_NormalizedSignal normalizedSignal : m_aNormalizedSignal)
		{					
			const float signalValue = m_SignalsManagerComponent.GetSignalValue(normalizedSignal.m_signalIndex);
			const float normalizedSignalValue = normalizedSignalData[i].GetNormalizedValue(signalValue);
			
			m_SignalsManagerComponent.SetSignalValue(normalizedSignal.m_normalizedSignalIndex, normalizedSignalValue);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	protected void RegisterHitZoneSignals(IEntity owner)
	{					
		SCR_DamageManagerComponent hitZoneContainerComponent = SCR_DamageManagerComponent.Cast(GetOwner().FindComponent(SCR_DamageManagerComponent));
		if (!hitZoneContainerComponent)
			return;
		
		array<ref SCR_HitZoneStateSignalData> hitZoneStateSignalData = GetHitZoneStateSignalData();
		if (!hitZoneStateSignalData)
			return;
		
		if (hitZoneStateSignalData.IsEmpty())
			return;
		
		if (!m_aHitZoneStateSignal)
		{
			m_aHitZoneStateSignal = new array<ref SCR_HitZoneStateSignal>();
		}
					
		foreach(SCR_HitZoneStateSignalData data : hitZoneStateSignalData)
		{		
			SCR_HitZoneStateSignal hitZoneSignal = new SCR_HitZoneStateSignal();			
			if (hitZoneSignal.RegisterSignal(hitZoneContainerComponent, data, m_SignalsManagerComponent))
				m_aHitZoneStateSignal.Insert(hitZoneSignal);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterNormalizedSignals(IEntity owner)
	{		
		array<ref SCR_NormalizedSignalData> normalizedSignalData = GetNormalizedSignalData();
		if (!normalizedSignalData)
			return;
		
		if (normalizedSignalData.IsEmpty())
			return;
		
		if (!m_aNormalizedSignal)
		{
			m_aNormalizedSignal = new array<ref SCR_NormalizedSignal>();
		}
		
		m_aNormalizedSignal.Resize(normalizedSignalData.Count());
			
		foreach(int i, SCR_NormalizedSignalData data : normalizedSignalData)
		{							
			const int signalIndex = m_SignalsManagerComponent.AddOrFindSignal(data.m_sSignalName);
			const int normalizedSignalIndex = m_SignalsManagerComponent.AddOrFindSignal(data.m_sNormalizedSignalName);
			
			m_aNormalizedSignal[i] = new SCR_NormalizedSignal(signalIndex, normalizedSignalIndex);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnregisterHitZoneSignals()
	{
		if (!m_aHitZoneStateSignal)
			return;
		
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
		SetScriptedMethodsCall(true);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_VehicleSoundComponent()
	{
		UnregisterHitZoneSignals();
	}
}
