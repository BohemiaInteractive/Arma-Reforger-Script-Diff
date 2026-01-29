//! Origin of setting is identified by this value. This enum can be extended by mods.
//! If you add more values, ensure that values are sequential, don't declare something with value 10000 in the middle, or it will blow up memory.
enum SCR_EAISettingOrigin
{
	EDITOR,				// Created by Editor (Game Master) at run time
	SCENARIO,			// Created by scenario framework
	WAYPOINT,			// Placed into waypoint
	COMMANDING,			// AI Commanding
	BEHAVIOR,			// Settings created by AI internally from their behaviors
	DEFAULT				// Settings added to prefabs, as default AI settings
}

enum SCR_EAISettingFlags
{
	CREATED_DYNAMICALLY	= 1<<0, // Means it was created via script at run time by Create method. It was never part of property.
	
	// These flags below are not mutually exclusive,
	// because as setting gets copied, it will get owned by settings component, while it might originate from waypoint
	SETTINGS_COMPONENT	= 1<<1,	// Some settings component owns it
	WAYPOINT 			= 1<<2,	// Some waypoint owns or owned it
	SCENARIO_FRAMEWORK	= 1<<3	// Some scenario framework container owns or owned this
}

//! Class for a table of priority value of each settings origin
class SCR_AISettingOriginPriorityTable
{
	protected ref array<int> m_aValues; // Index is origin ID, value is priority.
	
	//------------------------------------------------------------------------------------------------
	//! Returns priority value of given setting origin
	int GetPriorityForOrigin(SCR_EAISettingOrigin origin)
	{
		if (!m_aValues.IsIndexValid(origin))
			return 0;
		
		return m_aValues[origin];
	}
	
	//---------------------------------------------------------------------------------------------------
	void SCR_AISettingOriginPriorityTable()
	{
		m_aValues = {};
		
		// Init array with settings priorities
		typename t = SCR_EAISettingOrigin;
		int originsCount = Math.ClampInt(t.GetVariableCount(), 0, 32); // Limit array size to some reasonable value
		m_aValues.Resize(originsCount);
		InitPriorities(m_aValues);
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Initializes priority values of setting origins.
	//! Can be overriden in addon to add extra values or change priorities of some settings origins
	protected void InitPriorities(array<int> a)
	{
		// Currently idea is to have editor and scenario above commanding, prioritizing mission creator's choice
		a[SCR_EAISettingOrigin.EDITOR] 		= 6000;
		a[SCR_EAISettingOrigin.SCENARIO] 	= 5000;
		a[SCR_EAISettingOrigin.WAYPOINT] 	= 4000;
		a[SCR_EAISettingOrigin.COMMANDING] 	= 3000;
		a[SCR_EAISettingOrigin.BEHAVIOR] 	= 2000;
		a[SCR_EAISettingOrigin.DEFAULT] 	= 1000;
	}
}


/*!
Base class for all AI settings.

When making own settings classes, following parts must be done:

- Override GetCategorizationType
There must be a base class according to which settings affecting same aspect of AI are categorized.
For example: there is one base class for all settings which restrict AI stance, and thus GetCategorizationType returns typename of that class.
Underlying AI systems also use that class to find if currently there is a setting which restricts AI stance.
Several final classes might inherit from that base class, and implement their own way to restrict stance: restrict to one value, to range of values, etc.

- Decide if the setting works 'contextually', depending on Cause value of current action, or it always dictates some value to AI.
Depending on that decide from which class the new setting class must inherit.
There are several classes for that.
For character settings: SCR_AICharacterSetting and SCR_AICharacterSettingWithCause.
For group settings: SCR_AIGroupSetting and SCR_AIGroupSettingWithCause.

- Override ApplyEffects, if setting does not work with action Cause value.

- Override CreateCopy, if setting class holds non-trivial data (reference to something else).
*/
[BaseContainerProps(visible: false)]
class SCR_AISettingBase
{
	// Special value for settings which do not support action cause.
	// The value is big so that setting is always active regardless of current action cause value.
	const int ACTION_CAUSE_NOT_SUPPORTED = int.MAX;
	
	// Pointer to setting from which this was copied
	protected SCR_AISettingBase m_ParentSetting;
	
	protected SCR_EAISettingOrigin m_eOrigin;
	
	protected SCR_EAISettingFlags m_eFlags;
	
	[SortAttribute()]
	protected int m_iPriority;
	
	// Static table of priority of each origin of settings
	protected static ref SCR_AISettingOriginPriorityTable s_OriginPriorityTable = new SCR_AISettingOriginPriorityTable();
	
