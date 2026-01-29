[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_BloodTooltipDetail : SCR_DpsConditionBarBaseTooltipDetail
{	
	protected SCR_CharacterDamageManagerComponent m_CharacterDamageManager;
	
	[Attribute("BloodUnconsciousBar", desc: "Image that shows when Character becomes conscious")]
	protected string m_sUnconsciousBarName;
	
	[Attribute("BloodUnconsciousBarTop", desc: "Set at the end of the bar")]
	protected string m_sUnconsciousBarTopName;
	
	[Attribute("BloodUnconsciousFillRight", desc: "Filler make sure the bar is set the correct size")]
	protected string m_sUnconsciousFillerRightName;
	
	[Attribute("0.025", desc: "Size of the BarTop")]
	protected float m_iUnconsciousBarTopSize;
	
	protected float m_fBloodLevelLoseDeath;
	protected float m_fBloodLevelContiousness;
	protected float m_fBloodLevel;
	
	protected Widget m_UnconsciousBar;
	protected Widget m_UnconsciousBarTop;
	protected Widget m_UnconsciousFillerRight;
	
	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return m_CharacterDamageManager != null;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		super.UpdateDetail(entity);
		
		HitZone bloodHitZone = m_CharacterDamageManager.GetBloodHitZone();
		if (!bloodHitZone)
			return;
		
		//~ If character dead set blood 0
		if (m_CharacterDamageManager.GetState() == EDamageState.DESTROYED)
		{
			
			if (m_UnconsciousBar)
				m_UnconsciousBar.SetVisible(false);
			
			if (m_UnconsciousBarTop)
				m_UnconsciousBarTop.SetVisible(false);
			
			if (m_UnconsciousFillerRight)
				m_UnconsciousFillerRight.SetVisible(false);
			
			SetBarAndPercentageValue(0);
			return;
		}
		
		float bloodLevel = Math.InverseLerp(bloodHitZone.GetDamageStateThreshold(ECharacterBloodState.DESTROYED), 1, bloodHitZone.GetHealthScaled());
		
		//~ Update blood visual
		bloodLevel = Math.Clamp(bloodLevel, 0, 1);
		UpdateBloodBar(bloodLevel, bloodHitZone.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS));
		SetBarAndPercentageValue(bloodLevel);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateBloodBar(float bloodLevel, float unconsciousLevel)
	{
		if (bloodLevel >= unconsciousLevel)
		{
			m_UnconsciousBarTop.SetVisible(true);
			LayoutSlot.SetFillWeight(m_UnconsciousBar, Math.Clamp(unconsciousLevel - m_iUnconsciousBarTopSize, 0, 1));
			LayoutSlot.SetFillWeight(m_UnconsciousFillerRight, Math.Clamp(1 - unconsciousLevel, 0, 1));
		}
		else 
		{
			if (bloodLevel >= m_iUnconsciousBarTopSize)
			{
				m_UnconsciousBarTop.SetVisible(true);
				LayoutSlot.SetFillWeight(m_UnconsciousBar, Math.Clamp(bloodLevel - m_iUnconsciousBarTopSize, 0 , 1));
				LayoutSlot.SetFillWeight(m_UnconsciousFillerRight, Math.Clamp(1 - bloodLevel, 0, 1));
			}
			else 
			{
				m_UnconsciousBarTop.SetVisible(false);
				LayoutSlot.SetFillWeight(m_UnconsciousBar, Math.Clamp(bloodLevel, 0 , 1));
				LayoutSlot.SetFillWeight(m_UnconsciousFillerRight, Math.Clamp(1 - bloodLevel, 0, 1));
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetBarColor(bool SetConditionColor)
	{
		super.SetBarColor(SetConditionColor);
		
		if (!m_UnconsciousBar || !m_UnconsciousBarTop)
			return;
		
		//~ Reverse colors of unconciouss amount
		if (m_bBarIsColored)
		{
			m_UnconsciousBar.SetColor(m_cDefaultBarColor);
			m_UnconsciousBarTop.SetColor(m_cDefaultBarColor);
		}
		else 
		{
			m_UnconsciousBar.SetColor(m_cHasDpsBarColor);
			m_UnconsciousBarTop.SetColor(m_cHasDpsBarColor);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		if (!super.InitDetail(entity, widget))
			return false;
		
		m_CharacterDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_DamageManager);
		if (!m_CharacterDamageManager)
			return false;
		
		HitZone bloodHitZone = m_CharacterDamageManager.GetBloodHitZone();
		if (!bloodHitZone)
			return false;
		
		m_UnconsciousBarTop = widget.FindAnyWidget(m_sUnconsciousBarTopName);
		m_UnconsciousBar = widget.FindAnyWidget(m_sUnconsciousBarName);
		m_UnconsciousFillerRight = widget.FindAnyWidget(m_sUnconsciousFillerRightName);
		
		if (!m_UnconsciousBarTop || !m_UnconsciousBar || !m_UnconsciousFillerRight)
		{
			Print("'SCR_BloodTooltipDetail' is missing UI to display which means it will be hidden!", LogLevel.ERROR);
			return false;
		}
		
		if (m_CharacterDamageManager.GetState() == EDamageState.DESTROYED)
			return false;
		
		if (m_CharacterDamageManager.GetState() == EDamageState.DESTROYED)
		{
			m_UnconsciousBar.SetVisible(false);
			m_UnconsciousBarTop.SetVisible(false);
			m_UnconsciousFillerRight.SetVisible(false);
			return false; 
		}
		
		LayoutSlot.SetFillWeight(m_UnconsciousBarTop, m_iUnconsciousBarTopSize);
	
		UpdateDetail(entity);
		return true;
	}
}
