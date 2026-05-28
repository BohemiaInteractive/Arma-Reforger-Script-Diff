#ifdef WORKBENCH
class SCR_WorldEditorToolHelper
{
	protected static ref array<IEntity> s_aTempEntities;

	protected static const string WEB_PREFIX = "https://enfusionengine.com/api/redirect?to=";

	//------------------------------------------------------------------------------------------------
	//! Get the ResourceManager object
	//! \return available ResourceManager or null if unavailable
	static ResourceManager GetResourceManager()
	{
		return Workbench.GetModule(ResourceManager);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the WorldEditor object
	//! \return available WorldEditor or null if unavailable
	static WorldEditor GetWorldEditor()
	{
		return Workbench.GetModule(WorldEditor);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the World Editor API
	//! \return available WorldEditorAPI or null if unavailable
	static WorldEditorAPI GetWorldEditorAPI()
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		return worldEditor.GetApi();
	}

	//------------------------------------------------------------------------------------------------
	//! Return the currently opened Prefab's entity with Edit Prefab button in Resource Manager
	//! \return the entity's source, null if error
	static IEntitySource GetPrefabEditModeEntitySource()
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		if (!worldEditor.IsPrefabEditMode())
			return null;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (worldEditorAPI.GetEditorEntityCount() != 2) // world and Entity
			return null;

		return worldEditorAPI.GetEditorEntity(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the currently opened Prefab's entity with Edit Prefab button in Resource Manager
	//! \return the entity's source, null if error
	static ResourceName GetPrefabEditModeResourceName()
	{
		IEntitySource entitySource = GetPrefabEditModeEntitySource();
		if (!entitySource)
			return string.Empty;

		BaseContainer ancestor = entitySource.GetAncestor();
		if (!ancestor)
			return string.Empty;

		return ancestor.GetResourceName();
	}

	//------------------------------------------------------------------------------------------------
	//! Get a World Editor link in Enfusion protocol format (enfusion://) using current camera's position/angles
	//! \param[in] useWebPrefix use enfusionengine.com prefix or not
	//! \return Enfusion protocol World Editor link or empty on
	static string GetCurrentWorldEditorLink(bool useWebPrefix = false)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return string.Empty;

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return string.Empty;

		vector transform[4];
		world.GetCurrentCamera(transform);

		return SCR_WorldEditorToolHelper.GetCurrentWorldEditorLink(transform[3], Math3D.MatrixToAngles(transform), useWebPrefix);
	}

	//------------------------------------------------------------------------------------------------
	//! Get a World Editor link in Enfusion protocol format (enfusion://)\n
	//! position and camera are ignored if both are 0 0 0
	//! \param[in] position camera position
	//! \param[in] angles camera angles (pitch yaw roll) (NOT direction!)
	//! \param[in] useWebPrefix use enfusionengine.com prefix or not
	//! \return Enfusion protocol World Editor link or empty on error/no world loaded
	static string GetCurrentWorldEditorLink(vector position, vector angles, bool useWebPrefix = false)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return string.Empty;

		string fullLink;
		worldEditorAPI.GetWorldPath(fullLink);

		if (fullLink.IsEmpty())
			return string.Empty;

		string worldPath = fullLink;
		if (worldPath[0] == "$")
			worldPath = "~" + worldPath.Substring(1, worldPath.Length() - 1);
		else
		if (worldPath[0] != "~")
		{
			// if we don't have an exact path (should not happen),
			// fallback to the original solution:

			// we want to substring only /worlds/(...)
			// to prevent exposing local folders, etc.
			string fullLinkLC = fullLink;
			fullLinkLC.ToLower();

			int begin = fullLinkLC.IndexOf("worlds\\");
			if (begin == -1)
				begin = fullLinkLC.IndexOf("worlds/");

			if (begin == -1)
				return string.Empty;

			worldPath = fullLink.Substring(begin, fullLink.Length() - begin);
		}

		// Have consistent link
		worldPath.Replace("\\", "/");

		if (position == vector.Zero && angles == vector.Zero)
			return "enfusion://WorldEditor/" + worldPath;

		string result = string.Format(
			"enfusion://WorldEditor/%1;%2,%3,%4;%5,%6,%7",
			worldPath,
			position[0],
			position[1],
			position[2],
			angles[1],
			angles[0],
			angles[2]);

		if (useWebPrefix)
			return WEB_PREFIX + result;
		else
			return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the currently loaded world's name, or "Unknown" if empty
	static string GetWorldName()
	{
		string result = FilePath.StripExtension(FilePath.StripPath(GetWorldPath()));
		if (!result) // .IsEmpty()
			return "Unknown";

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return empty string if World Editor API is unavailable or if the world is not saved to storage
	static string GetWorldPath()
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return string.Empty;

		string result;
		worldEditorAPI.GetWorldPath(result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if a world is loaded, false otherwise - false if in Prefab Edit mode too
	static bool IsWorldLoaded()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return false;

		if (worldEditor.IsPrefabEditMode())
			return false;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (!worldEditorAPI)
			return false;

		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);
		return worldPath && worldEditorAPI.GetWorld() != null; // !.IsEmpty()
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if terrain has been generated, false if not or no terrain entity is present
	static bool HasTerrainMesh()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return false;

		vector min, max;
		worldEditor.GetTerrainBounds(min, max);
		return max != vector.Zero || min != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the currently opened terrain's GenericTerrainEntity source or null on error / no world
	static IEntitySource GetTerrainEntitySource()
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		if (worldEditor.IsPrefabEditMode())
			return null;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (worldEditorAPI.GetEditorEntityCount() < 2) // world and Entity
			return null;

		IEntitySource entitySource;
		for (int i, entitiesCount = worldEditorAPI.GetEditorEntityCount(); i < entitiesCount; ++i)
		{
			entitySource = worldEditorAPI.GetEditorEntity(i);
			if (entitySource && entitySource.GetClassName().ToType() && entitySource.GetClassName().ToType().IsInherited(GenericTerrainEntity))
				return entitySource;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return terrain's absolute dimensions (using terrain bounds)
	static vector GetTerrainDimensions()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		vector min, max;
		worldEditor.GetTerrainBounds(min, max);
		return max - min;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] minY
	//! \param[out] maxY
	static void GetTerrainMinMaxY(out float minY, out float maxY)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		vector min, max;
		worldEditor.GetTerrainBounds(min, max);

		minY = min[1];
		maxY = max[1];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] min
	//! \param[out] max
	//! \return true on success, false otherwise (e.g no terrain)
	static bool GetTerrainMinMaxElevationPositions(out vector min, out vector max)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!HasTerrainMesh())
			return false;

