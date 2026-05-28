#ifdef WORKBENCH
//! see https://community.bistudio.com/wiki/Arma_Reforger:Assets
// TODO:
// - weapons (rifles, handguns, launchers, throwables)
// - medical items
// - clothing items?
[WorkbenchPluginAttribute(
	name: "Export Assets",
	description: "Exports https://community.bistudio.com/wiki/Arma_Reforger:Assets's content",
	wbModules: { "ResourceManager" },
	category: SCR_PluginCategory.BIKI,
	awesomeFontCode: 0xF70E)]
class SCR_BIKIFactionAssetsExportPlugin : SCR_BIKIExportBasePlugin
{
	[Attribute(params: "conf class=SCR_Faction")]
	protected ref array<ResourceName> m_aFactionConfigs;

	[Attribute(defvalue: "1", desc: "Export Characters")]
	protected bool m_bExportCharacters;

	[Attribute(defvalue: "1", desc: "Export Weapons")]
	protected bool m_bExportWeapons;

	[Attribute(defvalue: "1", desc: "Export Groups")]
	protected bool m_bExportGroups;

	[Attribute(defvalue: "1", desc: "Export Vehicles")]
	protected bool m_bExportVehicles;

	[Attribute(defvalue: "1", desc: "Export Weapon Tripods")]
	protected bool m_bExportWeaponTripods;

	protected static const float MAX_SPEED_MULTIPLIER = 0.95;
	protected static const float RANGE_FULL_RPM_RATIO = 0.90; // 0.1 = idle
	protected static const int MAX_LINES_BEFORE_HIDING = 6;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		CheckAttributes();
		if (!Workbench.ScriptDialog("Faction to export", "", this))
			return;

		CheckAttributes();

		SCR_BIKIStringBuilder result = new SCR_BIKIStringBuilder();
		result.AddLine("{{TOC|side}}");
		result.AddLine("{{Feature|informative|The latest list can be found in the following config files, under '''Entity Catalogs''':");
		foreach (ResourceName factionConfig : m_aFactionConfigs)
		{
			result.AddLine("* " + SCR_BIKIStringBuilder.GetResourceManagerLink(factionConfig));
		}

		result.AddLine("}}");

		Resource factionResource;
		BaseContainer factionContainer;
		foreach (int i, ResourceName factionConfig : m_aFactionConfigs)
		{
			factionResource = Resource.Load(factionConfig);
			if (!factionResource.IsValid())
				continue;

			factionContainer = factionResource.GetResource().ToBaseContainer();
			if (!factionContainer)
				continue;

			AddFactionString(result, factionContainer);
		}

		result.AddLine(
			string.Format(
				"\n\nGenerated on %1 UTC by %2\n\n\n{{GameCategory|armaR|Content}}",
				SCR_DateTimeHelper.GetDateTimeUTC(),
				SCR_BIKIStringBuilder.GetScriptEditorLink(__FILE__)));

