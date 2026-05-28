#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: PLUGIN_NAME,
	description: "Import SHP file as forest, power line, lake, road, prefabs, comments etc.\n\n"
		+ "If Prefabs is filled, creates a prefab for each datapoint (point)\n"
		+ "or creates it as an imported shape's child (polyline/spline).\n"
		+ "- a comment prefab must have a SCR_EditableCommentComponent component\n"
		+ "- a prefab entry can be left empty in some cases (e.g comments)\n"
		+ "- a -1 id in this list prevents the SHP file to reference the entry\n"
		+ "- a negative or undefined id in the SHP file means a random prefab will be used",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF56F)]
class SCR_ImportShapefilePlugin : WorkbenchPlugin
{
	/*
		Import
	*/

	[Attribute(defvalue: "", uiwidget: UIWidgets.FileNamePicker, desc: "SHP File", params: "shp", category: "Import")]
	protected string m_sSHPFilePath;

	[Attribute(defvalue: "0 0 0", desc: "Import offset", category: "Import")]
	protected vector m_vOffset;

	/*
		Prefabs
	*/

	[Attribute(desc: "List of prefabs to be used - use either 'set class' or drag and drop an adequate SCR_SHPPrefabDataList .conf file", category: "Prefab")]
	protected ref SCR_SHPPrefabDataList m_PrefabDataList;

	[Attribute(defvalue: "1", desc: "Random direction for prefabs imported by points (point and multipoint)", category: "Prefab")]
	protected bool m_bRandomYaw;

	[Attribute(defvalue: "0", desc: "Generate ForestGenerator's Shape Point Data (polyline and polygon)", category: "Prefab")]
	protected bool m_bGenerateForestGeneratorPointData;

	[Attribute(defvalue: "0", desc: "Generate Powerline's Shape Point Data as \"per point\" (polyline only)", category: "Prefab")]
	protected bool m_bGeneratePowerlinePerPointPointData;

	[Attribute(defvalue: "", desc: "Name of the column with IDs - to match Prefabs' ID", category: "Prefab")]
	protected string m_sIDColumnName;

	[Attribute(defvalue: "", desc: "Name of the column holding the comments' text - can be used with ID Column Name\nLeave empty to force Prefab import", category: "Prefab")]
	protected string m_sCommentsColumnName;

	/*
		Shape
	*/

	[Attribute(defvalue: "0.93 0.13 0.33 1", UIWidgets.ColorPicker, desc: "Colour the imported polyline/spline(s) will be", category: "Shape")]
	protected vector m_vShapeColor;

	[Attribute(defvalue: "0", desc: "Create a spline from the input vectors if ticked (otherwise a polyline is created)", category: "Shape")]
	protected bool m_bCreateAsSpline;

	protected static const float DUPLICATE_RADIUS_SQ = 0.1 * 0.1; //!< anything below this distance will be considered a duplicate point
	protected static const int RANDOM_PREFAB_ID = -1;

	// used classes
	protected static const string COMMENT_ENTITY_CLASS = "CommentEntity"; // ((typename)CommentEntity).ToString()
	protected static const string EDITABLE_COMMENT_COMPONENT_CLASS = "SCR_EditableCommentComponent"; // ((typename)SCR_EditableCommentComponent).ToString();
	protected static const string EDITABLE_COMMENT_UI_INFO_CLASS = "SCR_EditableCommentUIInfo"; // ((typename)SCR_EditableCommentUIInfo).ToString();
	protected static const string MAP_DESCRIPTOR_COMPONENT_CLASS = "SCR_MapDescriptorComponent"; // ((typename)SCR_MapDescriptorComponent).ToString();
	protected static const string POLYLINE_SHAPE_ENTITY_CLASS = "PolylineShapeEntity"; // ((typename)PolylineShapeEntity).ToString();
	protected static const string SPLINE_SHAPE_ENTITY_CLASS = "SplineShapeEntity"; // ((typename)SplineShapeEntity).ToString();

	// default comment's settings
	protected static const vector COMMENT_TEXT_COLOUR = { 1, 0.960998, 0.297002 }; // debug yellow
	protected static const float COMMENT_TEXT_SIZE = 2;
	protected static const bool COMMENT_TEXT_BACKGROUND = true;
	protected static const bool COMMENT_FACE_CAMERA = true;