		float minY, maxY;
		GetTerrainMinMaxY(minY, maxY);

		vector terrainPosition = GetTerrainPosition();
		float minX = terrainPosition[0];
		float minZ = terrainPosition[2];

		vector terrainDimensions = GetTerrainDimensions();
		float maxX = minX + terrainDimensions[0];
		float maxZ = minZ + terrainDimensions[2];

		float step = worldEditorAPI.GetTerrainUnitScale();

		bool hasMinY, hasMaxY;

		float y;
		for (float z = minZ, limitZ = maxZ; z <= limitZ; z += step)
		{
			for (float x = minX, limitX = maxX; x <= limitX; x += step)
			{
				if (!worldEditorAPI.TryGetTerrainSurfaceY(x, z, y))
					continue;

				if (!hasMaxY && y == maxY)
				{
					max = { x, y, z };
					hasMaxY = true;
				}
				else
				if (!hasMinY && y == minY)
				{
					min = { x, y, z };
					hasMinY = true;
				}

				if (hasMinY && hasMaxY)
					return true;
			}
		}

		return hasMinY && hasMaxY;
	}

	//------------------------------------------------------------------------------------------------
	//! Get current terrain's land area (Y >= ocean level) in square metres, or -1 if no terrain is loaded or on wrong terrain index
	//! \param[in] terrainIndex 0 or above
	//! \return the land area in m² (multiply by 0.000001 to convert to km²)
	static float GetTerrainLandArea(int terrainIndex = 0)
	{
		if (terrainIndex < 0)
			return -1;

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return -1;

		const float cellSideSize = worldEditorAPI.GetTerrainUnitScale(terrainIndex);
		if (cellSideSize <= 0)
			return -1; // wrong terrain index or no terrain loaded

		const float cellSideSizeSq = cellSideSize * cellSideSize;

		if (!world.IsOcean()) // no ocean = everything is land
		{
			return cellSideSizeSq
				* (worldEditorAPI.GetTerrainResolutionX(terrainIndex) - 1)
				* (worldEditorAPI.GetTerrainResolutionY(terrainIndex) - 1);
		}

		const float oceanLevel = worldEditorAPI.GetWorld().GetOceanBaseHeight();
		if (oceanLevel == -float.MAX) // just some safety, most likely not needed
		{
			return cellSideSizeSq
				* (worldEditorAPI.GetTerrainResolutionX(terrainIndex) - 1)
				* (worldEditorAPI.GetTerrainResolutionY(terrainIndex) - 1);
		}

		const int tileXCount = worldEditorAPI.GetTerrainTilesX(terrainIndex);
		const int tileYCount = worldEditorAPI.GetTerrainTilesY(terrainIndex);

		const int tileSideSizeX = (worldEditorAPI.GetTerrainResolutionX(terrainIndex) - 1) / tileXCount;
		const int tileSideSizeY = (worldEditorAPI.GetTerrainResolutionY(terrainIndex) - 1) / tileYCount;

		const int tileArea = tileSideSizeX * tileSideSizeY;
		const int tileValuesCount = (tileSideSizeX + 1) * (tileSideSizeY + 1); // +1 - e.g 32×32 cells = 33×33 vertices
		array<float> tileYs = {};
		tileYs.Resize(tileValuesCount);

		float result;

		int bottomLeftIndex, bottomRightIndex;
		float topLeftY, bottomLeftY, topRightY, bottomRightY;
		bool aboveZeroTL, aboveZeroTR, aboveZeroBL, aboveZeroBR;
		bool tileHasAboveZeroArea, tileHasBelowZeroArea;
		for (int tileY; tileY < tileYCount; ++tileY)
		{
			for (int tileX; tileX < tileXCount; ++tileX)
			{
				if (!worldEditorAPI.GetTerrainSurfaceTile(terrainIndex, tileX, tileY, tileYs))
				{
					PrintFormat("Invalid tile %1:%2", tileX, tileY, level: LogLevel.ERROR);
					return -1;
				}

				tileHasAboveZeroArea = false;
				tileHasBelowZeroArea = false;
				foreach (float yValue : tileYs)
				{
					if (yValue >= oceanLevel)
					{
						tileHasAboveZeroArea = true;
						if (tileHasBelowZeroArea)
							break;
					}
					else
					{
						tileHasBelowZeroArea = true;
						if (tileHasAboveZeroArea)
							break;
					}
				}

				if (!tileHasAboveZeroArea) // full underwater
					continue;

				if (!tileHasBelowZeroArea) // full above water
				{
					result += tileArea * cellSideSizeSq;
					continue;
				}

				float tileSurface; // intermediate variable needed for big terrains (e.g Balta) due to float precision
				for (int tileValueY = 1; tileValueY <= tileSideSizeY; ++tileValueY)
				{
					for (int tileValueX = 1; tileValueX <= tileSideSizeX; ++tileValueX)
					{
						if (tileValueX == 1)
						{
							bottomLeftIndex = tileValueX - 1 + tileSideSizeY * (tileValueY - 1);
							topLeftY = tileYs[bottomLeftIndex + tileSideSizeY];
							bottomLeftY = tileYs[bottomLeftIndex];
						}
						else
						{
							bottomLeftIndex = bottomRightIndex;
							topLeftY = topRightY;
							bottomLeftY = bottomRightY;
						}

						bottomRightIndex = bottomLeftIndex + 1;
						topRightY = tileYs[bottomLeftIndex + tileSideSizeY + 1];
						bottomRightY = tileYs[bottomRightIndex];

						aboveZeroTL = topLeftY >= 0;
						aboveZeroTR = topRightY >= 0;
						aboveZeroBL = bottomLeftY >= 0;
						aboveZeroBR = bottomRightY >= 0;
						if (aboveZeroTL == aboveZeroTR && aboveZeroTL == aboveZeroBL && aboveZeroTL == aboveZeroBR) // everything equal
						{
							if (aboveZeroTL) // all points are >= 0
								tileSurface += 1; // one full cell

							continue;
						}

						// top-right triangle
						if (aboveZeroTL == aboveZeroTR && aboveZeroTL == aboveZeroBR)
						{
							if (aboveZeroTL)
								tileSurface += 0.5;
						}
						else
						{
							tileSurface += GetTriangle2DAreaAboveOceanLevel(topLeftY, topRightY, bottomRightY, oceanLevel);
						}

						// bottom-left triangle
						if (aboveZeroTL == aboveZeroBL && aboveZeroTL == aboveZeroBR)
						{
							if (aboveZeroTL)
								tileSurface += 0.5;
						}
						else
						{
							tileSurface += GetTriangle2DAreaAboveOceanLevel(topLeftY, bottomLeftY, bottomRightY, oceanLevel);
						}
					}
				}

				result += tileSurface * cellSideSizeSq;
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the above-ocean 2D area of a triangle
	//! \param[in] a y value at pos 0,0
	//! \param[in] b y value at pos 0,1
	//! \param[in] c y value at pos 1,1
	//! \param[in] oceanLevel
	//! \return triangle surface above zero in range 0..0.5 (given it is a half-cell triangle)
	protected static float GetTriangle2DAreaAboveOceanLevel(float a, float b, float c, float oceanLevel = 0)
	{
		array<float> heights = { a - oceanLevel, b - oceanLevel, c - oceanLevel };
		heights.Sort();

		if (heights[2] < 0)
			return 0;

		const float area = 0.5;

		// all above zero
		if (heights[0] >= 0)
			return area;

		// only heights[2] is above zero → small positive triangle near heights[2]
		if (heights[1] < 0)
		{
			float r1 = heights[2] / (heights[2] - heights[0]);
			float r2 = heights[2] / (heights[2] - heights[1]);
			return area * r1 * r2 * heights[2] / 3;
		}

		// heights[0] < 0, heights[1] >= 0, heights[2] > 0 → cut away negative triangle near heights[0]
		float s1 = -heights[0] / (heights[1] - heights[0]);
		float s2 = -heights[0] / (heights[2] - heights[0]);
		float negSubArea = area * s1 * s2;
		// negative sub-triangle has heights (heights[0], 0, 0), volume = negSubArea * heights[0] / 3
		// total = full volume - negative sub volume
		return area * (heights[0] + heights[1] + heights[2]) / 3 - negSubArea * heights[0] / 3;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static vector GetTerrainPosition()
	{
		IEntitySource terrainEntitySource = GetTerrainEntitySource();
		if (!terrainEntitySource)
			return vector.Zero;

		vector result;
		terrainEntitySource.Get("coords", result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static IEntitySource GetTimeAndWeatherManagerEntitySource()
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		if (worldEditor.IsPrefabEditMode())
			return null;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (worldEditorAPI.GetEditorEntityCount() < 2) // world and Entity
			return null;

		IEntitySource entitySource;
		for (int i, entitiesCount = worldEditorAPI.GetEditorEntityCount(); i < entitiesCount; ++i)
		{
			entitySource = worldEditorAPI.GetEditorEntity(i);
			if (entitySource && entitySource.GetClassName().ToType() && entitySource.GetClassName().ToType().IsInherited(TimeAndWeatherManagerEntity))
				return entitySource;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] direction
	//! \return
	static vector GetWorldEditorCameraPosition(out vector direction = vector.Zero)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return vector.Zero;

		vector result, traceEnd;
		worldEditorAPI.TraceWorldPos(0, 0, 0, result, traceEnd, direction);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get a registered file's ResourceName (requires the .meta file)
	//! \return the registered file's ResourceName
	static ResourceName GetResourceNameFromFile(string absoluteFilePath)
	{
		MetaFile metaFile = GetResourceManager().GetMetaFile(absoluteFilePath);
		if (!metaFile)
			return string.Empty;

		return metaFile.GetResourceID();
	}

	//------------------------------------------------------------------------------------------------
	//! Get selected directories - does not get subdirectories, only selected ones (in their order of selection)
	//! \return resource names of directory that are selected in World Editor's Resource Browser
	static array<ResourceName> GetSelectedDirectories()
	{
		array<ResourceName> result = GetSelectedResources(true);
		if (!result)
			return null;

		for (int i = result.Count() - 1; i >= 0; i--)
		{
			ResourceName selectedResource = result[i];
			if (FilePath.StripExtension(selectedResource) != selectedResource)
				result.RemoveOrdered(i);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Wrapper for the method below
	//! \param[in] wantedExtension (case-insensitive)
	//! \param[in] keywords words that should be present in the file name (case-insensitive)
	//! \return an array of found ResourceNames - cannot be null
	static array<ResourceName> GetSelectedOrOpenedResources(string wantedExtension, array<string> keywords = null)
	{
		return GetSelectedOrOpenedResources({ wantedExtension }, keywords);
	}

	//------------------------------------------------------------------------------------------------
	//! \return an array of selected entities's IEntitySources, null on error (e.g WorldEditorAPI not available)
	static array<IEntitySource> GetSelectedWorldEntitySources()
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return null;

		array<IEntitySource> result = {};
		IEntitySource entitySource;
		for (int i, count = worldEditorAPI.GetSelectedEntitiesCount(); i < count; ++i)
		{
			entitySource = worldEditorAPI.GetSelectedEntity(i);
			if (entitySource)
				result.Insert(entitySource);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get selected or opened resources
	//! \param[in] acceptedExtensions accepted extensions (case-insensitive)
	//! \param[in] keywords words that should be present in the file name (case-insensitive)
	//! \return an array of found ResourceNames - cannot be null
	static array<ResourceName> GetSelectedOrOpenedResources(array<string> acceptedExtensions = null, array<string> keywords = null)
	{
		array<ResourceName> tempResult = GetSelectedResources(true);
		if (tempResult.IsEmpty())
		{
			ResourceName openedResource = GetPrefabEditModeResourceName();
			if (!openedResource.IsEmpty())
				tempResult.Insert(openedResource);
		}

		if (tempResult.IsEmpty())
			return tempResult;

		if (acceptedExtensions && acceptedExtensions.IsEmpty())
			acceptedExtensions = null;

		if (acceptedExtensions)
		{
			foreach (int i, string wantedExtension : acceptedExtensions)
			{
				wantedExtension.ToLower();
				acceptedExtensions[i] = wantedExtension;
			}
		}

		array<string> keywordsLC;
		if (keywords && !keywords.IsEmpty())
		{
			keywordsLC = {};
			foreach (string keyword : keywords)
			{
				if (keyword.IsEmpty())
					continue;

				keyword.ToLower();
				keywordsLC.Insert(keyword);
			}
		}

		array<ResourceName> result = {};

		foreach (ResourceName resourceName : tempResult)
		{
			string resourceNameLC = resourceName;
			resourceNameLC.ToLower();

			string extensionLC;
			resourceNameLC = FilePath.StripExtension(resourceName, extensionLC);
			extensionLC.ToLower();

			if (acceptedExtensions && !acceptedExtensions.Contains(extensionLC))
				continue;

			resourceNameLC = FilePath.StripPath(resourceNameLC);

			if (!keywordsLC || SCR_StringHelper.ContainsEvery(resourceNameLC, keywordsLC))
				result.Insert(resourceName);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all ResourceName that are selected in the World Editor's Resource Browser
	//! \param[in] recursive true to get a selected directory's files, false to stop at the directory
	//! \return array of ResourceName of selected resources or null on error (e.g World Editor is not available)
	static array<ResourceName> GetSelectedResources(bool recursive = true)
	{
		WorldEditor worldEditor = GetWorldEditor();
		if (!worldEditor)
			return null;

		array<ResourceName> result = {};
		worldEditor.GetResourceBrowserSelection(result.Insert, recursive);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_WorkbenchHelper.SearchWorkbenchResources instead")] // obsolete since 2025-03-03
	static array<ResourceName> SearchWorkbenchResources(array<string> fileExtensions = null, array<string> searchStrArray = null, string rootPath = "", bool recursive = true)
	{
		return SCR_WorkbenchHelper.SearchWorkbenchResources(fileExtensions, searchStrArray, rootPath, recursive);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete the entitySource's entity IF it exists
	//! \param[in] entitySource can be null
	static void DeleteEntityFromSource(IEntitySource entitySource)
	{
		if (!entitySource)
			return;

		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return;
		}

		bool manageEditAction = BeginEntityAction();

		worldEditorAPI.DeleteEntity(entitySource);

		EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//! Begin an Entity Action in World Editor API if required
	//! \return true if a BeginEntityAction has been called and EndEntityAction has to be called by EndEntityAction(), false otherwise
	static bool BeginEntityAction()
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return false;
		}

		if (worldEditorAPI.IsDoingEditAction())
			return false;

		return worldEditorAPI.BeginEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	//! End an Entity Action in World Editor API if required
	//! \param[in] manageEditAction if World Editor Entity Action should be terminated, result of an earlier BeginEntityAction call
	static void EndEntityAction(bool manageEditAction)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return;
		}

		if (manageEditAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	//! Queries entities within an AABB in the world, returns results in an array
	//! \param[in] world represents the game's environment in which entities exist, used for querying entities within an Axis-Aligned Bounding Box
	//! \param[in] mins min bounds for an axis-aligned bounding box (AABB) query in 3D space
	//! \param[in] maxs max bounds for an axis-aligned bounding box (AABB) query in 3D space
	//! \param[in] queryFlags is an enumeration representing filter options for entity query in AABB - see BaseWorld.QueryEntitiesByAABB
	//! \return an array of entities within the specified AABB (Axis-Aligned Bounding Box) in the world or null on QueryEntitiesByAABB error
	static array<IEntity> QueryEntitiesByAABB(notnull World world, vector mins, vector maxs, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL)
	{
		s_aTempEntities = {};
		if (!world.QueryEntitiesByAABB(mins, maxs, QueryEntitiesCallbackMethod, null, queryFlags))
			return null;

		array<IEntity> result = {};
		result.Copy(s_aTempEntities);
		s_aTempEntities = null;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Queries entities within a sphere in the world, returns them in an array.
	//! \param[in] world World represents the game's environment in which entities exist, used for querying entities within a specified sphere radius in the method
	//! \param[in] worldPos represents the center point for the sphere query in 3D space
	//! \param[in] radius represents the distance from the center point (worldPos) within which entities are searched in the method
	//! \param[in] queryFlags specifies query flags for entity selection criteria - see BaseWorld.QueryEntitiesBySphere
	//! \return an array of entities within specified radius from world position
	static array<IEntity> QueryEntitiesBySphere(notnull World world, vector worldPos, float radius, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL)
	{
		array<IEntity> result = {};
		s_aTempEntities = {};
		world.QueryEntitiesBySphere(worldPos, radius, QueryEntitiesCallbackMethod, null, queryFlags);
		result.Copy(s_aTempEntities);
		s_aTempEntities = null;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Return all entities found by moved sphere trace
	//! \param[in] traceSphere
	//! \param[in] world
	//! \return found entities or null on error
	static array<IEntity> TraceMoveEntitiesBySphere(notnull World world, notnull TraceSphere traceSphere)
	{
		array<IEntity> result = {};

		s_aTempEntities = {};
		world.TraceMove(traceSphere, TraceCallbackMethod);

		result.Copy(s_aTempEntities);
		s_aTempEntities = null;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Return all entities found by sphere trace
	//! \param[in] traceSphere
	//! \param[in] world
	//! \return found entities or null on error
	static array<IEntity> TracePositionEntitiesBySphere(notnull World world, notnull TraceSphere traceSphere)
	{
		array<IEntity> result = {};

		s_aTempEntities = {};
		world.TracePosition(traceSphere, TraceCallbackMethod);

		result.Copy(s_aTempEntities);
		s_aTempEntities = null;

		return result;
	}

	//
	// Callbacks
	//

	//------------------------------------------------------------------------------------------------
	//! QueryEntitiesCallback method used for Entity querying
	protected static bool QueryEntitiesCallbackMethod(IEntity e)
	{
		s_aTempEntities.Insert(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! TraceEntitiesCallback method used for World tracing
	protected static bool TraceCallbackMethod(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		s_aTempEntities.Insert(e);
		return true;
	}
}
#endif // WORKBENCH
