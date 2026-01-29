[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypoint
{
	//TODO: Request Autocomplete attribute to be exposed to script with getters and setters
	//TODO: Request AI Behaviour Tree attribute to be exposed to script with getters and setters
	//TODO: Request AI Behaviour Tree Move To attribute to be exposed to script with getters and setters

	//! Getters and Setters in this class (And similar that inherit) are set in a way to allow for different default values for attributes of child classes.
	//! This is why we do not have the actual attribute here, but just empty Setters and Getters returning some value that will never be used.
	
	SCR_ScenarioFrameworkLayerBase m_SlotWaypoint;

	//------------------------------------------------------------------------------------------------
	//! \param[in] waypointEntity
	void SetupWaypoint(IEntity waypointEntity)
	{
		AIWaypoint waypoint = AIWaypoint.Cast(waypointEntity);
		if (!waypoint)
			return;

		waypoint.SetCompletionRadius(GetWaypointCompletionRadius());
		waypoint.SetCompletionType(GetWaypointCompletionType());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] radius
	void SetWaypointCompletionRadius(float radius)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetWaypointCompletionRadius()
	{
		return 10;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	void SetWaypointCompletionType(EAIWaypointCompletionType type)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	EAIWaypointCompletionType GetWaypointCompletionType()
	{
		return EAIWaypointCompletionType.Any;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] prefab
	void SetWaypointPrefab(ResourceName prefab)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetWaypointPrefab()
	{
		return string.Empty;
	}
}


[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointScripted : SCR_ScenarioFrameworkWaypoint
{
	[Attribute(desc: "Settings which are valid while this waypoint is active.")]
	protected ref array<ref SCR_AISettingBase> m_aSettings;
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkWaypointScripted()
	{
		// Init all settings, they must have proper origin value
		foreach (auto s : m_aSettings)
			s.Internal_ConstructedAtProperty(SCR_EAISettingOrigin.SCENARIO, SCR_EAISettingFlags.SCENARIO_FRAMEWORK);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_AIWaypoint waypointScripted = SCR_AIWaypoint.Cast(waypointEntity);
		if (!waypointScripted)
			return;

		waypointScripted.SetPriorityLevel(GetWaypointPriorityLevel());
		
		// Add settings
		foreach (auto s : m_aSettings)
			waypointScripted.AddSetting(s);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] priority
	void SetWaypointPriorityLevel(float priority)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetWaypointPriorityLevel()
	{
		return 0;
	}
}


[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointSmartAction : SCR_ScenarioFrameworkWaypointScripted
{
	[Attribute(desc: "Static reference to entity with smart action")]
	string m_sStaticEntityName;

	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_SmartActionWaypoint waypointSmartAction = SCR_SmartActionWaypoint.Cast(waypointEntity);
		if (!waypointSmartAction)
			return;

		waypointSmartAction.m_sSmartActionTag = GetWaypointSmartActionTag();
		waypointSmartAction.m_sStaticEntityName = m_sStaticEntityName;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] tag
	void SetWaypointSmartActionTag(string tag)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetWaypointSmartActionTag()
	{
		return string.Empty;
	}
}


[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointEntity : SCR_ScenarioFrameworkWaypointScripted
{
	[Attribute(desc: "Related entity")]
	string	m_sEntityName;

	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_EntityWaypoint waypointScriptedEntity = SCR_EntityWaypoint.Cast(waypointEntity);
		if (!waypointScriptedEntity)
			return;

		waypointScriptedEntity.SetEntityName(m_sEntityName);
	}
}


[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointTimed : SCR_ScenarioFrameworkWaypointScripted
{
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_TimedWaypoint waypointTimed = SCR_TimedWaypoint.Cast(waypointEntity);
		if (!waypointTimed)
			return;

		waypointTimed.SetHoldingTime(GetWaypointHoldingTime());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] time
	void SetWaypointHoldingTime(float time)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetWaypointHoldingTime()
	{
		return 0;
	}
}


[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointTimedDefend : SCR_ScenarioFrameworkWaypointTimed
{
	[Attribute("0", UIWidgets.Object, "If enabled, it will clear default Defend presets that are on prefab. There has to be at least one Preset set afterwards in order for this Waypoint to work.")]
	bool m_bClearDefaultDefendPresets;
	
	[Attribute("", UIWidgets.Object, "Defend presets that will be added to the waypoint")];
	ref array<ref SCR_DefendWaypointPreset> m_aDefendPresets;
	
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_DefendWaypoint waypointDefend = SCR_DefendWaypoint.Cast(waypointEntity);
		if (!waypointDefend)
			return;

		waypointDefend.SetFastInit(GetFastInit());
		
		if (m_bClearDefaultDefendPresets)
			waypointDefend.ClearDefendPresets();
		
		foreach (SCR_DefendWaypointPreset defendPreset : m_aDefendPresets)
		{
			waypointDefend.AddDefendPreset(defendPreset);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled
	void SetFastInit(bool enabled)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetFastInit()
	{
		return false;
	}
}


[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointBoarding : SCR_ScenarioFrameworkWaypointScripted
{
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_BoardingWaypoint waypointBoarding = SCR_BoardingWaypoint.Cast(waypointEntity);
		if (!waypointBoarding)
			return;

		waypointBoarding.SetAllowance(GetDriverAllowed(), GetGunnerAllowed(), GetCargoAllowed());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled
	void SetDriverAllowed(bool enabled)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetDriverAllowed()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled
	void SetGunnerAllowed(bool enabled)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetGunnerAllowed()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled
	void SetCargoAllowed(bool enabled)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetCargoAllowed()
	{
		return false;
	}
}


[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointBoardingEntity : SCR_ScenarioFrameworkWaypointBoarding
{
	[Attribute(desc: "Related entity")]
	string	m_sEntityName;
	
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);

		SCR_BoardingEntityWaypoint waypointBoardingEntity = SCR_BoardingEntityWaypoint.Cast(waypointEntity);
		if (!waypointBoardingEntity)
			return;

		waypointBoardingEntity.SetEntityName(m_sEntityName);
	}
}


[BaseContainerProps()]
class SCR_ScenarioFrameworkWaypointBoardingTimed : SCR_ScenarioFrameworkWaypointBoarding
{
	//------------------------------------------------------------------------------------------------
	override void SetupWaypoint(IEntity waypointEntity)
	{
		super.SetupWaypoint(waypointEntity);
		
		SCR_BoardingTimedWaypoint waypointBoardingTimed = SCR_BoardingTimedWaypoint.Cast(waypointEntity);
		if (!waypointBoardingTimed)
			return;

		waypointBoardingTimed.SetHoldingTime(GetWaypointHoldingTime());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] time
	void SetWaypointHoldingTime(float time);

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetWaypointHoldingTime()
	{
		return 0;
	}
}