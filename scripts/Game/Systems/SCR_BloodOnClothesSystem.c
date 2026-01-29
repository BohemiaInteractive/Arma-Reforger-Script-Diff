/*
System to update the bleeding mask on clothing items covering bleeding hitZones
*/

class SCR_BloodOnClothesSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.Frame);
	}

	protected ref array<SCR_BleedingDamageEffect> m_aEffects = {};
	
	protected float m_fUpdateClothesTimer;
	protected const float UPDATE_CLOTHES_DELAY = 0.5;
		
	SCR_CharacterDamageManagerComponent m_CharacterDamageManager;
	SCR_CharacterHitZone m_HitZone;

	//------------------------------------------------------------------------------------------------
	protected override void OnInit()
	{
		Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		bool nullValuePresent;
		m_fUpdateClothesTimer += args.GetTimeSliceSeconds();
		if (m_fUpdateClothesTimer < UPDATE_CLOTHES_DELAY)
			return;
		
		foreach (SCR_BleedingDamageEffect effect : m_aEffects)
		{
			if (!effect)
			{
				nullValuePresent = true;
				continue;
			}
			
			m_HitZone = SCR_CharacterHitZone.Cast(effect.GetAffectedHitZone());
			m_CharacterDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_HitZone.GetHitZoneContainer());
			if (m_CharacterDamageManager)
				m_CharacterDamageManager.AddBloodToClothes(m_HitZone, effect.GetDPS() * m_fUpdateClothesTimer);
		}
		
		if (nullValuePresent)
		{
			for (int i = m_aEffects.Count() - 1; i >= 0; i--)
			{
				if (!m_aEffects[i])
					m_aEffects.Remove(i);
			}
		}
		
		m_fUpdateClothesTimer = 0;
	}

	//------------------------------------------------------------------------------------------------
	void Register(SCR_BleedingDamageEffect bleedingEffect)
	{
		if (!bleedingEffect)
			return;
		
		if (!m_aEffects.Contains(bleedingEffect))
			m_aEffects.Insert(bleedingEffect);
		
		if (!IsEnabled())
			Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_BleedingDamageEffect bleedingEffect)
	{
		m_aEffects.RemoveItem(bleedingEffect);
		if (m_aEffects.IsEmpty())
			Enable(false);
	}
}
