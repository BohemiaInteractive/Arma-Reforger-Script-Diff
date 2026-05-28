#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: PLUGIN_NAME,
	description: "Exports terrain statistics (https:/" + "/community.bistudio.com/wiki/Category:Arma_Reforger/Content/Terrains)",
	wbModules: { "ResourceManager", "WorldEditor" },
	category: SCR_PluginCategory.BIKI,
	awesomeFontCode: 0xF70E)]
class SCR_BIKIExportTerrainStatisticsPlugin : SCR_BIKIExportBasePlugin
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Terrain measurement step", params: "0.01 100 0.01")]
	protected float m_fTerrainMeasurementResolution;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, desc: "Radius around the position in which to search for a Map Descriptor (e.g hill name, city name etc)", params: "0 1000 1")]
	protected float m_fMapDescriptorSearchRadius;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "Enable advanced entity statistics (forests total area, total road length, number of lakes/rivers etc)", params: "0 1000 1")]
	protected bool m_bEnableEntityStatistics;

	// imperial units, like stormtroopers or something
//	protected static const float KMH2MPH = 1 / 1.60934;	//!< KM/H to MP/H
//	protected static const float M2FT = 1 / 0.3048;		//!< Metres to Feet

	protected static const string KEY_FORESTAREA = "forestArea";
//	protected static const string KEY_TREESCOUNT = "treesCount";

	protected static const string KEY_LAKESCOUNT = "lakesCount";

	protected static const string KEY_RIVERSCOUNT = "riversCount";
	protected static const string KEY_RIVERSLENGTH = "riversLength";
	protected static const string KEY_LONGESTRIVER = "longestRiver";

	protected static const string KEY_ROADSCOUNT = "roadsCount";
	protected static const string KEY_ROADSLENGTH = "roadsLength";
