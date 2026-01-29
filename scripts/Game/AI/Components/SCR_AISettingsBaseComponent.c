//---------------------------------------------------------------------------------------------------
[ComponentEditorProps(visible: false)]
class SCR_AISettingsBaseComponentClass : ScriptComponentClass
{
}

/*!
Base class for storing settings individual AI Agents.

It solves following tasks:
- It stores setting objects.

- Setting objects are categorized by their type, for easier lookup.

- Setting objects of each type are sorted by their priority.
This allows multiple game systems provide their value of same type of AI settings, without affecting value provided by other game systems.
For example, AI Commanding, Scenario Framework and Editor both might want to affect AI stance setting.

- Underlying AI systems are supposed to call GetCurrentSetting to find currently active AI setting of given type.

- Alternatively it can work the opposite way: setting objects can propagate data into underlying parts of AI. It depends on how Setting class is configured.
*/
class SCR_AISettingsBaseComponent : ScriptComponent
{
	protected ref map<typename, ref array<ref SCR_AISettingBase>> m_mSettings = new map<typename, ref array<ref SCR_AISettingBase>>();
	protected AIBaseUtilityComponent m_UtilityComp;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_UtilityComp = AIBaseUtilityComponent.Cast(owner.FindComponent(AIBaseUtilityComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds setting object.
	//! Depending on createCopy value, from now on it will be owned by this component, or it will create a copy which will be owned by this component.
	//! It's protected. Instead use Add...Setting of derived class.
	//! createCopy - when true, a copy of the setting object will be created. When false, this component takes ownership of passed setting object.
	//! If you want to add same setting to many agents, always add it with createCopy=true, or make a copy yourself.
	//! removeSameTypeAndOrigin - if true, removes all settings which have same categorization type and origin.
	bool AddSetting(notnull SCR_AISettingBase setting, bool createCopy, bool removeSameTypeAndOrigin = false)
	{
		#ifdef AI_DEBUG
		string strDebugText = string.Format("AddSetting: %1, Origin: %2, Priority: %3, %4",
			setting,
			typename.EnumToString(SCR_EAISettingOrigin, setting.GetOrigin()),
			setting.GetPriority(),
			setting.GetDebugText());
		AddDebugMessage(strDebugText);
		#endif
		
		typename t = setting.GetCategorizationType();
		// Remove same type and origin?
		if (removeSameTypeAndOrigin)
			RemoveSettingsOfTypeAndOrigin(t, setting.GetOrigin());
		
		// Create copy of the setting if needed
		SCR_AISettingBase settingToAdd;
		if (createCopy)
		{
			SCR_AISettingBase settingCopy = setting.CreateCopy();
			settingCopy.Internal_InitCopyFrom(setting);
			settingToAdd = settingCopy;
		}
		else
			settingToAdd = setting;
		
		array<ref SCR_AISettingBase> settingsOfType = m_mSettings.Get(t);
		if (!settingsOfType)
		{
			settingsOfType = {};
			m_mSettings.Set(t, settingsOfType);
		}
		else
		{
			// Bail if we have already added exactly this setting
			if (settingsOfType.Contains(settingToAdd))
				return false;
		}
		
		settingToAdd.Internal_SetFlag(SCR_EAISettingFlags.SETTINGS_COMPONENT); // Now setting is registered here
		settingsOfType.Insert(settingToAdd);
		settingsOfType.Sort(true); // Sorted by priority, biggest priority first
		
		OnSettingAdded(settingToAdd);
		settingToAdd.OnAdded(this);
		
		// Call setting with highest priority to apply its effects
		settingsOfType[0].ApplyEffects(this);
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	bool RemoveSetting(notnull SCR_AISettingBase setting)
	{
		#ifdef AI_DEBUG
		string strDebugText = string.Format("RemoveSetting: %1, Origin: %2, Priority: %3, %4",
			setting,
			typename.EnumToString(SCR_EAISettingOrigin, setting.GetOrigin()),
			setting.GetPriority(),
			setting.GetDebugText());
		AddDebugMessage(strDebugText);
		#endif
		
		typename t = setting.GetCategorizationType();
		array<ref SCR_AISettingBase> settingsOfType = m_mSettings.Get(t);
		if (!settingsOfType)
			return false;
		
		if (!settingsOfType.Contains(setting))
			return false;
		
		settingsOfType.RemoveItemOrdered(setting);
		
		OnSettingRemoved(setting);
		setting.OnRemoved(this);
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Removes setting which has provided setting object as parent
	bool RemoveChildSettingsOfParent(notnull SCR_AISettingBase parentSetting)
	{
		#ifdef AI_DEBUG
		string strDebugText = string.Format("RemoveChildSettingsOfParent: %1, %2",
			parentSetting,
			parentSetting.GetDebugText());
		
		AddDebugMessage(strDebugText);
		#endif
		
		typename t = parentSetting.GetCategorizationType();
		array<ref SCR_AISettingBase> settingsOfType = m_mSettings.Get(t);
		if (!settingsOfType)
			return false;
		
		int id = -1;
		foreach (int i, auto s : settingsOfType)
		{
			if (s.GetParentSetting() == parentSetting)
			{
				id = i;
				break;
			}
		}
		
		if (id == -1)
			return false;
		
		SCR_AISettingBase setting = settingsOfType[id]; // Hold strong reference
		settingsOfType.RemoveOrdered(id);
		OnSettingRemoved(setting);
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Iterates all settings, removes all which have given flag
	void RemoveSettingsWithFlag(SCR_EAISettingFlags f)
	{
		#ifdef AI_DEBUG
		string strDebugText = string.Format("RemoveSettingsWithFlag: %1", typename.EnumToString(SCR_EAISettingFlags, f));
		AddDebugMessage(strDebugText);
		#endif
		
		foreach (typename t, array<ref SCR_AISettingBase> a : m_mSettings)
		{
			for (int i = a.Count()-1; i >= 0; i--)
			{
				auto s = a[i];
				if (s.HasFlag(f))
				{
					a.RemoveOrdered(i);
					OnSettingRemoved(s);
				}
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! typename t - must be one of those returned by GetCategorizationType
	void RemoveSettingsOfTypeAndOrigin(typename t, SCR_EAISettingOrigin origin)
	{
		#ifdef AI_DEBUG
		string strDebugText = string.Format("RemoveSetingsOfTypeAndOrigin: %1, %2", t, typename.EnumToString(SCR_EAISettingOrigin, origin));
		AddDebugMessage(strDebugText);
		#endif
		
		array<ref SCR_AISettingBase> settingsOfType = m_mSettings.Get(t);
		if (!settingsOfType)
			return;
		
		for (int i = settingsOfType.Count()-1; i >= 0; i--)
		{
			auto s = settingsOfType[i];
			if (s.GetOrigin() == origin)
			{
				settingsOfType.RemoveOrdered(i);
				OnSettingRemoved(s);
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Returns current setting based on type and cause value of current action
	//! typename settingType - must be one of those returned by GetCategorizationType
	SCR_AISettingBase GetCurrentSetting(typename settingType)
	{
		SCR_AIActionBase currentAction;
		
		if (m_UtilityComp)
			currentAction = SCR_AIActionBase.Cast(m_UtilityComp.GetCurrentAction());
		
		int actionCause = 0;
		if (currentAction)
			actionCause = currentAction.GetCause();
		
		return GetCurrentSetting(settingType, actionCause);
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Returns current setting based on type and cause value
	//! typename settingType - must be one of those returned by GetCategorizationType
	SCR_AISettingBase GetCurrentSetting(typename settingType, int currentActionCause)
	{		
		array<ref SCR_AISettingBase> settingsOfType = m_mSettings.Get(settingType);
		
		if (!settingsOfType || settingsOfType.IsEmpty())
			return null;
		
		// Settings are already ordered by priority, return first matching behavior type
		foreach (SCR_AISettingBase setting : settingsOfType)
		{
			if (currentActionCause <= setting.GetMaxActionCause())				
				return setting;
		}
		
		return null;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Returns all setting objects held by this component
	void GetAllSettings(notnull array<SCR_AISettingBase> outSettings)
	{
		outSettings.Clear();
		
		foreach (typename t, array<ref SCR_AISettingBase> settingsOfType : m_mSettings)
		{
			foreach (auto s : settingsOfType)
				outSettings.Insert(s);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------
	// Protected / Internal
	
	//------------------------------------------------------------------------------------------------
	protected void OnSettingRemoved(notnull SCR_AISettingBase setting);
	protected void OnSettingAdded(notnull SCR_AISettingBase setting);
	
	#ifdef AI_DEBUG
	//------------------------------------------------------------------------------------------------
	protected void AddDebugMessage(string str, EAIDebugMsgType messageType = EAIDebugMsgType.SETTINGS)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(str, msgType: messageType);
	}
	#endif
}