	protected static const string PLUGIN_NAME = "Import Shapefile";

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		if (Workbench.ScriptDialog(PLUGIN_NAME, "", this) == 0)
			return;

		Execute();
	}

	//------------------------------------------------------------------------------------------------
	//! Load the SHP file and treat all the shapes present in it
	//! Multiple shape types can be present in the same SHP file (although it is not recommended data-wise)
	protected void Execute()
	{
		if (m_sSHPFilePath.IsEmpty())
		{
			Print("Please provide an SHP file", LogLevel.WARNING);
			return;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		// one -must- keep reference to the collection otherwise chaos ensues: shapes become null, attribute names become empty, etc
		GeoShapeCollection shapeCollection = GeoShapeLoader.LoadShapeFile(m_sSHPFilePath);
		if (!shapeCollection)
		{
			Print("Shapefile cannot be loaded", LogLevel.ERROR);
			return;
		}

		if (shapeCollection.Count() < 1)
		{
			Print("Shapefile is empty - leaving", LogLevel.NORMAL);
			return;
		}

		Debug.BeginTimeMeasure();

		if (!HasPrefabListPrefabs())
			Print("[INFO] Prefab list is null/empty", LogLevel.NORMAL);

		worldEditorAPI.BeginEntityAction();

		array<ref SCR_GeneratorShapeImportData> shapeDataArray = {};

		GeoShapeType shapeType;
		// treat all shapes
		Debug.BeginTimeMeasure();
		ProcessShapes(shapeCollection, shapeDataArray, shapeType);
		Debug.EndTimeMeasure("Shape process");

		if (!shapeDataArray.IsEmpty())
		{
			// woodwork
			if (m_bGenerateForestGeneratorPointData)
			{
				Debug.BeginTimeMeasure();
				GenerateForestGeneratorPointData(shapeDataArray);
				Debug.EndTimeMeasure("Generate forest generator's point data");
			}

			// shocking: electrical work
			if (m_bGeneratePowerlinePerPointPointData && shapeType == GeoShapeType.POLYGON)
			{
				Debug.BeginTimeMeasure();
				GeneratePowerLinePointData(shapeDataArray);
				Debug.EndTimeMeasure("Generate powerline generator's point data");
			}

			Debug.BeginTimeMeasure();
			AttachChildren(shapeDataArray);
			Debug.EndTimeMeasure("Attach children");
		}

		worldEditorAPI.EndEntityAction();
		Debug.EndTimeMeasure("Shape import");
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] shapeCollection
	//! \param[out] shapeDataArray
	//! \param[out] shapeType
	protected void ProcessShapes(
		notnull GeoShapeCollection shapeCollection,
		out notnull array<ref SCR_GeneratorShapeImportData> shapeDataArray,
		out GeoShapeType shapeType)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WBProgressDialog progress = new WBProgressDialog("Importing geometries...", worldEditor);
		array<ref SCR_GeneratorShapeImportData> shapeDataTempArray;
		GeoShape shape;
		GeoPoint geoPoint;
		GeoMultiPoint geoMultiPoint;
		GeoPolyline geoPolyline;
		GeoPolygon geoPolygon;

		float prevProgress, currProgress;
		for (int i, count = shapeCollection.Count(); i < count; i++)
		{
			shape = shapeCollection.Get(i);
			if (!shape)
			{
				PrintFormat("Shape #%1/%2 is null?! Skipping", i + 1, count, level: LogLevel.WARNING);
				continue;
			}

			shapeDataTempArray = null;

			shapeType = shape.GetType();
			switch (shapeType)
			{
				case GeoShapeType.POINT:
					geoPoint = GeoPoint.Cast(shape);
					if (geoPoint)
						Load_Point(geoPoint);

					break;

				case GeoShapeType.MULTI_POINT:
					shapeType = GeoShapeType.MULTI_POINT;
					geoMultiPoint = GeoMultiPoint.Cast(shape);
					if (geoMultiPoint)
						Load_Multipoint(geoMultiPoint);

					break;

				case GeoShapeType.POLYLINE:
					geoPolyline = GeoPolyline.Cast(shape);
					if (geoPolyline)
						shapeDataTempArray = Load_Polylines(GeoPolyline.Cast(shape));

					break;

				case GeoShapeType.POLYGON:
					geoPolygon = GeoPolygon.Cast(shape);
					if (geoPolygon)
						shapeDataTempArray = Load_Polygons(GeoPolygon.Cast(shape));

					break;

				default:
					Print("Unsupported type " + typename.EnumToString(GeoShapeType, shape.GetType()), LogLevel.WARNING);
					break;
			}

			if (shapeDataTempArray)
			{
				foreach (SCR_GeneratorShapeImportData data : shapeDataTempArray)
				{
					shapeDataArray.Insert(data); // can't InsertAll with array of ref
				}
			}

			currProgress = i / count;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Load from GeoPoint (using CreateFromPoints)
	protected void Load_Point(notnull GeoPoint point)
	{
		CreateFromPoints(point, { point.GetCoords() });
	}

	//------------------------------------------------------------------------------------------------
	//! Load from GeoMultiPoint (using CreateFromPoints)
	protected void Load_Multipoint(notnull GeoMultiPoint multipoint)
	{
		array<vector> points = {};
		GeoVertexCollection vertices = multipoint.GetPoints();
		for (int i = 0, count = vertices.Count(); i < count; i++)
		{
			points.Insert(vertices[i]);
		}

		CreateFromPoints(multipoint, points);
	}

	//------------------------------------------------------------------------------------------------
	//! Load polygons (closed shapes)
	protected array<ref SCR_GeneratorShapeImportData> Load_Polygons(notnull GeoPolygon polygon)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		bool hasIdAttribute = polygon.GetAttributes().HasAttrib(m_sIDColumnName);

		array<ref SCR_GeneratorShapeImportData> result = {};
		IEntitySource entitySource;
		SCR_GeneratorShapeImportData data;

		int currentLayerId = worldEditorAPI.GetCurrentEntityLayerId();

		GeoVertexCollection geoVertexCollectionJ;
		// polygon's parts (in case polygon has holes)
		for (int j = 0, count = polygon.PartsCount(); j < count; j++)
		{
			geoVertexCollectionJ = polygon.GetPart(j);
			int polygonPointsCount = geoVertexCollectionJ.Count();

			float traceX = geoVertexCollectionJ[0][0] + m_vOffset[0];
			float traceZ = geoVertexCollectionJ[0][2] + m_vOffset[2];
			float pntY = worldEditorAPI.GetTerrainSurfaceY(traceX, traceZ) + m_vOffset[1];

			vector bb3Dmin = { traceX, pntY, traceZ };
			vector bb3Dmax = { traceX, pntY, traceZ };

			//Part's points - calculate polygon's 3D Bbox
			for (int k = 0; k < polygonPointsCount; k++)
			{
				traceX = geoVertexCollectionJ[k][0] + m_vOffset[0];
				traceZ = geoVertexCollectionJ[k][2] + m_vOffset[2];
				pntY = worldEditorAPI.GetTerrainSurfaceY(traceX, traceZ) + m_vOffset[1];

				vector pnt = { traceX, pntY, traceZ };

				for (int xyz = 0; xyz < 3; xyz++)
				{
					float val = pnt[xyz];

					if (val < bb3Dmin[xyz])
						bb3Dmin[xyz] = val;

					if (val > bb3Dmax[xyz])
						bb3Dmax[xyz] = val;
				}
			}

			// calculate polygon's origin(as center of 3D Bbox)
			vector polygonOrigin = ((bb3Dmax - bb3Dmin) * 0.5) + bb3Dmin;

			// create Polyline/Spline ShapeEntity
			if (m_bCreateAsSpline)
				entitySource = worldEditorAPI.CreateEntity(SPLINE_SHAPE_ENTITY_CLASS, "", currentLayerId, null, polygonOrigin, vector.Zero);
			else
				entitySource = worldEditorAPI.CreateEntity(POLYLINE_SHAPE_ENTITY_CLASS, "", currentLayerId, null, polygonOrigin, vector.Zero);

			worldEditorAPI.SetVariableValue(entitySource, null, "IsClosed", "1");

			data = new SCR_GeneratorShapeImportData();

			// part's points - calculate point's local coords(center of 3D Bbox is the origin)
			for (int k = 0; k < polygonPointsCount - 1; k++) // polygonPointsCount - 1 because the last points duplicates the first one
			{
				vector worldCoords;
				worldCoords[0] = geoVertexCollectionJ[k][0] + m_vOffset[0];
				worldCoords[2] = geoVertexCollectionJ[k][2] + m_vOffset[2];
				worldCoords[1] = worldEditorAPI.GetTerrainSurfaceY(worldCoords[0], worldCoords[2]) + m_vOffset[1];
				data.points.Insert(worldCoords);

				worldEditorAPI.CreateObjectArrayVariableMember(entitySource, null, "Points", "ShapePoint", k);
				worldEditorAPI.SetVariableValue(entitySource, { new ContainerIdPathEntry("Points", k) }, "Position", (worldCoords - polygonOrigin).ToString(false));
			}

			// setup polygons colors
			float finalR = m_vShapeColor[0];
			float finalG = m_vShapeColor[1];
			float finalB = m_vShapeColor[2];

			if (j != 0)
			{
				// invert default color to better distinguish inner polygons (holes) from outer (main) polygon
				finalR = Math.AbsFloat(finalR - 1);
				finalG = Math.AbsFloat(finalG - 1);
				finalB = Math.AbsFloat(finalB - 1);
			}

			worldEditorAPI.SetVariableValue(entitySource, null, "LineColor", string.Format("%1 %2 %3 1", finalR, finalG, finalB));

			data.source = entitySource;
			data.entity = worldEditorAPI.SourceToEntity(entitySource);
			data.GenerateAAB();

			if (hasIdAttribute)
				data.id = polygon.GetAttributes().GetIntByName(m_sIDColumnName);
			else
				data.id = RANDOM_PREFAB_ID;

			result.Insert(data);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Load polylines (open shapes) - as polyline or spline, depending on m_bCreateAsSpline
	protected array<ref SCR_GeneratorShapeImportData> Load_Polylines(notnull GeoPolyline polyline)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		GeoVertexCollection vertices = polyline.GetVertices();
		int polylinePointsCount = vertices.Count();

		float traceX = vertices[0][0] + m_vOffset[0];
		float traceZ = vertices[0][2] + m_vOffset[2];
		float pntY = worldEditorAPI.GetTerrainSurfaceY(traceX, traceZ) + m_vOffset[1];

		vector bb3Dmin = { traceX, pntY, traceZ };
		vector bb3Dmax = { traceX, pntY, traceZ };

		// polyline's points - calculate polyline's 3D Bbox
		for (int k = 0; k < polylinePointsCount; k++)
		{
			traceX = vertices[k][0] + m_vOffset[0];
			traceZ = vertices[k][2] + m_vOffset[2];
			pntY = worldEditorAPI.GetTerrainSurfaceY(traceX, traceZ) + m_vOffset[1];

			vector pnt = { traceX, pntY, traceZ };

			for (int xyz = 0; xyz < 3; xyz++)
			{
				float val = pnt[xyz];

				if (val < bb3Dmin[xyz])
					bb3Dmin[xyz] = val;

				if (val > bb3Dmax[xyz])
					bb3Dmax[xyz] = val;
			}
		}

		// calculate polygon's origin(as center of 3D Bbox)
		vector polylineOrigin = ((bb3Dmax - bb3Dmin) * 0.5) + bb3Dmin;

		// create Polyline/Spline ShapeEntity
		IEntitySource entitySource;
		if (m_bCreateAsSpline)
			entitySource = worldEditorAPI.CreateEntity(SPLINE_SHAPE_ENTITY_CLASS, "", worldEditorAPI.GetCurrentEntityLayerId(), null, polylineOrigin, vector.Zero);
		else
			entitySource = worldEditorAPI.CreateEntity(POLYLINE_SHAPE_ENTITY_CLASS, "", worldEditorAPI.GetCurrentEntityLayerId(), null, polylineOrigin, vector.Zero);

		SCR_GeneratorShapeImportData data = new SCR_GeneratorShapeImportData();

		// polyline's points - calculate point's local coords(center of 3D Bbox is the origin)
		for (int k = 0; k < polylinePointsCount; k++)
		{
			vector worldCoords;
			worldCoords[0] = vertices[k][0] + m_vOffset[0];
			worldCoords[2] = vertices[k][2] + m_vOffset[2];
			worldCoords[1] = worldEditorAPI.GetTerrainSurfaceY(worldCoords[0], worldCoords[2]) + m_vOffset[1];
			data.points.Insert(worldCoords);
			data.GenerateAAB();

			worldEditorAPI.CreateObjectArrayVariableMember(entitySource, null, "Points", "ShapePoint", k);
			worldEditorAPI.SetVariableValue(entitySource, { new ContainerIdPathEntry("Points", k) }, "Position", (worldCoords - polylineOrigin).ToString(false));
		}

		worldEditorAPI.SetVariableValue(entitySource, null, "LineColor", m_vShapeColor.ToString(false) + " 1");

		data.source = entitySource;
		data.entity = worldEditorAPI.SourceToEntity(entitySource);
		data.GenerateAAB();

		if (polyline.GetAttributes().HasAttrib(m_sIDColumnName))
			data.id = polyline.GetAttributes().GetIntByName(m_sIDColumnName);
		else
			data.id = RANDOM_PREFAB_ID;

		return { data };
	}

	//------------------------------------------------------------------------------------------------
	//! Common method treating points from POINT or MULTI_POINT GeoShapes
	protected void CreateFromPoints(notnull GeoShape shape, notnull array<vector> points)
	{
		// comment import
		if (!m_sCommentsColumnName.IsEmpty() && shape.GetAttributes().HasAttrib(m_sCommentsColumnName))
		{
			Print("Importing comment", LogLevel.NORMAL);
			string comment = shape.GetAttributes().GetStringByName(m_sCommentsColumnName).Trim();
			if (comment.IsEmpty())
			{
				Print("Empty comment, skipping...", LogLevel.NORMAL);
				return;
			}

			foreach (vector pos : points)
			{
				CreateComment(comment, pos + m_vOffset, GetPrefab());
			}

			return;
		}

		// let's save some CPU cycles
		if (!HasPrefabListPrefabs())
		{
			Print("Cannot import (multi)point as no Prefabs are available and the shape does not have comment column ID \"" + m_sCommentsColumnName + "\" defined", LogLevel.WARNING);
			return;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		// prefab import
		Print("Importing prefab(s)", LogLevel.NORMAL);
		for (int i = 0, count = points.Count(); i < count; i++)
		{
			ResourceName chosenPrefab = GetPrefab();
			if (chosenPrefab.IsEmpty())
			{
				Print("Prefab is empty, skipping", LogLevel.VERBOSE);
				continue;
			}

			vector pos = points[0] + m_vOffset;
			vector angles;
			if (m_bRandomYaw)
				angles[1] = Math.RandomFloat(0, 360);

			worldEditorAPI.CreateEntityExt(chosenPrefab, "", worldEditorAPI.GetCurrentEntityLayerId(), null, pos, angles, TraceFlags.WORLD);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create a comment from provided arguments. if the prefab is empty, creates a CommentEntity comment
	protected void CreateComment(string comment, vector pos, ResourceName commentPrefab = string.Empty)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		
		if (commentPrefab.IsEmpty()) // then generic comment
		{
			IEntitySource entitySource = worldEditorAPI.CreateEntity(COMMENT_ENTITY_CLASS, "", worldEditorAPI.GetCurrentEntityLayerId(), null, pos, vector.Zero);

			worldEditorAPI.SetVariableValue(entitySource, null, "m_Comment", comment);
			worldEditorAPI.SetVariableValue(entitySource, null, "m_Color", COMMENT_TEXT_COLOUR.ToString(false));
			worldEditorAPI.SetVariableValue(entitySource, null, "m_FaceCamera", COMMENT_FACE_CAMERA.ToString(true));
			worldEditorAPI.SetVariableValue(entitySource, null, "m_TextBackground", COMMENT_TEXT_BACKGROUND.ToString(true));
			worldEditorAPI.SetVariableValue(entitySource, null, "m_Size", COMMENT_TEXT_SIZE.ToString());
			return;
		}

		// otherwise, fancy comment
		IEntitySource entitySource = worldEditorAPI.CreateEntity(commentPrefab, "", worldEditorAPI.GetCurrentEntityLayerId(), null, pos, vector.Zero);
		if (!entitySource)
		{
			Print("Cannot create " + commentPrefab + " comment entity", LogLevel.ERROR);
			return;
		}

		if (SCR_BaseContainerTools.FindComponentSource(entitySource, EDITABLE_COMMENT_COMPONENT_CLASS))
		{
			array<ref ContainerIdPathEntry> path = {};
			path.Insert(new ContainerIdPathEntry(EDITABLE_COMMENT_COMPONENT_CLASS));
			worldEditorAPI.CreateObjectVariableMember(entitySource, path, "m_UIInfo", EDITABLE_COMMENT_UI_INFO_CLASS);

			path.Insert(new ContainerIdPathEntry("m_UIInfo"));
			worldEditorAPI.SetVariableValue(entitySource, path, "Name", comment);
		}

		if (SCR_BaseContainerTools.FindComponentSource(entitySource, MAP_DESCRIPTOR_COMPONENT_CLASS))
		{
			array<ref ContainerIdPathEntry> path = {};
			path.Insert(new ContainerIdPathEntry(MAP_DESCRIPTOR_COMPONENT_CLASS));
			worldEditorAPI.SetVariableValue(entitySource, path, "DisplayName", comment);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the prefab list has at least one non-empty prefab
	protected bool HasPrefabListPrefabs()
	{
		if (!m_PrefabDataList || !m_PrefabDataList.m_aPrefabsData)
			return false;

		foreach (SCR_SHPPrefabData prefabData : m_PrefabDataList.m_aPrefabsData)
		{
			if (!prefabData.m_sPrefab.IsEmpty())
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	// \param[in] id -1 for a random value
	protected ResourceName GetPrefab(int id = RANDOM_PREFAB_ID)
	{
		if (!m_PrefabDataList || !m_PrefabDataList.m_aPrefabsData || m_PrefabDataList.m_aPrefabsData.IsEmpty())
			return string.Empty;

		// if (id == RANDOM_PREFAB_ID)
		if (id < 0)
			return m_PrefabDataList.m_aPrefabsData.GetRandomElement().m_sPrefab;

		// else it's a search
		foreach (SCR_SHPPrefabData prefabData : m_PrefabDataList.m_aPrefabsData)
		{
			if (prefabData.m_iID == id)
				return prefabData.m_sPrefab;
		}

		// not found = empty
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the provided point distance is less than DUPLICATE_RADIUS from one of the provided points
	protected bool IsPointDuplicate(vector toCheck, array<vector> points)
	{
		foreach (vector point : points)
		{
			if (vector.DistanceSq(toCheck, point) < DUPLICATE_RADIUS_SQ)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! modifies forestShapeData but sets it back to its original value afterwards (duplicates a point then removes it)
	//! \param[in] forestShapeDataArray
	protected void GenerateForestGeneratorPointData(notnull array<ref SCR_GeneratorShapeImportData> forestShapeDataArray)
	{
		int count = forestShapeDataArray.Count();
		if (count < 1)
			return;

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		WBProgressDialog progress = new WBProgressDialog("Generating forest generator points data...", worldEditor);

		BaseContainerList points;
		array<SCR_GeneratorShapeImportData> collidedShapes;
		SCR_GeneratorShapeImportData otherForestShapeData;

		float prevProgress, currProgress;
		foreach (int i, SCR_GeneratorShapeImportData forestShapeData : forestShapeDataArray)
		{
			points = forestShapeData.source.GetObjectArray("Points");
			collidedShapes = {};

			for (int y = 0; y < count; y++)
			{
				if (y == i)
					continue;

				otherForestShapeData = forestShapeDataArray[y];
				if (forestShapeData.bbox.DetectCollision2D(otherForestShapeData.bbox))
					collidedShapes.Insert(otherForestShapeData);
			}

			bool wasPreviousDuplicate = false;

			forestShapeData.points.Insert(forestShapeData.points[0]); // Duplicate first point

			for (int p = 0, countPoints = forestShapeData.points.Count(); p < countPoints; p++)
			{
				bool isDuplicate = false;
				for (int y = 0, otherCount = collidedShapes.Count(); y < otherCount; y++)
				{
					if (!IsPointDuplicate(forestShapeData.points[p], collidedShapes[y].points))
						continue;

					isDuplicate = true;
					if (!wasPreviousDuplicate)
					{
						wasPreviousDuplicate = true;
						break;
					}

					worldEditorAPI.CreateObjectArrayVariableMember(points[p - 1], null, "Data", "ForestGeneratorPointData", 0);

					BaseContainerList dataArr = points[p - 1].GetObjectArray("Data");
					int dataCount = dataArr.Count();
					for (int j = 0; j < dataCount; ++j)
					{
						BaseContainer data = dataArr.Get(j);
						if (data.GetClassName() != "ForestGeneratorPointData")
							continue;

						array<ref ContainerIdPathEntry> containerPath = {
							new ContainerIdPathEntry("Points", p - 1),
							new ContainerIdPathEntry("Data", j),
						};

						worldEditorAPI.SetVariableValue(forestShapeData.source, containerPath, "m_bSmallOutline", "false");
						worldEditorAPI.SetVariableValue(forestShapeData.source, containerPath, "m_bMiddleOutline", "false");
						break;
					}

					wasPreviousDuplicate = true;
					break;
				}

				if (isDuplicate)
					continue;

				wasPreviousDuplicate = false;
			}

			forestShapeData.points.Remove(forestShapeData.points.Count() - 1); // Remove the duplicate point

			currProgress = i / count;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GeneratePowerLinePointData(notnull array<ref SCR_GeneratorShapeImportData> shapeDataArray)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		
		BaseContainerList shapePoints;
		foreach (int i, SCR_GeneratorShapeImportData forestShapeData : shapeDataArray)
		{
			if (!forestShapeData.source || !forestShapeData.points || forestShapeData.points.IsEmpty())
				continue;

			shapePoints = forestShapeData.source.GetObjectArray("Points");
			if (!shapePoints)
				continue;

			if (!worldEditorAPI.CreateObjectArrayVariableMember(shapePoints[0], null, "Data", "SCR_PowerlineGeneratorPointData", 0))
				continue;

			worldEditorAPI.SetVariableValue(shapePoints[0], { new ContainerIdPathEntry("Data", 0) }, "m_bGeneratePerPoint", "1");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AttachChildren(notnull array<ref SCR_GeneratorShapeImportData> shapeDataArray)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		WBProgressDialog progress = new WBProgressDialog("Attaching children...", worldEditor);
		int count = shapeDataArray.Count();

		float prevProgress, currProgress;
		foreach (int i, SCR_GeneratorShapeImportData shapeData : shapeDataArray)
		{
			ResourceName dataPrefab = GetPrefab(shapeData.id);
			if (dataPrefab.IsEmpty()) // id not found? go random
				dataPrefab = GetPrefab();

			if (!dataPrefab.IsEmpty())
				worldEditorAPI.CreateEntity(dataPrefab, "", 0, shapeData.source, vector.Zero, vector.Zero);

			currProgress = i / count;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Process", true)]
	protected int ButtonProcess()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}

//! See it as an array<Tuple2<int, ResourceName>>
//! id = SHP ID
//! ResourceName = corresponding prefab
[BaseContainerProps()]
class SCR_SHPPrefabDataList
{
	[Attribute(desc: "ID-Prefab list")]
	ref array<ref SCR_SHPPrefabData> m_aPrefabsData;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleFields({ "m_iID", "m_sPrefab" }, "%1 - %2")]
class SCR_SHPPrefabData
{
	[Attribute(defvalue: "-1", desc: "Prefab's .shp ID")]
	int m_iID;

	[Attribute(desc: "Prefab assigned to this ID", params: "et")]
	ResourceName m_sPrefab;
}

class SCR_GeneratorShapeImportData
{
	IEntitySource source;
	IEntity entity;
	int id;
	ref SCR_AABB bbox;
	ref array<vector> points = {};

	//------------------------------------------------------------------------------------------------
	//!
	void GenerateAAB()
	{
		bbox = new SCR_AABB(points);
	}
}
#endif // WORKBENCH