//	protected static const string KEY_MAINROADSLENGTH = "mainRoadsLength"; // most roads in Kolguyev are missing "GeneratorTypeRoad" marker
	protected static const string KEY_LONGESTROAD = "longestRoad";

	protected static const string PLUGIN_NAME = "Export Terrain Statistics";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();

		string worldPath;
		bool needToLoad;
		if (worldEditorAPI)
		{
			worldEditorAPI.GetWorldPath(worldPath);
			needToLoad = !worldPath.EndsWith(".ent");
		}
		else
		{
			needToLoad = true;
		}

		array<ResourceName> selectedEntFiles = {};

		array<ResourceName> selectedFiles = {};
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		resourceManager.GetResourceBrowserSelection(selectedFiles.Insert, false);

		foreach (ResourceName selectedResource : selectedFiles)
		{
			if (selectedResource.EndsWith(".ent"))
				selectedEntFiles.Insert(selectedResource);
		}

		if (needToLoad)
		{
			int count = selectedEntFiles.Count();
			if (count != 1)
			{
				if (count < 1)
					SCR_WorkbenchHelper.PrintDialog("No world selected/loaded. Please load a world or select the world's .ent file in Resource Browser.", PLUGIN_NAME, LogLevel.WARNING);
				else
					SCR_WorkbenchHelper.PrintDialog(count.ToString() + " .ent files selected. Please select only one.", PLUGIN_NAME, LogLevel.WARNING);

				return;
			}

			if (!Workbench.OpenModule(WorldEditor))
			{
				SCR_WorkbenchHelper.PrintDialog("Cannot open World Editor", PLUGIN_NAME, LogLevel.ERROR);
				return;
			}

			worldEditorAPI = worldEditor.GetApi();

			if (Workbench.ScriptDialog(PLUGIN_NAME, "You are about to load " + selectedEntFiles[0] + " - continue?", new WorkbenchDialog_OKCancel()) == 0)
				return;

			if (!worldEditor.SetOpenedResource(selectedEntFiles[0]))
			{
				SCR_WorkbenchHelper.PrintDialog("Cannot open " + selectedEntFiles[0], PLUGIN_NAME, LogLevel.ERROR);
				return;
			}
		}
		else
		{
			if (selectedEntFiles.Count() == 1)
			{
				string selectedAbsPath;
				bool showLoadProcessCancel = !Workbench.GetAbsolutePath(selectedEntFiles[0].GetPath(), selectedAbsPath);
				if (!showLoadProcessCancel)
				{
					string loadedAbsPath;
					showLoadProcessCancel = !Workbench.GetAbsolutePath(worldPath, loadedAbsPath);

					if (!showLoadProcessCancel)
					{
						selectedAbsPath.ToLower();
						loadedAbsPath.ToLower();
						showLoadProcessCancel = selectedAbsPath != loadedAbsPath;
					}
				}

				if (showLoadProcessCancel)
				{
					SCR_BIKIExportTerrainStatisticsPlugin_LoadProcessCancel choice = new SCR_BIKIExportTerrainStatisticsPlugin_LoadProcessCancel();
					if (Workbench.ScriptDialog(
						PLUGIN_NAME,
						string.Format("Do you want to load the selected world (%1)\nor process the currently-opened one (%2)?", selectedEntFiles[0], worldPath),
						choice) == 0)
						return;

					if (choice.m_iResult == 2)
					{
						worldEditor.SetOpenedResource(selectedEntFiles[0]);
						worldEditorAPI.GetWorldPath(worldPath);
					}
				}
			}
		}

		if (Workbench.ScriptDialog(PLUGIN_NAME, "", this) == 0)
			return;

		string result = GetTerrainInfobox();

		ShowResult(result);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetTerrainInfobox()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);

		SCR_BIKIStringBuilder stringBuilder = new SCR_BIKIStringBuilder("{{Infobox/Terrain");

		//
		// basic intel
		//

		string worldNameUnderscoreLC = FilePath.StripExtension(FilePath.StripPath(worldPath));
		worldNameUnderscoreLC.ToLower();
		worldNameUnderscoreLC.Replace(" ", "_");
		string worldNamePretty = SCR_StringHelper.FormatSnakeCaseToUserFriendly(worldNameUnderscoreLC);

		// name
		stringBuilder.AddKeyValueLine("name", worldNamePretty);

		// wbLink
		stringBuilder.AddKeyValueLine("wbLink", "enfusion:/" + "/WorldEditor/" + SCR_AddonTool.StripFileSystem(worldPath));

		// image
		stringBuilder.AddKeyValueLine("image", "[[File:armaR-" + worldNameUnderscoreLC + " _preview.png|300px]]");

		// defaultDateTime
		// season
		// coords
		string defaultDateTime;
		string season;
		string coords;
		GetTimeAndWeatherInfo(defaultDateTime, season, coords);

		// defaultDateTime
		stringBuilder.AddKeyValueLine("defaultDateTime", defaultDateTime);

		// season
		stringBuilder.AddKeyValueLine("season", season);

		// coords
		stringBuilder.AddKeyValueLine("coords", coords);

		//
		// advanced intel
		//

		Debug.BeginTimeMeasure();
		float landSurface = SCR_WorldEditorToolHelper.GetTerrainLandArea();
		Debug.EndTimeMeasure(string.Format("Getting land surface for %1: %2 km² with a %3m step", worldPath, (landSurface * 0.000001).ToString(lenDec: 2), m_fTerrainMeasurementResolution));

		vector terrainDimensions = SCR_WorldEditorToolHelper.GetTerrainDimensions();
		float terrainSurface = terrainDimensions[0] * terrainDimensions[2];

		// landArea
		stringBuilder.AddKeyValueLine(
			"landArea",
			string.Format(
				"<span title=\"%2 measurement precision\">%1</span>",
				FormatSurface(landSurface),
				FormatDistance(m_fTerrainMeasurementResolution)));

		// totalArea
		stringBuilder.AddKeyValueLine("totalArea", FormatSurface(terrainSurface));

		vector minElevationPos, maxElevationPos;
		SCR_WorldEditorToolHelper.GetTerrainMinMaxElevationPositions(minElevationPos, maxElevationPos);

		// maxElevation
		string locationName = GetClosestLocationName(maxElevationPos, m_fMapDescriptorSearchRadius);
		if (locationName)
			stringBuilder.AddKeyValueLine("maxElevation", string.Format("%1 (%2)", FormatDistance(maxElevationPos[1]), SCR_StringHelper.UCFirstAll(locationName, true)));
		else
			stringBuilder.AddKeyValueLine("maxElevation", FormatDistance(maxElevationPos[1]));

		// minElevation
		locationName = GetClosestLocationName(minElevationPos, m_fMapDescriptorSearchRadius);
		if (locationName)
			stringBuilder.AddKeyValueLine("minElevation", string.Format("%1 (%2)", FormatDistance(minElevationPos[1]), SCR_StringHelper.UCFirstAll(locationName, true)));
		else
			stringBuilder.AddKeyValueLine("minElevation", FormatDistance(minElevationPos[1]));

		//
		// technical intel
		//

		stringBuilder.AddKeyValueLine("dimensions", string.Format("%1 &times; %2", FormatDistance(terrainDimensions[0]), FormatDistance(terrainDimensions[2])));
		stringBuilder.AddKeyValueLine("resolution", FormatDistance(worldEditorAPI.GetTerrainUnitScale()));
		stringBuilder.AddKeyValueLine("basedOn", "{{n/a}}");

		//
		// entity stats (heavy)
		//

		if (m_bEnableEntityStatistics)
		{
			map<string, float> entityStats = GetEntityStatistics();

			foreach (string key, float value : entityStats)
			{
				if (key == KEY_FORESTAREA)
					stringBuilder.AddKeyValueLine(key, FormatSurface(value));
				else
					stringBuilder.AddKeyValueLine(key, value.ToString());
			}
		}

		stringBuilder.AddLine("}}");

		return stringBuilder.GetContent();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] defaultDateTime
	//! \param[out] season
	//! \param[out] coords
	//! \return
	protected static bool GetTimeAndWeatherInfo(out string defaultDateTime, out string season, out string coords)
	{
		IEntitySource timeAndWeatherManagerSource = SCR_WorldEditorToolHelper.GetTimeAndWeatherManagerEntitySource();
		if (!timeAndWeatherManagerSource)
			return false;

		float latitude, longitude;
		if (timeAndWeatherManagerSource.Get("Latitude", latitude) && timeAndWeatherManagerSource.Get("Longitude", longitude))
		{
			string ns, ew;
			if (latitude < 0)
			{
				latitude = -latitude;
				ns = "S";
			}
			else
			{
				ns = "N";
			}

			if (longitude < 0)
			{
				longitude = -longitude;
				ew = "W";
			}
			else
			{
				ew = "E";
			}

			coords = string.Format(
				"{{Link|https:/" + "/www.google.com/maps/?q{{=}}%1%%20%2%%20%3%%20%4|%1&nbsp;%2<br>%3&nbsp;%4}}",
				latitude,
				ns,
				longitude,
				ew);
		}

		int year, month, day;
		if (timeAndWeatherManagerSource.Get("Year", year)
			&& timeAndWeatherManagerSource.Get("Month of the Year", month)
			&& timeAndWeatherManagerSource.Get("Day of the Month", day))
		{
			defaultDateTime = SCR_FormatHelper.FormatDate(year, month, day);

			// 1989 season dates
			// 1989-03-20 = Spring = day 79
			// 1989-06-21 = Summer = day 172
			// 1989-09-23 = Autumn = day 266
			// 1989-12-21 = Winter = day 355
			int dayNumber = SCR_DateTimeHelper.GetDayNumber(year, month, day);
			if (dayNumber < 1)
				season = string.Empty;
			else
			if (dayNumber < 79)
				season = "Winter";
			else
			if (dayNumber < 172)
				season = "Spring";
			else
			if (dayNumber < 266)
				season = "Summer";
			else
			if (dayNumber < 355)
				season = "Autumn";
			else
				season = "Winter";
		}

		float dayTime;
		if (timeAndWeatherManagerSource.Get("Initial Day Time", dayTime))
			defaultDateTime += " " + SCR_FormatHelper.FormatTime(dayTime * 3600); // .Substring(0, 5); // removing seconds

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] worldPos
	//! \param[in] searchRadius
	//! \return
	protected static string GetClosestLocationName(vector worldPos, float searchRadius)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		BaseWorld world = worldEditorAPI.GetWorld();

		if (searchRadius < 0)
			searchRadius = -searchRadius;

		vector min = worldPos - searchRadius * vector.One;
		vector max = worldPos + searchRadius * vector.One;
		min[1] = -float.MAX;
		max[1] = float.MAX;

		set<IEntity> searchResults = new set<IEntity>();
		if (!world.QueryEntitiesByAABB(min, max, searchResults.Insert, null, EQueryEntitiesFlags.STATIC))
		{
			PrintFormat("Error querying entities by AABB at %1 (radius %2) with <%3, %4>", worldPos, searchRadius, min, max, level: LogLevel.WARNING);
			return string.Empty;
		}

		int count = searchResults.Count();
		if (count < 1)
			return string.Empty;

		string result;
		SCR_MapDescriptorComponent mapDescriptorComponent;
		IEntitySource entitySource;
		BaseContainerList componentSources;
		IEntityComponentSource componentSource;
		float currDistanceSqXZ;
		if (searchRadius > 0)
			currDistanceSqXZ = searchRadius * searchRadius;

		foreach (IEntity entity : searchResults)
		{
			float entityDistanceSqXZ;
			if (searchRadius > 0)
			{
				entityDistanceSqXZ = vector.DistanceSqXZ(worldPos, entity.GetOrigin());
				if (entityDistanceSqXZ >= currDistanceSqXZ)
					continue;
			}

			entitySource = worldEditorAPI.EntityToSource(entity);
			if (!entitySource)
				continue;

			componentSource = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_MapDescriptorComponent);
			if (!componentSource)
				continue;

			if (!componentSource.Get("DisplayName", result))
				continue;

			currDistanceSqXZ = entityDistanceSqXZ;
		}

		if (result.Contains("#"))
			result = WidgetManager.Translate(result);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static map<string, float> GetEntityStatistics()
	{
		// TODO: get house count, get forest surface, get road km length

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		int entitiesCount = worldEditorAPI.GetEditorEntityCount();
		if (entitiesCount < 3) // world or world + terrain
			return null;

		map<string, float> result = new map<string, float>();

		IEntitySource entitySource;
		IEntitySource childSource;
		typename typeName;
		typename childTypeName;
		BaseContainer baseContainer;
		ShapeEntity shapeEntity;
		SCR_ShapeNextPointHelper shapeNextPointHelper;
		array<string> generatorTypes = {};
		for (int i; i < entitiesCount; ++i)
		{
			entitySource = worldEditorAPI.GetEditorEntity(i);
			typeName = entitySource.GetClassName().ToType();
			if (!typeName)
				continue;

			bool isPolyline = typeName.IsInherited(PolylineShapeEntity);
			bool isSpline = typeName.IsInherited(SplineShapeEntity);

			if (!isPolyline && !isSpline)
				continue;

			bool isForest, isLake, isRiver, isRoad;

			generatorTypes.Clear();
			for (int childIndex, childrenCount = entitySource.GetNumChildren(); childIndex < childrenCount; ++childIndex)
			{
				childSource = entitySource.GetChild(childIndex);
				childTypeName = childSource.GetClassName().ToType();
				if (!childTypeName)
					continue;

				baseContainer = childSource.GetObject("GeneratorType");
				string generatorType;
				if (baseContainer)
				{
					generatorType = baseContainer.GetClassName();
					if (generatorType)
						generatorTypes.Insert(generatorType);
				}

				if (!isForest && isPolyline && childTypeName.IsInherited(ForestGeneratorEntity))
				{
					if (generatorType == "GeneratorTypeForest") // avoid rock generators
						isForest = true;
				}

				if (!isLake && childTypeName.IsInherited(LakeGeneratorEntity))
					childSource.Get("m_bIsLake", isLake);

				if (!isRiver && childTypeName.IsInherited(RiverEntity))
					isRiver = true;

				if (!isRoad && childTypeName.IsInherited(RoadGeneratorEntity))
					isRoad = true;
			}

			if (isForest)
			{
				AddToMapResult(result, KEY_FORESTAREA, GetPolylineShapeArea(entitySource));
//				AddToMapResult(result, KEY_TREESCOUNT, entitySource.GetNumChildren()); // could be trees, bushes, rocks, etc
			}

			if (isLake)
				AddToMapResult(result, KEY_LAKESCOUNT, 1);

			if (isRiver)
			{
				AddToMapResult(result, KEY_RIVERSCOUNT, 1);
				float shapeLength = GetShapeLength(entitySource);
				AddToMapResult(result, KEY_RIVERSLENGTH, shapeLength);
				SetBiggestValue(result, KEY_LONGESTRIVER, shapeLength);
			}

			if (isRoad)
			{
				AddToMapResult(result, KEY_ROADSCOUNT, 1);
				float shapeLength = GetShapeLength(entitySource);
//				if (generatorTypes.Contains("GeneratorTypeRoad"))
//					AddToMapResult(result, KEY_MAINROADSLENGTH, shapeLength);

				AddToMapResult(result, KEY_ROADSLENGTH, shapeLength);
				SetBiggestValue(result, KEY_LONGESTROAD, shapeLength);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected static void AddToMapResult(notnull map<string, float> result, string key, float value)
	{
		float currentValue;
		if (result.Find(key, currentValue))
			result.Set(key, currentValue + value);
		else
			result.Insert(key, value);
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetBiggestValue(notnull map<string, float> result, string key, float value)
	{
		float currentValue;
		if (result.Find(key, currentValue))
		{
			if (currentValue < value)
				result.Set(key, value);
		}
		else
		{
			result.Insert(key, value);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static float GetPolylineShapeArea(notnull IEntitySource shapeEntitySource)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		PolylineShapeEntity polylineShapeEntity = PolylineShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeEntitySource));
		if (!polylineShapeEntity)
			return 0;

		array<vector> points = {};
		polylineShapeEntity.GetPointsPositions(points);
		if (points.Count() < 3)
			return 0;

		array<float> pointsXZ = {};
		foreach (vector point : points)
		{
			pointsXZ.Insert(point[0]);
			pointsXZ.Insert(point[2]);
		}

		return SCR_Math2D.GetPolygonArea(pointsXZ);
	}

	//------------------------------------------------------------------------------------------------
	protected static float GetShapeLength(notnull IEntitySource shapeEntitySource)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeEntitySource));
		if (!shapeEntity)
			return 0;

		SCR_ShapeNextPointHelper shapeNextPointHelper = SCR_ShapeNextPointHelper.CreateFromShape(shapeEntity);
		if (!shapeNextPointHelper)
			return 0;

		return shapeNextPointHelper.GetShapeLength();
	}

	//------------------------------------------------------------------------------------------------
	protected string FormatDistance(float distanceM)
	{
		if (distanceM > 10000) // more than 10000m = go km
			return SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(distanceM * 0.001, 2) + "&nbsp;km";
		else
			return SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(distanceM, 2) + "m";
	}

	//------------------------------------------------------------------------------------------------
	protected string FormatSurface(float surfaceM2)
	{
		if (surfaceM2 > 10000) // more than 10000m² = go km²
			return SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(surfaceM2 * 0.000001, 2) + "&nbsp;km&sup2;";
		else
			return SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(surfaceM2, 2) + "&nbsp;m&sup2;";
	}
}

class SCR_BIKIExportTerrainStatisticsPlugin_LoadProcessCancel
{
	//! 0 = cancel, 1 = process loaded terrain, 2 = load selected terrain
	int m_iResult;

	[ButtonAttribute("Load terrain")]
	protected int ButtonLoad()
	{
		m_iResult = 2;
		return 1;
	}

	[ButtonAttribute("Process loaded terrain", true)]
	protected int ButtonProcess()
	{
		m_iResult = 1;
		return 1;
	}

	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}
#endif