		ShowResult(result.GetContent());
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckAttributes()
	{
		if (!m_aFactionConfigs || m_aFactionConfigs.IsEmpty())
			m_aFactionConfigs = {
				"{5EB46557DF2AA24F}Configs/Factions/US.conf",
				"{09727032415AC39B}Configs/Factions/USSR.conf",
				"{8053DB656DD18B14}Configs/Factions/FIA.conf",
				"{3FA20B01D950D31F}Configs/Factions/CIV.conf"
			};
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in, out] stringBuilder
	//! \param[in] factionContainer
	//! \return
	protected void AddFactionString(inout notnull SCR_BIKIStringBuilder stringBuilder, notnull BaseContainer factionContainer)
	{
		string factionKey;
		factionContainer.Get("FactionKey", factionKey);
		stringBuilder.AddTitle(2, factionKey);

		if (m_bExportCharacters)
			AddFactionCatalogString(stringBuilder, factionContainer, EEntityCatalogType.CHARACTER);

		if (m_bExportWeapons)
			AddFactionWeaponString(stringBuilder, factionContainer);

		if (m_bExportGroups)
			AddFactionCatalogString(stringBuilder, factionContainer, EEntityCatalogType.GROUP);

		if (m_bExportVehicles)
			AddFactionCatalogString(stringBuilder, factionContainer, EEntityCatalogType.VEHICLE);

		if (m_bExportWeaponTripods)
			AddFactionCatalogString(stringBuilder, factionContainer, EEntityCatalogType.WEAPONS_TRIPOD);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in,out] stringBuilder
	//! \param[in] factionContainer
	//! \param[in] catalogType
	protected static void AddFactionCatalogString(inout notnull SCR_BIKIStringBuilder stringBuilder, notnull BaseContainer factionContainer, EEntityCatalogType catalogType)
	{
		string factionKey;
		factionContainer.Get("FactionKey", factionKey);
		array<ResourceName> resourceNames = GetCategoryResourceNames(factionContainer, catalogType);
		if (resourceNames.IsEmpty())
		{
			PrintFormat("%1 Faction: catalog %2 is empty, skipping", factionKey, typename.EnumToString(EEntityCatalogType, catalogType), level: LogLevel.NORMAL);
			return;
		}

		stringBuilder.AddTitle(3, SCR_StringHelper.FormatSnakeCaseToUserFriendly(typename.EnumToString(EEntityCatalogType, catalogType)));
		stringBuilder.BeginTable(classes: "wikitable sortable valign-top", style: "min-width: 60em");
		stringBuilder.AddTableHeader("Name", "", "min-width: 25%");
		stringBuilder.AddTableHeader("Prefab");
		stringBuilder.AddTableHeader("Details", "unsortable");

		Resource resource;
		foreach (ResourceName prefab : resourceNames)
		{
			string userFriendlyName = SCR_BIKIGeneralHelper.GetName(prefab);
			if (!userFriendlyName)
			{
				userFriendlyName = SCR_StringHelper.FormatResourceNameToUserFriendly(prefab);
				if (!userFriendlyName)
					userFriendlyName = FilePath.StripExtension(FilePath.StripPath(prefab));
			}

			stringBuilder.AddTableDataCellsLine(userFriendlyName, SCR_BIKIStringBuilder.GetResourceManagerLink(prefab));

			string details;
			resource = Resource.Load(prefab);
			if (resource.IsValid())
				details = GetDetails(resource.GetResource().ToBaseContainer());

			if (details) // !.IsEmpty()
			{
				if (SCR_StringHelper.CountOccurrences(details.Trim(), "\n") < MAX_LINES_BEFORE_HIDING)
					stringBuilder.AddLine("| style=\"font-size: smaller\" | " + details);
				else
					stringBuilder.AddLine("| style=\"font-size: smaller\" | <spoiler text=\"Show details\">" + details + "\n</spoiler>");
			}
			else
			{
				stringBuilder.AddLine("|"); // empty line to still display an empty cell
			}
		}

		stringBuilder.EndTable();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in,out] stringBuilder
	//! \param[in] factionContainer
	protected static void AddFactionWeaponString(inout notnull SCR_BIKIStringBuilder stringBuilder, notnull BaseContainer factionContainer)
	{
		string factionKey;
		factionContainer.Get("FactionKey", factionKey);

		array<ResourceName> weaponResourceNames = GetCategoryResourceNames(factionContainer, EEntityCatalogType.ITEM, "Weapons");
		if (weaponResourceNames.IsEmpty())
		{
			Print(factionKey + " Faction: ITEM/Weapons catalog list is empty, skipping", LogLevel.NORMAL);
			return;
		}

		map<typename, ref array<ResourceName>> magazineWells;
		array<ResourceName> magazineResourceNames = GetCategoryResourceNames(factionContainer, EEntityCatalogType.ITEM, "Ammunition");
		if (magazineResourceNames.IsEmpty())
			Print(factionKey + " Faction: ITEM/Ammunition catalog list is empty, skipping magazine wells info", LogLevel.WARNING);
		else
			magazineWells = GetMagazineWells(magazineResourceNames);

		stringBuilder.AddTitle(3, "Weapons");
		stringBuilder.BeginTable(classes: "wikitable sortable valign-top", style: "min-width: 60em");
		stringBuilder.AddTableHeader("Name", "", "min-width: 25%");
		stringBuilder.AddTableHeader("Prefab");
		stringBuilder.AddTableHeader("Details", "unsortable");

		Resource resource;
		foreach (ResourceName weaponResourceName : weaponResourceNames)
		{
			string userFriendlyName = SCR_BIKIGeneralHelper.GetName(weaponResourceName);
			if (!userFriendlyName) // backup
			{
				userFriendlyName = SCR_StringHelper.FormatResourceNameToUserFriendly(weaponResourceName);
				if (!userFriendlyName)
					userFriendlyName = FilePath.StripExtension(FilePath.StripPath(weaponResourceName));
			}

			stringBuilder.AddTableDataCellsLine(userFriendlyName, SCR_BIKIStringBuilder.GetResourceManagerLink(weaponResourceName));

			string details;
			resource = Resource.Load(weaponResourceName);
			if (resource.IsValid())
				details = GetWeaponDetails(resource.GetResource().ToBaseContainer(), magazineWells);

			if (details) // !.IsEmpty()
			{
				if (SCR_StringHelper.CountOccurrences(details.Trim(), "\n") < MAX_LINES_BEFORE_HIDING)
					stringBuilder.AddLine("| style=\"font-size: smaller\" | " + details);
				else
					stringBuilder.AddLine("| style=\"font-size: smaller\" | <spoiler text=\"Show details\">" + details + "\n</spoiler>");
			}
			else
			{
				stringBuilder.AddLine("|"); // empty line to still display an empty cell
			}
		}

		stringBuilder.EndTable();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] factionContainer
	//! \param[in] catalogType
	//! \param[in] listName case-sensitive
	//! \return
	static array<ResourceName> GetCategoryResourceNames(notnull BaseContainer factionContainer, EEntityCatalogType catalogType, string listName = string.Empty)
	{
		array<ResourceName> result = {};

		BaseContainerList baseContainerList = factionContainer.GetObjectArray("m_aEntityCatalogs");
		BaseContainer baseContainer;
		BaseContainer entryContainer;
		BaseContainerList multiListEntitiesContainerList;
		for (int catalogId, catalogsCount = baseContainerList.Count(); catalogId < catalogsCount; ++catalogId)
		{
			baseContainer = baseContainerList.Get(catalogId);
			EEntityCatalogType type;
			if (!baseContainer.Get("m_eEntityCatalogType", type) || type != catalogType)
				continue;

			baseContainerList = baseContainer.GetObjectArray("m_aEntityEntryList");
			for (int entryId, entriesCount = baseContainerList.Count(); entryId < entriesCount; ++entryId)
			{
				entryContainer = baseContainerList.Get(entryId);
				bool enabled;
				if (!entryContainer.Get("m_bEnabled", enabled) || !enabled)
					continue;

				ResourceName resourceName;
				if (!entryContainer.Get("m_sEntityPrefab", resourceName) || !resourceName)
					continue;

				result.Insert(resourceName);
			}

			baseContainerList = baseContainer.GetObjectArray("m_aMultiLists");
			if (!baseContainerList)
				break;

			for (int multiListId, multiListCount = baseContainerList.Count(); multiListId < multiListCount; ++multiListId)
			{
				baseContainer = baseContainerList.Get(multiListId);

				if (listName)
				{
					string identifier;
					if (baseContainer.Get("m_sIdentifier", identifier) && identifier != listName)
						continue;
				}

				multiListEntitiesContainerList = baseContainer.GetObjectArray("m_aEntities");
				for (int multiListEntityId, multiListEntitiesCount = multiListEntitiesContainerList.Count(); multiListEntityId < multiListEntitiesCount; ++multiListEntityId)
				{
					entryContainer = multiListEntitiesContainerList.Get(multiListEntityId);
					bool enabled;
					if (!entryContainer.Get("m_bEnabled", enabled) || !enabled)
						continue;

					ResourceName resourceName;
					if (!entryContainer.Get("m_sEntityPrefab", resourceName) || !resourceName)
						continue;

					result.Insert(resourceName);
				}
			}

			break;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	static string GetDetails(notnull BaseContainer baseContainer)
	{
		if (SCR_BIKIGeneralHelper.IsCharacter(baseContainer))
			return GetCharacterDetails(baseContainer);

//		if (SCR_BIKIGeneralHelper.IsWeapon(baseContainer)) // uses GetWeaponDetails directly
//			return GetWeaponDetails(baseContainer);

		if (SCR_BIKIGeneralHelper.IsGroup(baseContainer))
			return GetGroupDetails(baseContainer);

		if (SCR_BIKIGeneralHelper.IsWheeled(baseContainer))
			return GetWheeledDetails(baseContainer);

		if (SCR_BIKIGeneralHelper.IsTracked(baseContainer))
			return GetTrackedDetails(baseContainer);

		if (SCR_BIKIGeneralHelper.IsHelicopter(baseContainer))
			return GetHelicopterDetails(baseContainer);

		if (SCR_BIKIGeneralHelper.IsStaticWeapon(baseContainer))
			return GetStaticWeaponDetails(baseContainer);

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetCharacterDetails(notnull BaseContainer baseContainer)
	{
		string result = "{{{!}}\n{{!}}";

		array<ResourceName> elements = SCR_BIKIGearHelper.GetWeapons(baseContainer);
		if (!elements.IsEmpty())
		{
			Resource resource;
			result += "\n* Weapons:";
			foreach (ResourceName element : elements) // no sorting/grouping here (otherwise accessories will be... tricky)
			{
				result += "\n** " + SCR_BIKIGeneralHelper.GetName(element);
				resource = Resource.Load(element);
				if (!resource.IsValid())
					continue;

				array<string> attachments = SCR_BIKIWeaponHelper.GetAttachmentNames(resource.GetResource().ToBaseContainer());
				attachments = GetCountedSortedStringArray(attachments);
				if (attachments.Count() == 1)
				{
					result += " + " + attachments[0];
				}
				else
				{
					foreach (string attachment : attachments)
					{
						result += "\n*** " + attachment;
					}
				}
			}
		}

		elements = SCR_BIKIGearHelper.GetGear(baseContainer);
		if (!elements.IsEmpty())
		{
			result += "\n* Equipment:";
			foreach (string element : GetCountedSortedStringArray(elements))
			{
				result += "\n** " + element;
			}
		}

		result += "\n{{!}}";

		elements = SCR_BIKIGearHelper.GetItems(baseContainer);
		if (!elements.IsEmpty())
		{
			result += "\n* Items:";
			foreach (string element : GetCountedSortedStringArray(elements))
			{
				result += "\n** " + element;
			}
		}

		result += "\n{{!}}}";

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponContainer
	//! \param[in] magazineWells
	//! \return
	static string GetWeaponDetails(notnull BaseContainer weaponContainer, map<typename, ref array<ResourceName>> magazineWells)
	{
		if (!SCR_BIKIGeneralHelper.IsWeapon(weaponContainer))
			return string.Empty;

		string result;
		float mass = SCR_BIKIWeaponHelper.GetMass(weaponContainer);
		if (mass > 0)
			result += "\n* Mass: " + SCR_BIKIStringBuilder.FormatMass(mass);

		float maxRateOfFire = SCR_BIKIWeaponHelper.GetMaxRateOfFire(weaponContainer);
		if (maxRateOfFire > 0)
			result += "\n* Max. rate of fire: " + maxRateOfFire + "&nbsp;rounds/s";

		array<string> fireModes = SCR_BIKIWeaponHelper.GetFireModes(weaponContainer);
		if (!fireModes.IsEmpty())
			result += "\n* Fire modes: " + SCR_StringHelper.Join(", ", fireModes);

		array<string> magazineNames;
		ResourceName defaultMagazine = SCR_BIKIWeaponHelper.GetDefaultMagazine(weaponContainer);
		if (defaultMagazine)
			magazineNames = { SCR_BIKIGeneralHelper.GetName(defaultMagazine) };
		else
			magazineNames = {};

		if (magazineWells)
		{
			typename magazineWell = SCR_BIKIWeaponHelper.GetMagazineWell(weaponContainer);
			if (magazineWell && magazineWells.Contains(magazineWell))
			{
				foreach (ResourceName magazineWellMag : magazineWells.Get(magazineWell))
				{
					string magazineName = SCR_BIKIGeneralHelper.GetName(magazineWellMag);
					if (!magazineNames.Contains(magazineName))
						magazineNames.Insert(magazineName);
				}
			}
		}

		int count = magazineNames.Count();
		if (count == 1)
		{
			result += "\n* Compatible magazine: " + magazineNames[0];
		}
		else
		if (count > 1)
		{
			if (defaultMagazine) // always first then
			{
				result += "\n* Compatible magazines:\n** " + magazineNames[0];
				magazineNames.RemoveOrdered(0);
			}

			magazineNames.Sort();
			result += "\n** " + SCR_StringHelper.Join("\n** ", magazineNames);
		}

		array<string> attachments = SCR_BIKIWeaponHelper.GetAttachmentNames(weaponContainer);
		count = attachments.Count();
		if (count == 1)
		{
			result += "\n* Attachment: " + attachments[0];
		}
		else
		if (count > 1)
		{
			attachments.Sort();
			result += "\n* Attachments:\n** " + SCR_StringHelper.Join("\n** ", attachments);
		}

		if (defaultMagazine) // default bullet stats
		{
			Resource resource = Resource.Load(defaultMagazine);
			if (resource.IsValid())
			{
				ResourceName firstBullet = SCR_BIKIMagazineHelper.GetFirstBullet(resource.GetResource().ToBaseContainer());
				if (firstBullet)
				{
					resource = Resource.Load(firstBullet);
					if (resource.IsValid())
					{
						BaseContainer bulletContainer = resource.GetResource().ToBaseContainer();
						float defaultBulletVelocity = SCR_BIKIBulletHelper.GetInitialSpeed(bulletContainer);
						if (defaultBulletVelocity > 0)
						{
							float weaponBulletSpeedCoefficient = SCR_BIKIWeaponHelper.GetInitialSpeedCoefficient(weaponContainer);
							if (weaponBulletSpeedCoefficient <= 0)
								weaponBulletSpeedCoefficient = 1;

							float finalBulletVelocity = defaultBulletVelocity * weaponBulletSpeedCoefficient;
							result += "\n* Initial velocity: " + SCR_BIKIStringBuilder.FormatSpeedMPS(finalBulletVelocity);
						}
					}
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] magazines
	//! \return
	static map<typename, ref array<ResourceName>> GetMagazineWells(notnull array<ResourceName> magazines)
	{
		if (magazines.IsEmpty())
			return new map<typename, ref array<ResourceName>>();

		map<typename, ref array<ResourceName>> result = new map<typename, ref array<ResourceName>>();

		Resource resource;
		BaseContainer magazineContainer;
		IEntityComponentSource magazineComponentSource;
		foreach (ResourceName magazine : magazines)
		{
			resource = Resource.Load(magazine);
			if (!resource.IsValid())
				continue;

			magazineContainer = resource.GetResource().ToBaseContainer();
			if (!magazineContainer)
				continue;

			magazineComponentSource = SCR_BaseContainerTools.FindComponentSource(magazineContainer, MagazineComponent);
			if (!magazineComponentSource)
				continue;

			magazineContainer = magazineComponentSource.GetObject("MagazineWell"); // variable reuse
			if (!magazineContainer)
				continue;

			typename typeName = magazineContainer.GetClassName().ToType();
			if (!typeName)
				continue;

			if (result.Contains(typeName))
				result.Get(typeName).Insert(magazine);
			else
				result.Insert(typeName, { magazine });
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetGroupDetails(notnull BaseContainer baseContainer)
	{
		array<ResourceName> units;
		if (!baseContainer.Get("m_aUnitPrefabSlots", units))
			return string.Empty;

		string result;
		foreach (string entry : GetCountedSortedStringArray(units))
		{
			result += "\n* " + entry;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetWheeledDetails(notnull BaseContainer baseContainer)
	{
		string result;

		result += GetMassInfo(baseContainer);
		result += GetEnginePowerInfo(baseContainer);

		result += "\n* Max. theoretical speed: approx. "
			+ SCR_BIKIStringBuilder.FormatSpeedKMH(SCR_BIKIVehicleHelper.GetTheoreticalMaxSpeed(baseContainer) * MAX_SPEED_MULTIPLIER);

		result += GetGearboxInfo(baseContainer);
		result += GetFuelInfo(baseContainer);
		result += GetRangeInfo(baseContainer);

		result += string.Format(
			"\n* Idle/red/max RPM: %1/%2/%3&nbsp;RPM",
			SCR_BIKIVehicleHelper.GetIdleRPM(baseContainer),
			SCR_BIKIVehicleHelper.GetRPMRedLine(baseContainer),
			SCR_BIKIVehicleHelper.GetMaxRPM(baseContainer));

		result += GetSeatsInfo(baseContainer);
		result += GetTurretsInfo(baseContainer);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetTrackedDetails(notnull BaseContainer baseContainer)
	{
		string result;

		result += GetMassInfo(baseContainer);
		result += GetEnginePowerInfo(baseContainer);

		result += "\n* Max. theoretical speed: approx. "
			+ SCR_BIKIStringBuilder.FormatSpeedKMH(SCR_BIKIVehicleHelper.GetTheoreticalMaxSpeed(baseContainer) * MAX_SPEED_MULTIPLIER);

		result += GetGearboxInfo(baseContainer);
		result += GetFuelInfo(baseContainer);
		result += GetRangeInfo(baseContainer);

		result += string.Format(
			"\n* Idle/red/max RPM: %1/%2/%3&nbsp;RPM",
			SCR_BIKIVehicleHelper.GetIdleRPM(baseContainer),
			SCR_BIKIVehicleHelper.GetRPMRedLine(baseContainer),
			SCR_BIKIVehicleHelper.GetMaxRPM(baseContainer));

		result += GetSeatsInfo(baseContainer);
		result += GetTurretsInfo(baseContainer);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetHelicopterDetails(notnull BaseContainer baseContainer)
	{
		string result;

		result += GetMassInfo(baseContainer);
		result += GetEnginePowerInfo(baseContainer);

//		result += "\n* Max. theoretical speed: approx. "
//			+ SCR_BIKIStringBuilder.FormatKMH(SCR_BIKIVehicleHelper.GetTheoreticalMaxSpeed(baseContainer) * MAX_SPEED_MULTIPLIER);

		result += GetFuelInfo(baseContainer);
//		result += GetRangeInfo(baseContainer);

		result += string.Format(
			"\n* Idle/max RPM: %1/%2&nbsp;RPM",
			SCR_BIKIVehicleHelper.GetIdleRPM(baseContainer),
			SCR_BIKIVehicleHelper.GetMaxRPM(baseContainer));

		result += GetSeatsInfo(baseContainer, true);
		result += GetTurretsInfo(baseContainer);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetStaticWeaponDetails(notnull BaseContainer baseContainer)
	{
		string result;

		ResourceName weaponResourceName = SCR_BIKIWeaponHelper.GetStaticWeaponWeapon(baseContainer);
		if (weaponResourceName) // !IsEmpty()
			result += "\n* Mounted weapon: " + SCR_BIKIGeneralHelper.GetName(weaponResourceName);

		array<float> angles = SCR_BIKIWeaponHelper.GetStaticWeaponAngles(baseContainer);
		if (angles[0] != 0 || angles[1] != 0 || angles[2] != 0 || angles[3] != 0)
		{
			result += "\n* Angles:";
			result += string.Format("\n** from %1 to %2° (%3° horizontal range)", angles[0], angles[1], angles[1] - angles[0]);
			result += string.Format("\n** from %1 to %2° (%3° vertical range)", angles[2], angles[3], angles[3] - angles[2]);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// Get information (get fields as list elements)
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetMassInfo(notnull BaseContainer baseContainer)
	{
		return "\n* Mass: " + SCR_BIKIStringBuilder.FormatMass(SCR_BIKIGeneralHelper.GetMass(baseContainer));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetEnginePowerInfo(notnull BaseContainer baseContainer)
	{
		float powerKW = SCR_BIKIVehicleHelper.GetEnginePowerKW(baseContainer);
		return string.Format(
			"\n* Horsepower: %1&nbsp;HP / %2&nbsp;PS",
			Math.Round(powerKW * SCR_BIKIVehicleHelper.KW2HP),
			Math.Round(powerKW * SCR_BIKIVehicleHelper.KW2PS));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetFuelInfo(notnull BaseContainer baseContainer)
	{
		float fuelCapacity = SCR_BIKIVehicleHelper.GetFuelCapacity(baseContainer);
		if (fuelCapacity <= 0)
			return string.Empty;

		float initialFuelQuantity = SCR_BIKIVehicleHelper.GetInitialFuelQuantity(baseContainer);
		array<string> fuelTypes = SCR_BIKIVehicleHelper.GetFuelTypesAsString(baseContainer);

		return string.Format(
			"\n* Fuel capacity: %1/%2&nbsp;L (%3%% fill, %4)",
			initialFuelQuantity,
			fuelCapacity,
			(initialFuelQuantity / fuelCapacity * 100).ToString(-1, 0),
			SCR_StringHelper.Join(", ", fuelTypes));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetRangeInfo(notnull BaseContainer baseContainer)
	{
		float totalFuel = SCR_BIKIVehicleHelper.GetFuelCapacity(baseContainer);
		if (totalFuel <= 0)
			return string.Empty;

		float minConsumption = SCR_BIKIVehicleHelper.GetFuelIdleConsumption(baseContainer);
		float maxConsumption = SCR_BIKIVehicleHelper.GetFuelMaxConsumption(baseContainer);

		float redZoneLerp = Math.InverseLerp(
			0,
			SCR_BIKIVehicleHelper.GetMaxRPM(baseContainer),
			SCR_BIKIVehicleHelper.GetRPMRedLine(baseContainer));

		if (redZoneLerp <= 0)
			redZoneLerp = 1;

		float consumptionRatio = maxConsumption * redZoneLerp * RANGE_FULL_RPM_RATIO + minConsumption * (1 - RANGE_FULL_RPM_RATIO);
		if (consumptionRatio <= 0)
			return string.Empty;

		float timeRange = totalFuel / consumptionRatio;
		if (timeRange <= 0)
			return string.Empty;

		float maxSpeed = SCR_BIKIVehicleHelper.GetTheoreticalMaxSpeed(baseContainer) * MAX_SPEED_MULTIPLIER;
		float distanceRange = Math.Round(maxSpeed * timeRange * 0.1) * 10;						// rounded to the 10km

		string time = SCR_FormatHelper.FormatTime(Math.Round(timeRange * 4) * 0.25 * 3600);		// rounded to the quarter
		if (time[0] == "0")
			time = time.Substring(1, time.Length() - 4) + "m";
		else
			time = time.Substring(0, time.Length() - 3) + "m";

		time.Replace(":", "h");
		if (distanceRange <= 0)
			return "\n* Estimated range: " + time;

		return string.Format("\n* Estimated range: %1&nbsp;km in %2", distanceRange, time);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetGearboxInfo(notnull BaseContainer baseContainer)
	{
		string result;
		if (SCR_BIKIVehicleHelper.GetAutomaticGearbox(baseContainer))
			result = "\n* Automatic gearbox";
		else
			result = "\n* Manual gearbox";

		array<float> gearRatios = SCR_BIKIVehicleHelper.GetGearRatios(baseContainer);
		if (gearRatios.IsEmpty())
			return result;

		int forwardCount, reverseCount;
		foreach (float gearRatio : gearRatios)
		{
			if (gearRatio > 0)
				++forwardCount;
			else // we avoid 0
			if (gearRatio < 0)
				++reverseCount;
		}

		return result + string.Format(": %1 forward, %2 reverse gears", forwardCount, reverseCount);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \param[in] isPilot true to use "pilot" instead of "driver"
	//! \return
	protected static string GetSeatsInfo(notnull BaseContainer baseContainer, bool isPilot = false)
	{
		string result;

		array<int> seats = SCR_BIKIVehicleHelper.GetSeats(baseContainer);
		int totalCount = seats[0] + seats[1] + seats[2] + seats[3];
		if (totalCount > 0)
		{
			string driverPilot;
			if (isPilot)
				driverPilot = "pilot";
			else
				driverPilot = "driver";

			result += "\n* Capacity: " + totalCount + " persons (";
			array<string> seatDescriptions = {};
			if (seats[0] == 1)
				seatDescriptions.Insert(seats[0].ToString() + " " + driverPilot);
			else
			if (seats[0] > 1)
				seatDescriptions.Insert(seats[0].ToString() + " " + driverPilot + "s");

			if (seats[1] == 1)
				seatDescriptions.Insert(seats[1].ToString() + " gunner");
			else
			if (seats[1] > 1)
				seatDescriptions.Insert(seats[1].ToString() + " gunners");

			if (seats[2] > 0)
				seatDescriptions.Insert(seats[2].ToString() + " commander");

			if (seats[3] > 0)
				seatDescriptions.Insert(seats[3].ToString() + " cargo");

			result += SCR_StringHelper.Join(", ", seatDescriptions) + ")";
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseContainer
	//! \return
	protected static string GetTurretsInfo(notnull BaseContainer baseContainer)
	{
		array<string> turrets = SCR_BIKIWeaponHelper.GetAllTurretNames(baseContainer);
		int count = turrets.Count();
		if (count < 1)
			return string.Empty;

		if (count == 1)
			return "\n* Turret: " + turrets[0];

		turrets = GetCountedSortedStringArray(turrets);
		count = turrets.Count();
		if (count == 1)
			return "\n* Turrets: " + turrets[0];

		return "\n* Turrets:\n** " + SCR_StringHelper.Join("\n** ", GetCountedSortedStringArray(turrets));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceNames
	//! \return
	protected static array<string> GetCountedSortedStringArray(notnull array<ResourceName> resourceNames)
	{
		array<string> result = {};
		foreach (ResourceName resourceName : resourceNames)
		{
			string name = SCR_BIKIGeneralHelper.GetName(resourceName);
			if (name)
				result.Insert(SCR_BIKIGeneralHelper.GetName(resourceName));
			else
				result.Insert(FilePath.StripPath(resourceName));
		}

		return GetCountedSortedStringArray(result);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] elements
	//! \return
	protected static array<string> GetCountedSortedStringArray(notnull array<string> elements)
	{
		map<string, int> elementsAndCount = new map<string, int>();
		array<string> sortedElements = {};
		foreach (string element : elements)
		{
			if (elementsAndCount.Contains(element))
			{
				elementsAndCount.Set(element, elementsAndCount.Get(element) + 1);
			}
			else
			{
				sortedElements.Insert(element);
				elementsAndCount.Insert(element, 1);
			}
		}

		sortedElements.Sort();

		array<string> result = {};
		foreach (string sortedElement : sortedElements)
		{
			int count = elementsAndCount.Get(sortedElement);
			if (count > 1)
				result.Insert(count.ToString() + "&times; " + sortedElement);
			else
				result.Insert(sortedElement);
		}

		return result;
	}
}
#endif // WORKBENCH
