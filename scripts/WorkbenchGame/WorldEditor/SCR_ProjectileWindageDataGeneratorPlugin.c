#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Generate projectile wind data", description: "", wbModules: { "WorldEditor" }, awesomeFontCode: 0xf72e)]
class SCR_ProjectileWindageDataGeneratorPlugin : WorldEditorPlugin
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Projectiles", params: "et")]
	protected ref TResourceNameArray m_aProjectiles;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Override config files to which generated data will be saved.\nIf left empty, or number of entries is less than number of entries in [Projectiles] list, then system will try to fetch the config from the projectile prefab data.", params: "conf class=SCR_ProjectileWindTable")]
	protected ref TResourceNameArray m_aConfigsOverride;

	[Attribute(defvalue: "10", desc: "Wind Speeds", params: "0.01 inf")]
	protected ref TFloatArray m_aWindSpeeds;

	[Attribute(defvalue: "1", desc: "Init speed coefs for which data should be generated.\nWhen this array is left empty, then system will use init speed coef = 1, and in case of mortar shells, it will fetch their values from SCR_MortarShellGadgetComponent.ChargeRingConfig", params: "0.01 inf")]
	protected ref TFloatArray m_aInitSpeedCoefs;

	[Attribute(defvalue: "10000", desc: "Max Simulated Distance[m]", params: "1 inf 0.01")]
	protected float m_fMaxSimulatedHorizontalDistance;

	[Attribute(defvalue: "60", desc: "Maximum simulated time for the projectile’s flight. When simulated time will reach this value, then simulation will be stopped even when projectile has not reached its end of travel", params: "1 inf 0.01")]
	protected float m_fMaxSimulationTime;

	[Attribute(defvalue: "3", desc: "Peak Search Precision[m]", params: "0.1 inf 0.01")]
	protected float m_fPeakSearchPrecision;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (Workbench.ScriptDialog("Configuration", "Select prefabs and their coresponding configs.", this) == 0)
			return;

		if (!m_aProjectiles || m_aProjectiles.IsEmpty())
			return;

		GenerateData();
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns provided entity in the editor
	//! \param[in] api World Editor API
	//! \param[in] resourceName prefab to spawn
	//! \param[out] entitySource
	//! \return entity that was spawned, or null
	protected IEntity SpawnEntity(notnull WorldEditorAPI api, ResourceName resourceName, out IEntitySource entitySource = null)
	{
		Resource res = Resource.Load(resourceName);
		if (!res.IsValid())
			return null;

		entitySource = res.GetResource().ToEntitySource();
		if (!entitySource)
			return null;

		BaseWorld world = api.GetWorld();
		return GetGame().SpawnEntityPrefab(res, world);
	}

	//------------------------------------------------------------------------------------------------
	//! Finds the initial velocity of the projectile
	//! \param[in] entitySource
	//! \return inital velocity of the projectile
	protected float GetInitSpeed(notnull IEntitySource entitySource)
	{
		IEntityComponentSource componentSrc;
		for (int i, componentsCount = entitySource.GetComponentCount(); i < componentsCount; i++)
		{
			componentSrc = entitySource.GetComponent(i);
			if (!componentSrc)
				continue;

			if (!componentSrc.GetClassName().ToType().IsInherited(ProjectileMoveComponent))
				continue;

			float initSpeed;
			componentSrc.Get("InitSpeed", initSpeed);
			return initSpeed;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to find specific initial speed coefs or provides the default value
	//! \param[in] projectile
	//! \return list of initial speed coefs
	protected TFloatArray FetchInitSpeedCoefs(notnull IEntity projectile)
	{
		TFloatArray initSpeedCoefs = {};
		SCR_MortarShellGadgetComponent mortarShellComp = SCR_MortarShellGadgetComponent.Cast(projectile.FindComponent(SCR_MortarShellGadgetComponent));
		if (mortarShellComp)
		{
			for (int j, count = mortarShellComp.GetNumberOfChargeRingConfigurations(); j < count; j++)
			{
				initSpeedCoefs.Insert(mortarShellComp.GetChargeRingConfig(j)[1]);
			}
		}
		else
		{
			initSpeedCoefs.Insert(1);
		}

		return initSpeedCoefs;
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateData()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
			return;

		TFloatArray windSpeeds = m_aWindSpeeds;
		if (windSpeeds.IsEmpty())
			windSpeeds = {10};

		TFloatArray initSpeedCoefs = m_aInitSpeedCoefs;
		bool initCoefsOverride = !initSpeedCoefs.IsEmpty();

		TResourceNameArray configs = {};
		configs.Copy(m_aConfigsOverride);
		map<int, string> failedEntries = new map<int, string>();
		float initSpeed;
		IEntity projectile;
		ResourceName configFile;
		IEntitySource entitySource;
		SCR_ProjectileWindTable config;
		ProjectileMoveComponent moveComp;
		foreach (int i, ResourceName resourceName : m_aProjectiles)
		{
			if (projectile)
				delete projectile; // delete no longer used entity

			projectile = SpawnEntity(api, resourceName, entitySource);
			if (!projectile)
			{
				failedEntries.Set(i, "Failed to spawn projectile");
				continue;
			}

			moveComp = ProjectileMoveComponent.Cast(projectile.FindComponent(ProjectileMoveComponent));
			if (!moveComp)
			{
				failedEntries.Set(i, "Failed to find some form of ProjectileMoveComponent");
				continue;
			}

			initSpeed = GetInitSpeed(entitySource);
			if (initSpeed <= 0)
			{
				failedEntries.Set(i, "Failed to find init speed");
				continue;
			}

			if (!initCoefsOverride)
				initSpeedCoefs = FetchInitSpeedCoefs(projectile);

			if (configs.IsIndexValid(i))
				configFile = configs[i];
			else
				configFile = string.Empty;

			if (configFile.IsEmpty())
			{
				ProjectileMoveComponentClass data = ProjectileMoveComponentClass.Cast(moveComp.GetComponentData(projectile));
				configFile = data.GetProjectileWindTable();
				if (configFile.IsEmpty())
				{
					failedEntries.Set(i, "Failed to find config file");
					continue;
				}
				else
				{
					configs.Insert(configFile);
				}
			}

			config = GenerateWindData(initSpeedCoefs, initSpeed, windSpeeds, moveComp);
			Resource res = BaseContainerTools.CreateContainerFromInstance(config);
			if (res.IsValid() && !BaseContainerTools.SaveContainer(res.GetResource().ToBaseContainer(), configFile))
				failedEntries.Set(i, "Failed save generated data");
		}

		if (projectile)
			delete projectile; // delete the last prefab that was spawned

		string failedResult;
		foreach (int id, string reason : failedEntries)
		{
			failedResult += string.Format("\nID=%1 - %2 - Reason=%3", id.ToString(), FilePath.StripPath(m_aProjectiles[id]), reason);
		}

		string detailedTextAddition = ".";
		if (!failedEntries.IsEmpty())
		{
			failedResult = string.Format("%1%2%3", "Generation failed for given entries:", failedResult, "\n\n\nYou can use Ctrl+C to copy the content of this dialog.");
			detailedTextAddition = " with errors! For more inforamtion check details below."
		}

		ResourceManager resourceMgr = SCR_WorldEditorToolHelper.GetResourceManager();
		if (resourceMgr && !configs.IsEmpty())
		{
			array<string> configPaths = {};
			foreach (ResourceName resourceName : configs)
			{
				configPaths.Insert(resourceName.GetPath());
			}

			// reimport config files to ensure that workbench reloads them, so user doesnt have to reopen them to see the changes
			resourceMgr.RebuildResourceFiles(configPaths, "PC");
		}

		Workbench.Dialog("Finished", "Generation of windage data has been finished" + detailedTextAddition, failedResult);
	}

	//------------------------------------------------------------------------------------------------
	//! Generates a container with wind data
	//! \param[in] initSpeedCoefs list of initial speed coefficients
	//! \param[in] initSpeed initial velocity of the projectile
	//! \param[in] windSpeeds list of speeds of the wind for which data should be generated
	//! \param[in] projectileMoveComp projectile's move component
	//! \return windage data container
	protected SCR_ProjectileWindTable GenerateWindData(notnull TFloatArray initSpeedCoefs, float initSpeed, notnull TFloatArray windSpeeds, notnull ProjectileMoveComponent projectileMoveComp)
	{
		const float minElevAngle = 1;
		const float maxElevAngle = 90;
		vector poiNoWind, poiCrosswind, poiTailwind, poiHeadwind;
		vector tailwind, headwind, crosswind;
		vector peakPoint;
		vector preImpactPosition;

		array<ref SCR_ProjectileWindData> data = {};
		TVectorArray values;
		TVectorArray firingSolutions;
		TFloatArray windData;
		float initSpeedWithCoef;
		foreach (float wind : windSpeeds)
		{
			headwind = -vector.Forward * wind;
			tailwind = vector.Forward * wind;
			crosswind = vector.Right * wind;
			foreach (float coef : initSpeedCoefs)
			{
				values = {};
				firingSolutions = {};
				initSpeedWithCoef = initSpeed * coef;
				for (int angleDeg = minElevAngle; angleDeg <= maxElevAngle; angleDeg++)
				{
					poiNoWind = projectileMoveComp.GetProjectileSimulationResult(vector.Zero, initSpeedWithCoef, angleDeg, maxSimulationTime: m_fMaxSimulationTime, maxHorizontalDistance: m_fMaxSimulatedHorizontalDistance);
					poiCrosswind = projectileMoveComp.GetProjectileSimulationResult(vector.Zero, initSpeedWithCoef, angleDeg, windSpeed: crosswind, maxSimulationTime: m_fMaxSimulationTime, maxHorizontalDistance: m_fMaxSimulatedHorizontalDistance);
					poiHeadwind = projectileMoveComp.GetProjectileSimulationResult(vector.Zero, initSpeedWithCoef, angleDeg, windSpeed: headwind, maxSimulationTime: m_fMaxSimulationTime, maxHorizontalDistance: m_fMaxSimulatedHorizontalDistance);
					poiTailwind = projectileMoveComp.GetProjectileSimulationResult(vector.Zero, initSpeedWithCoef, angleDeg, windSpeed: tailwind, maxSimulationTime: m_fMaxSimulationTime, maxHorizontalDistance: m_fMaxSimulatedHorizontalDistance);

					float angleOfImpact;
					if (poiNoWind[2] > 0)
					{
						peakPoint = FindPeakHeight(projectileMoveComp, initSpeedWithCoef, angleDeg, poiNoWind[2], m_fPeakSearchPrecision, m_fMaxSimulationTime);
						preImpactPosition = projectileMoveComp.GetProjectileSimulationResult(vector.Zero, initSpeedWithCoef, angleDeg, maxHorizontalDistance: poiNoWind[2] - 1, maxSimulationTime: m_fMaxSimulationTime);
						angleOfImpact = vector.Direction(poiNoWind, preImpactPosition).Normalized().VectorToAngles()[1];
					}
					else
					{
						peakPoint = vector.Zero;
						preImpactPosition = vector.Zero;
					}

					vector fireSolValues = {
						SCR_Math.ConvertToRadians(angleDeg, SCR_EOpticsAngleUnits.DEGREES),
						poiNoWind[2],
						peakPoint[1]
					};

					vector value = {
						SCR_Math.ConvertFromRadians(Math.Atan2(poiCrosswind[0], poiCrosswind[2]), SCR_EOpticsAngleUnits.MILLIRADIANS),
						(Math.AbsFloat(poiHeadwind[2] - poiNoWind[2]) + Math.AbsFloat(poiTailwind[2] - poiNoWind[2])) * 0.5,
						angleOfImpact
					};

					firingSolutions.Insert(fireSolValues);
					values.Insert(value);
				}

				data.Insert(new SCR_ProjectileWindData(coef, wind, values, firingSolutions));
			}
		}

		return new SCR_ProjectileWindTable(data);
	}

	//------------------------------------------------------------------------------------------------
	//! Searches for the peak altitude during projectile fly path
	//! \param[in] projectileMoveComp projectile's move component
	//! \param[in] initSpeedWithCoef inital speed of the projectile with included init speed coefs
	//! \param[in] angleDeg angle in degrees at which projectile will be fired during simulation
	//! \param[in] maxDist how far projectile has managed to reach
	//! \param[in] precision how big are the jumps in the distance when searching for the peak value
	//! \param[in] maxSimulationTime override of the max simulation time value that might be necessary for projectiles that can travel for longer than 60s
	//! \return vector with position where Y component was the highest
	protected vector FindPeakHeight(notnull ProjectileMoveComponent projectileMoveComp, float initSpeedWithCoef, float angleDeg, float maxDist, float precision = 10, float maxSimulationTime = 60)
	{
		if (precision <= 0)
			precision = 10;

		float minDist;
		float midDist;
		vector first;
		vector second;

		while (minDist < maxDist)
		{
			midDist = (minDist + maxDist) * 0.5;

			first = projectileMoveComp.GetProjectileSimulationResult(vector.Zero, initSpeedWithCoef, angleDeg, maxHorizontalDistance: midDist - precision, maxSimulationTime: maxSimulationTime);
			second = projectileMoveComp.GetProjectileSimulationResult(vector.Zero, initSpeedWithCoef, angleDeg, maxHorizontalDistance: midDist, maxSimulationTime: maxSimulationTime);

			if (first[1] > second[1])
				maxDist = midDist - precision;
			else if (first[1] <= second[1])
				minDist = midDist + precision;
			else
				break;
		}

		if (first[1] > second[1])
			return first;

		return second;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run", true)]
	protected int ButtonRun()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected int ButtonClose()
	{
		return 0;
	}
}
#endif
