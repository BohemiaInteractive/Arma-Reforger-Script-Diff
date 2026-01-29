[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_EntityConditionTooltipDetail : SCR_EntityTooltipDetail
{	
	[Attribute("{01E150D909447632}Configs/Damage/DamageStateConfig.conf", desc: "Config to get visual data from", params: "conf class=SCR_DamageStateConfig")]
	protected ref SCR_DamageStateConfig m_DamageStateConfig;
	
	[Attribute("{19A8157C2CA02EFB}UI/layouts/Damage/DamageStateIcon.layout", desc: "Layout to create and apply visuals to", params: "layout")]
	protected ResourceName m_sStateIconLayout;
	
	[Attribute("32", desc: "Size of condition icons")]
	protected float m_fConditionIconSize;
	
	[Attribute("0", desc: "If true show death icon rather then destroy icon")]
	protected bool m_bDiesOnDestroy;
	
	//~ Ref
	protected Widget m_ConditionHolder;
	protected ref array<EDamageType> m_aActiveDamageTypes = {};
	
	//~ States
	protected ref array<ref SCR_DamageStateInfo> m_aDamageStateUiInfo = {};
	protected bool m_bIsDestroyed = false;
	protected bool m_bIsUnconscious = false;
	protected bool m_bHasNoConditions = false;
	
	protected Widget m_wLabel;

	protected DamageManagerComponent m_DamageManager;
	
	//------------------------------------------------------------------------------------------------
	override bool CreateDetail(SCR_EditableEntityComponent entity, Widget parent, TextWidget label, bool setFrameslot = true)
	{
		m_wLabel = label;
		
		return super.CreateDetail(entity, parent, label, setFrameslot);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return m_ConditionHolder && m_DamageManager && (m_DamageManager.GetState() != EDamageState.DESTROYED || (m_DamageManager.GetState() == EDamageState.DESTROYED && !m_bIsDestroyed) && !m_aDamageStateUiInfo.IsEmpty());// && m_MultiLineTextWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		bool isDestroyed, isUnconscious, conditionChanged;
		array<EDamageType> activeCondition = {};
		
		//~ If has no conditions but is active hide it
		if (!HasConditions(isDestroyed, isUnconscious, activeCondition, conditionChanged))
		{
			//~ Already hidden
			if (m_bHasNoConditions)
				return;
			
			m_wLabel.SetVisible(false);
			m_ConditionHolder.SetVisible(false);
			m_bHasNoConditions = true;
			return;
		}
		//~ Show again
		else if (m_bHasNoConditions)
		{
			m_wLabel.SetVisible(true);
			m_ConditionHolder.SetVisible(true);
			m_bHasNoConditions = false;
		}
		
		//~ If destroyed
		if (isDestroyed)
		{
			//~ Already set is destroyed
			if (m_bIsDestroyed)
				return;
			
			m_bIsDestroyed = true;
			
			CreateDestroyIcon();
		}
		else 
		{
			m_bIsDestroyed = false;
		}
		
		//~ Conditions still the same
		if (!conditionChanged && m_bIsUnconscious == isUnconscious)
			return;
		
		m_bIsUnconscious = isUnconscious;
		
		CreateConditionIcons(activeCondition, m_bIsUnconscious);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateConditionIcons(array<EDamageType> activeCondition, bool isUnconscious)
	{
		ClearConditionHolder();
		Widget damageStateWidget;
		SCR_DamageStateUIComponent damageStateUIComponent;
		
		if (isUnconscious)
		{
			damageStateWidget = GetGame().GetWorkspace().CreateWidgets(m_sStateIconLayout, m_ConditionHolder);
			if (!damageStateWidget)
			{
				Print("'SCR_EntityConditionTooltipDetail' Unable to create icon widget!",LogLevel.ERROR);
				return;
			}
			
			damageStateUIComponent = SCR_DamageStateUIComponent.Cast(damageStateWidget.FindHandler(SCR_DamageStateUIComponent));
			if (!damageStateUIComponent)
			{
				Print("'SCR_EntityConditionTooltipDetail' Unable to create find SCR_DamageStateUIComponent on entry!",LogLevel.ERROR);
				return;
			}
			
			damageStateUIComponent.SetSize(m_fConditionIconSize);
			damageStateUIComponent.SetVisuals(m_DamageStateConfig.GetUnconciousStateUiInfo());
		}
		
		//~ Todo: Currently all icons are on one line. This might cause an overflow if more conditions are added. Make sure that the m_Layout used is a vertical instead of a horizontal then add horizontal for each x amount and use the horizontal for m_ConditionHolder
		
		foreach (EDamageType damageType: activeCondition)
		{
			damageStateWidget = GetGame().GetWorkspace().CreateWidgets(m_sStateIconLayout, m_ConditionHolder);
			
			if (!damageStateWidget)
			{
				Print("'SCR_EntityConditionTooltipDetail' Unable to create icon widget!",LogLevel.ERROR);
				return;
			}
				
			damageStateUIComponent = SCR_DamageStateUIComponent.Cast(damageStateWidget.FindHandler(SCR_DamageStateUIComponent));
			if (!damageStateUIComponent)
			{
				Print("'SCR_EntityConditionTooltipDetail' Unable to create find SCR_DamageStateUIComponent on entry!",LogLevel.ERROR);
				return;
			}
				
			damageStateUIComponent.SetSize(m_fConditionIconSize);
			damageStateUIComponent.SetVisuals(damageType);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateDestroyIcon()
	{
		ClearConditionHolder();
		
		Widget damageStateWidget = GetGame().GetWorkspace().CreateWidgets(m_sStateIconLayout, m_ConditionHolder);
		if (!damageStateWidget)
		{
			Print("'SCR_EntityConditionTooltipDetail' Unable to create icon widget!",LogLevel.ERROR);
			return;
		}
		
		SCR_DamageStateUIComponent damageStateUIComponent = SCR_DamageStateUIComponent.Cast(damageStateWidget.FindHandler(SCR_DamageStateUIComponent));
		if (!damageStateUIComponent)
		{
			Print("'SCR_EntityConditionTooltipDetail' Unable to create find SCR_DamageStateUIComponent on entry!",LogLevel.ERROR);
			return;
		}
		
		damageStateUIComponent.SetSize(m_fConditionIconSize);
		
		if (!m_bDiesOnDestroy)
			damageStateUIComponent.SetVisuals(m_DamageStateConfig.GetDestroyedStateUiInfo());
		else 
			damageStateUIComponent.SetVisuals(m_DamageStateConfig.GetDeathStateUiInfo());
	}
	
//	//------------------------------------------------------------------------------------------------
//	protected void CreateConditionText(LocalizedString text)
//	{
//		ClearConditionHolder();
//
//		TextWidget textWidget = TextWidget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sStateTextOnlyLayout, m_ConditionHolder));
//		if (!textWidget)
//		{
//			Print("'SCR_EntityConditionTooltipDetail' Unable to create text widget!",LogLevel.ERROR);
//			return;
//		}
//
//		textWidget.SetText(text);
//	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearConditionHolder()
	{
		Widget child = m_ConditionHolder.GetChildren();
		Widget childtemp;
		while (child)
		{
			childtemp = child;
			child = child.GetSibling();
			childtemp.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		if (!widget || !m_DamageStateConfig || m_sStateIconLayout.IsEmpty())
			return false;
		
		m_ConditionHolder = widget;
		
		m_DamageManager = DamageManagerComponent.Cast(entity.GetOwner().FindComponent(DamageManagerComponent));
		if (!m_DamageManager)
			return false;
		
		if (m_DamageStateConfig.GetDamageStateInfoArray(m_aDamageStateUiInfo) <= 0)
			return false;
	
		bool isDestroyed, isUnconscious, conditionChanged;
		array<EDamageType> activeCondition = {};
		
		if (HasConditions(isDestroyed, isUnconscious, activeCondition, conditionChanged, true))
		{
			UpdateDetail(entity);
			return true;
		}
			
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool HasConditions(out bool isDestroyed, out bool isUnconscious, out notnull array<EDamageType> activeCondition, out bool conditionChanged, bool onlyCheckOneTrue = false)
	{		
		bool hasCondition = false;
		
		//~ Always return if destroyed as no need to check the other conditions
		if (m_DamageManager.GetState() == EDamageState.DESTROYED)
		{
			isDestroyed = true;
			return true;
		}
		
		ChimeraCharacter char = ChimeraCharacter.Cast(m_DamageManager.GetOwner());
		if (char)
		{
			CharacterControllerComponent controller = char.GetCharacterController();
			if (controller.GetLifeState() == ECharacterLifeState.INCAPACITATED)
			{
				isUnconscious = true;
				
				if (onlyCheckOneTrue)
					return true;
				else 
					hasCondition = true;
			}
		}
		
		bool isDamagedOverTime;
		SCR_CharacterDamageManagerComponent characterDamageManager;
		
		foreach (SCR_DamageStateInfo damageStateInfo: m_aDamageStateUiInfo)
		{
			if (damageStateInfo.m_eDamageType != EDamageType.BLEEDING)
			{
				isDamagedOverTime = m_DamageManager.IsDamagedOverTime(damageStateInfo.m_eDamageType);
			}
			else
			{
				characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_DamageManager);
				isDamagedOverTime = characterDamageManager && characterDamageManager.IsBleeding();
			}
			
			if (isDamagedOverTime)
			{
				activeCondition.Insert(damageStateInfo.m_eDamageType);
				
				if (onlyCheckOneTrue)
				{
					return true;
				}
				else 
				{
					if (!m_aActiveDamageTypes.Contains(damageStateInfo.m_eDamageType))
						conditionChanged = true;
					
					hasCondition = true;
				}
			}
		}
		
		if (activeCondition.Count() != m_aActiveDamageTypes.Count())
			conditionChanged = true;
		
		if (conditionChanged)
			m_aActiveDamageTypes.Copy(activeCondition);
		
		return hasCondition;
	}
}
