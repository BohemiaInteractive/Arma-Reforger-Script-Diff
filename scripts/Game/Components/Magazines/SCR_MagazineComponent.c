class SCR_MagazineComponentClass : MagazineComponentClass
{
	[Attribute(defvalue: SCR_EMagazineRepackingRules.CAN_WHEN_OTHER_CONTAINS_THIS_AMMO_TYPES.ToString(), desc: "Rule that will be used for determining how this magazine can be repacked", uiwidget: UIWidgets.ComboBox, enumType: SCR_EMagazineRepackingRules)]
	protected SCR_EMagazineRepackingRules m_eRepackingRule;

	[Attribute(defvalue: "100", desc: "Time in ms that it takes to insert a round into this magazine", params: "0 inf")]
	protected int m_iRoundLoadingTime;

	[Attribute(defvalue: "100", desc: "Time in ms that it takes to extract the round from this magazine", params: "0 inf")]
	protected int m_iRoundExtractingTime;

	protected ref set<int> m_aAmmoComposition;
	protected int m_iCaliberId = -1;

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRoundExtractionTime()
	{
		return m_iRoundExtractingTime;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRoundLoadingTime()
	{
		return m_iRoundLoadingTime;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	set<int> GetAmmoComposition()
	{
		return m_aAmmoComposition;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetCaliberId()
	{
		return m_iCaliberId;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] otherMagazine
	//! \return
	bool CanBeRepacked(notnull SCR_MagazineComponentClass otherData)
	{
		int rule = m_eRepackingRule;
		int ruleOverride = SCR_MagazineRepackingSystem.GetGameModeRepackingRule();
		if (ruleOverride != SCR_EMagazineRepackingRulesOverride.NO_OVERRIDE)
			rule = ruleOverride;

		if (rule == SCR_EMagazineRepackingRules.CANNOT)
			return false;

		if (this == otherData)
			return true;
		else if (rule == SCR_EMagazineRepackingRules.CAN_WITH_SAME_MAGAZINE)
			return false;

		switch (rule)
		{
			case SCR_EMagazineRepackingRules.CAN_WITH_SAME_AMMO_CONFIG:
				MagazineConfig myConfig = GetMagazineConfig();
				return myConfig && myConfig == otherData.GetMagazineConfig();

			case SCR_EMagazineRepackingRules.CAN_WITH_SAME_AMMO_COMPOSITION:
				return CompareAmmoComposition(otherData, true);

			case SCR_EMagazineRepackingRules.CAN_WHEN_OTHER_CONTAINS_THIS_AMMO_TYPES:
				return CompareAmmoComposition(otherData);

			case SCR_EMagazineRepackingRules.CAN_WITH_SAME_CALIBER:
				return m_iCaliberId > -1 && m_iCaliberId == otherData.GetCaliberId();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Compares if ammo types used by each magazine match
	//! \param[in] secondData from which ammo composition will be extracted
	//! \param[in] exact decides if both sets have to have exactly same types of ammunition, when false, then it allows second set to contain more types than first set, but it still cannot not have types from this set
	//! \return true when ammo type composition from first data is present in composition from second data, otherwise false
	protected bool CompareAmmoComposition(notnull SCR_MagazineComponentClass secondData, bool exact = false)
	{
		if (!m_aAmmoComposition || m_aAmmoComposition.IsEmpty())
			return false;

		set<int> secondComposition = secondData.GetAmmoComposition();
		if (!secondComposition)
			return false;

		if (exact && m_aAmmoComposition.Count() != secondComposition.Count())
			return false; // if they have different amount of elements they surely cannot match

		foreach (int ammoType : m_aAmmoComposition)
		{
			if (!secondComposition.Contains(ammoType))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] system
	void SetupAmmoComposition(notnull SCR_MagazineRepackingSystem system)
	{
		if (m_aAmmoComposition)
			return;

		MagazineConfig config = GetMagazineConfig();
		if (!config)
			return;

		array<int> ammoMapping = {};
		GetAmmoMapping(ammoMapping);
		if (ammoMapping.IsEmpty())
			return;

		array<ResourceName> configContent = {};
		config.GetAmmoResourceArray(configContent);
		if (configContent.IsEmpty())
			return;

		set<int> uniqueList = new set<int>();
		foreach(int ammoType : ammoMapping)
		{
			uniqueList.Insert(ammoType);
		}

		ResourceName prefabPath;
		m_aAmmoComposition = new set<int>();
		foreach (int prefabId : uniqueList)
		{
			if (!configContent.IsIndexValid(prefabId))
			{
				Debug.Error(string.Format("ERROR: While setting up ammo composition -> index %1 is invalid for its ammo config!", prefabId));
				continue;
			}

			int ammoId = system.GetAmmoTypeId(configContent[prefabId]);
			if (ammoId < 0)
			{
				Debug.Error(string.Format("ERROR: While setting up ammo composition -> SCR_MagazineComponentClass was unable to find %1 in the SCR_MagazineRepackingSystem ammunition list ensure that all projectiles are in %2", configContent[prefabId], SCR_MagazineRepackingSystem.AMMO_LIST));
				continue;
			}

			m_aAmmoComposition.Insert(ammoId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initialization of shared data used by all instances of this prefab
	//! \param[in] component which triggered the initialization
	//! \param[in] componentSource
	//! \param[in] ent
	//! \param[in] parent
	void Init(SCR_MagazineComponent component, IEntityComponentSource componentSource, IEntity ent, IEntity parent)
	{
		if (m_iCaliberId >= 0)
			return;

		SCR_MagazineRepackingSystem system = SCR_MagazineRepackingSystem.GetInstance();
		if (!system)
			return;

		if (system.IsReady())
			SetupAmmoComposition(system);
		else
			system.AddToTheInitialiationQueue(this);

		MagazineUIInfo magInfo = MagazineUIInfo.Cast(component.GetUIInfo());
		if (!magInfo)
			return;

		string caliber = magInfo.GetAmmoCaliber();
		if (caliber.IsEmpty())
		{
#ifdef WORKBENCH
			Debug.Error(string.Format("SCR_MagazineComponent failed to initalize component data of %1 as it has \"UI Info\" -> \"Ammo Caliber\" not configured", SCR_FileIOHelper.GetShortResourceName(ent.GetPrefabData().GetPrefabName())));
#endif
			return;
		}

		m_iCaliberId = system.GetCaliberId(caliber);
	}
}

class SCR_MagazineComponent : MagazineComponent
{
	private void SCR_MagazineComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SCR_MagazineComponentClass data = SCR_MagazineComponentClass.Cast(GetComponentData(ent));
		if (data)
			data.Init(this, src, ent, parent);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] otherMagazine
	//! \return
	bool CanBeRepacked(notnull SCR_MagazineComponent otherMagazine)
	{
		if (GetAmmoCount() >= GetMaxAmmoCount())
			return false;

		if (otherMagazine.GetAmmoCount() <= 0)
			return false;

		SCR_MagazineComponentClass data = SCR_MagazineComponentClass.Cast(GetComponentData(GetOwner()));
		SCR_MagazineComponentClass otherData = SCR_MagazineComponentClass.Cast(otherMagazine.GetComponentData(otherMagazine.GetOwner()));
		return data && otherData && data.CanBeRepacked(otherData);
	}
}