	//---------------------------------------------------------------------------------------------------
	int GetPriority()
	{
		return m_iPriority;
	}
	
	//---------------------------------------------------------------------------------------------------
	SCR_EAISettingOrigin GetOrigin()
	{
		return m_eOrigin;
	}
	
	//---------------------------------------------------------------------------------------------------
	SCR_AISettingBase GetParentSetting()
	{
		return m_ParentSetting;
	}
	
	//---------------------------------------------------------------------------------------------------
	SCR_EAISettingFlags GetFlags()
	{
		return m_eFlags;
	}
	
	//---------------------------------------------------------------------------------------------------
	bool HasFlag(SCR_EAISettingFlags f)
	{
		return (m_eFlags & f);
	}
	
	//---------------------------------------------------------------------------------------------------
	//!  Returns action cause value to which this setting applies.
	//!  Must be implemented in derived class.
	//!  The returned value means: this setting applies to this action cause and all below it.
	//!  For instance, setting applies to combat cause and all below it.
	//!  If ACTION_CAUSE_NOT_SUPPORTED is returned, the setting is always active as long as it has highest priority,
	//! since some settings can't be active or inactive based on current action. For instance settings related to AI skills.
	int GetMaxActionCause();
	
	
	
	//----------------------------------------------------------------------------------------------------------------------------
	// Internal and protected methods, mostly related to initialization
	
	//---------------------------------------------------------------------------------------------------
	//!  This method must make a deep copy of data describing this setting.
	//!  Data aquired after application of the setting to Agent should be omitted.
	//!  !!! This must be implemented in derived class if data of the setting is non-trivial.
	SCR_AISettingBase CreateCopy()
	{
		Managed sManaged = Clone();
		auto s = SCR_AISettingBase.Cast(sManaged);
		return s;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Called right after _this_ setting object was created as copy from elsewhere
	void Internal_InitCopyFrom(SCR_AISettingBase from)
	{
		m_ParentSetting = from;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Must be called from the factory method, if setting is created dynamically
	protected void Init(SCR_EAISettingOrigin origin)
	{
		m_eFlags |= SCR_EAISettingFlags.CREATED_DYNAMICALLY;
		m_eOrigin = origin;
		m_iPriority = s_OriginPriorityTable.GetPriorityForOrigin(m_eOrigin);
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Must be called if the setting is part of some property.
	//! Call this if you declare property with AI settings somewhere.
	void Internal_ConstructedAtProperty(SCR_EAISettingOrigin origin, SCR_EAISettingFlags flagsToSet)
	{
		m_eFlags |= flagsToSet;
		m_eOrigin = origin;
		m_iPriority = s_OriginPriorityTable.GetPriorityForOrigin(m_eOrigin);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------
	void Internal_SetFlag(SCR_EAISettingFlags flagsToSet)
	{
		m_eFlags |= flagsToSet;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------
	// Methods to override in derived classes
	
	//---------------------------------------------------------------------------------------------------
	//! Returns type which is used for identification of setting type.
	//! There might be settings which inherit some base setting class and should be categorized according to that parent class.
	//! For instance, some specific classes of movement restriction can be categorized according to their base class of movement restriction.
	typename GetCategorizationType()
	{
		return Type();
	}
	
	//---------------------------------------------------------------------------------------------------
	//!  Called after the setting was added to SCR_AISettingsBaseComponent.
	//!  Can be overridden in child classes.
	//!  OnAdded call does not mean that this setting now has highest priority,
	//! as there can be other settings of same type but with different priority.
	//! For that see function ApplyEffects.
	void OnAdded(SCR_AISettingsBaseComponent settingsComp);
	
	//---------------------------------------------------------------------------------------------------
	//! Called after the setting was removed from SCR_AISettingsBaseComponent.
	//! Can be overridden in child classes.
	void OnRemoved(SCR_AISettingsBaseComponent settingsComp);
	
	//---------------------------------------------------------------------------------------------------
	//!  This is meant for settings which apply some permanent effect to AI (like setting skill),
	//! instead of some other system to look up current settings through GetCurrentSetting call.
	//!  The method is called when addition or removal of this or another setting causes _this_ setting
	//! to be the one with highest priority among those of same categorization type.
	//!  Keep in mind that now it might get called several times, and also we can't know
	//! if our setting unbecame highest priority. So here we should just propagate our values 'downwards' and do nothing else.
	void ApplyEffects(SCR_AISettingsBaseComponent settingsComp);
	
	//---------------------------------------------------------------------------------------------------
	string GetDebugText()
	{
		return string.Empty;
	}
}

