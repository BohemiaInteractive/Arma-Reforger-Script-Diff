#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: "Import Shapefile",
	description: "Import SHP file as forest, lake, road, prefabs, comments etc.\n\n"
		+ "If Prefabs is filled, creates a prefab for each datapoint (point)\n"
		+ "or creates it as an imported shape's child (polyline/spline).\n"
		+ "- a comment prefab must have a SCR_EditableCommentComponent component\n"
		+ "- a prefab entry can be left empty in some cases (e.g comments)\n"
		+ "- a -1 id in this list prevents the SHP file to reference the entry\n"
		+ "- a negative or undefined id in the SHP file means a random prefab will be used",
	awesomeFontCode: 0xF56F)]
class SCR_ImportShapefileTool : WorldEditorTool
{
	/*
		Category: Import
	*/

	[Attribute(defvalue: "", desc: "SHP File", params: "shp", category: "Import")]
	protected ResourceName m_sSHPPath;

	[Attribute(defvalue: "0 0 0", desc: "Import offset", category: "Import")]
	protected vector m_vOffset;

	/*
		Category: Prefabs
	*/

	[Attribute(desc: "List of prefabs to be used - use either 'set class' or drag and drop an adequate SCR_SHPPrefabDataList .conf file", category: "Prefab")]
	protected ref SCR_SHPPrefabDataList m_PrefabDataList;

	[Attribute(defvalue: "1", desc: "Random direction for prefabs imported by points (point and multipoint)", category: "Prefab")]
	protected bool m_bRandomYaw;

	[Attribute(defvalue: "0", desc: "Generate ForestGenerator's Shape Point Data", category: "Prefab")]
	protected bool m_bGenerateForestGeneratorPointData;

	[Attribute(defvalue: "", desc: "Name of the column with IDs - to match Prefabs' ID", category: "Prefab")]
	protected string m_sIDColumnName;

	[Attribute(defvalue: "", desc: "Name of the column holding the comments' text - can be used with ID Column Name\nLeave empty to force Prefab import", category: "Prefab")]
	protected string m_sCommentsColumnName;

	/*
		Category: Shape
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

	//------------------------------------------------------------------------------------------------
	//! Load the SHP file and treat all the shapes present in it
	//! Multiple shape types can be present in the same SHP file (although it is not recommended data-wise)
	[ButtonAttribute("Import SHP")]
	protected void Execute()
	{
		if (m_sSHPPath.IsEmpty())
		{
			Print("Please provide an SHP file", LogLevel.WARNING);
			return;
		}

		// one -must- keep reference to the collection otherwise chaos ensues: shapes become null, attribute names become empty, etc
		GeoShapeCollection shapeCollection = GeoShapeLoader.LoadShapeFile(m_sSHPPath.GetPath());
		if (!shapeCollection)
		{
			Print("Shapefile could not be loaded", LogLevel.ERROR);
			return;
		}

		if (shapeCollection.Count() == 0)
		{
			Print("Shapefile is empty - leaving", LogLevel.NORMAL);
			return;
		}

		Debug.BeginTimeMeasure();

		if (!HasPrefabListPrefabs())
			Print("[INFO] Prefab list is null/empty", LogLevel.NORMAL);

		if (!m_API.IsDoingEditAction() && !m_API.UndoOrRedoIsRestoring())
			m_API.BeginEntityAction();

		array<ref ForestGeneratorShapeImportData> forestShapeData = {};

		// treat all shapes
		Debug.BeginTimeMeasure();
		ProcessShapes(shapeCollection, forestShapeData);
		Debug.EndTimeMeasure("Shape process");

		// woodwork
		if (!forestShapeData.IsEmpty())
		{
			if (m_bGenerateForestGeneratorPointData)
			{
				Debug.BeginTimeMeasure();
				GenerateForestGeneratorPointData(forestShapeData);
				Debug.EndTimeMeasure("Generate forest generator's point data");
			}

			Debug.BeginTimeMeasure();
			AttachChildren(forestShapeData);
			Debug.EndTimeMeasure("Attach children");
		}

		m_API.EndEntityAction();
		Debug.EndTimeMeasure("Shape import");
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessShapes(GeoShapeCollection shapeCollection, out notnull array<ref ForestGeneratorShapeImportData> forestShapeData)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WBProgressDialog progress = new WBProgressDialog("Importing geometries...", worldEditor);
		array<ref ForestGeneratorShapeImportData> forestShapePointData;
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
				PrintFormat("shape #%1/%2 is null?! Skipping", i + 1, count, level: LogLevel.WARNING);
				continue;
			}

			forestShapePointData = null;

			switch (shape.GetType())
			{
				case GeoShapeType.POINT:
					geoPoint = GeoPoint.Cast(shape);
					if (geoPoint)
						Load_Point(geoPoint);
					break;

				case GeoShapeType.MULTI_POINT:
					geoMultiPoint = GeoMultiPoint.Cast(shape);
					if (geoMultiPoint)
						Load_Multipoint(geoMultiPoint);
					break;

				case GeoShapeType.POLYLINE:
					geoPolyline = GeoPolyline.Cast(shape);
					if (geoPolyline)
						forestShapePointData = Load_Polylines(GeoPolyline.Cast(shape));
					break;

				case GeoShapeType.POLYGON:
					geoPolygon = GeoPolygon.Cast(shape);
					if (geoPolygon)
						forestShapePointData = Load_Polygons(GeoPolygon.Cast(shape));
					break;

				default:
					Print("unsupported type " + typename.EnumToString(GeoShapeType, shape.GetType()), LogLevel.WARNING);
					break;
			}

			if (forestShapePointData)
			{
				foreach (ForestGeneratorShapeImportData data : forestShapePointData)
				{
					forestShapeData.Insert(data); // can't InsertAll with array of ref
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
	protected array<ref ForestGeneratorShapeImportData> Load_Polygons(notnull GeoPolygon polygon)
	{
		bool hasIdAttribute = polygon.GetAttributes().HasAttrib(m_sIDColumnName);

		array<ref ForestGeneratorShapeImportData> result = {};
		IEntitySource entitySource;
		ForestGeneratorShapeImportData data;

		// polygon's parts (in case polygon has holes)
		for (int j = 0, count = polygon.PartsCount(); j < count; j++)
		{
			int polygonPointsCount = polygon.GetPart(j).Count();

			float traceX = polygon.GetPart(j)[0][0] + m_vOffset[0];
			float traceZ = polygon.GetPart(j)[0][2] + m_vOffset[2];
			float pntY = m_API.GetTerrainSurfaceY(traceX, traceZ) + m_vOffset[1];

			vector bb3Dmin = { traceX, pntY, traceZ };
			vector bb3Dmax = { traceX, pntY, traceZ };

			//Part's points - calculate polygon's 3D Bbox
			for (int k = 0; k < polygonPointsCount; k++)
			{
				traceX = polygon.GetPart(j)[k][0] + m_vOffset[0];
				traceZ = polygon.GetPart(j)[k][2] + m_vOffset[2];
				pntY = m_API.GetTerrainSurfaceY(traceX, traceZ) + m_vOffset[1];

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
				entitySource = m_API.CreateEntity(SPLINE_SHAPE_ENTITY_CLASS, "", m_API.GetCurrentEntityLayerId(), null, polygonOrigin, vector.Zero);
			else
				entitySource = m_API.CreateEntity(POLYLINE_SHAPE_ENTITY_CLASS, "", m_API.GetCurrentEntityLayerId(), null, polygonOrigin, vector.Zero);

			m_API.SetVariableValue(entitySource, null, "IsClosed", "1");

			data = new ForestGeneratorShapeImportData();

			// part's points - calculate point's local coords(center of 3D Bbox is the origin)
			for (int k = 0; k < polygonPointsCount - 1; k++) // polygonPointsCount - 1 because the last points duplicates the first one
			{
				vector worldCoords;
				worldCoords[0] = polygon.GetPart(j)[k][0] + m_vOffset[0];
				worldCoords[2] = polygon.GetPart(j)[k][2] + m_vOffset[2];
				worldCoords[1] = m_API.GetTerrainSurfaceY(worldCoords[0], worldCoords[2]) + m_vOffset[1];
				data.points.Insert(worldCoords);

				m_API.CreateObjectArrayVariableMember(entitySource, null, "Points", "ShapePoint", k);
				m_API.SetVariableValue(entitySource, { new ContainerIdPathEntry("Points", k) }, "Position", (worldCoords - polygonOrigin).ToString(false));
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

			m_API.SetVariableValue(entitySource, null, "LineColor", finalR.ToString() + " " + finalG.ToString() + " " + finalB.ToString() + " 1");

			data.source = entitySource;
			data.entity = m_API.SourceToEntity(entitySource);
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
	protected array<ref ForestGeneratorShapeImportData> Load_Polylines(notnull GeoPolyline polyline)
	{
		int polylinePointsCount = polyline.GetVertices().Count();

		float traceX = polyline.GetVertices()[0][0] + m_vOffset[0];
		float traceZ = polyline.GetVertices()[0][2] + m_vOffset[2];
		float pntY = m_API.GetTerrainSurfaceY(traceX, traceZ) + m_vOffset[1];

		vector bb3Dmin = { traceX, pntY, traceZ };
		vector bb3Dmax = { traceX, pntY, traceZ };

		// polyline's points - calculate polyline's 3D Bbox
		for (int k = 0; k < polylinePointsCount; k++)
		{
			traceX = polyline.GetVertices()[k][0] + m_vOffset[0];
			traceZ = polyline.GetVertices()[k][2] + m_vOffset[2];
			pntY = m_API.GetTerrainSurfaceY(traceX, traceZ) + m_vOffset[1];

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
			entitySource = m_API.CreateEntity(SPLINE_SHAPE_ENTITY_CLASS, "", m_API.GetCurrentEntityLayerId(), null, polylineOrigin, vector.Zero);
		else
			entitySource = m_API.CreateEntity(POLYLINE_SHAPE_ENTITY_CLASS, "", m_API.GetCurrentEntityLayerId(), null, polylineOrigin, vector.Zero);

		ForestGeneratorShapeImportData data = new ForestGeneratorShapeImportData();

		// polyline's points - calculate point's local coords(center of 3D Bbox is the origin)
		for (int k = 0; k < polylinePointsCount; k++)
		{
			vector worldCoords;
			worldCoords[0] = polyline.GetVertices()[k][0] + m_vOffset[0];
			worldCoords[2] = polyline.GetVertices()[k][2] + m_vOffset[2];
			worldCoords[1] = m_API.GetTerrainSurfaceY(worldCoords[0], worldCoords[2]) + m_vOffset[1];
			data.points.Insert(worldCoords);
			data.GenerateAAB();

			m_API.CreateObjectArrayVariableMember(entitySource, null, "Points", "ShapePoint", k);
			m_API.SetVariableValue(entitySource, { new ContainerIdPathEntry("Points", k) }, "Position", (worldCoords - polylineOrigin).ToString(false));
		}

		m_API.SetVariableValue(entitySource, null, "LineColor", m_vShapeColor.ToString(false) + " 1");

		data.source = entitySource;
		data.entity = m_API.SourceToEntity(entitySource);
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

		// prefab import
		Print("Importing prefab(s)", LogLevel.NORMAL);
		for (int i = 0, count = points.Count(); i < count; i++)
		{
			ResourceName chosenPrefab = GetPrefab();
			if (chosenPrefab.IsEmpty())
			{
				Print("prefab is empty, skipping", LogLevel.VERBOSE);
				continue;
			}

			vector pos = points[0] + m_vOffset;
			vector angles;
			if (m_bRandomYaw)
				angles[1] = Math.RandomFloat(0, 360);

			m_API.CreateEntityExt(chosenPrefab, "", m_API.GetCurrentEntityLayerId(), null, pos, angles, TraceFlags.WORLD);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create a comment from provided arguments. if the prefab is empty, creates a CommentEntity comment
	protected void CreateComment(string comment, vector pos, ResourceName commentPrefab = string.Empty)
	{
		if (commentPrefab.IsEmpty()) // then generic comment
		{
			IEntitySource entitySource = m_API.CreateEntity(COMMENT_ENTITY_CLASS, "", m_API.GetCurrentEntityLayerId(), null, pos, vector.Zero);
			
			m_API.SetVariableValue(entitySource, null, "m_Comment", comment);
			m_API.SetVariableValue(entitySource, null, "m_Color", COMMENT_TEXT_COLOUR.ToString(false));
			m_API.SetVariableValue(entitySource, null, "m_FaceCamera", COMMENT_FACE_CAMERA.ToString(true));
			m_API.SetVariableValue(entitySource, null, "m_TextBackground", COMMENT_TEXT_BACKGROUND.ToString(true));
			m_API.SetVariableValue(entitySource, null, "m_Size", COMMENT_TEXT_SIZE.ToString());
			return;
		}

		// otherwise, fancy comment
		IEntitySource entitySource = m_API.CreateEntity(commentPrefab, "", m_API.GetCurrentEntityLayerId(), null, pos, vector.Zero);
		if (!entitySource)
		{
			Print("Could not create " + commentPrefab + " comment entity", LogLevel.ERROR);
			return;
		}

		if (SCR_BaseContainerTools.FindComponentSource(entitySource, EDITABLE_COMMENT_COMPONENT_CLASS))
		{
			array<ref ContainerIdPathEntry> path = {};
			path.Insert(new ContainerIdPathEntry(EDITABLE_COMMENT_COMPONENT_CLASS));
			m_API.CreateObjectVariableMember(entitySource, path, "m_UIInfo", EDITABLE_COMMENT_UI_INFO_CLASS);

			path.Insert(new ContainerIdPathEntry("m_UIInfo"));
			m_API.SetVariableValue(entitySource, path, "Name", comment);
		}

		if (SCR_BaseContainerTools.FindComponentSource(entitySource, MAP_DESCRIPTOR_COMPONENT_CLASS))
		{
			array<ref ContainerIdPathEntry> path = {};
			path.Insert(new ContainerIdPathEntry(MAP_DESCRIPTOR_COMPONENT_CLASS));
			m_API.SetVariableValue(entitySource, path, "DisplayName", comment);
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
	protected void GenerateForestGeneratorPointData(notnull array<ref ForestGeneratorShapeImportData> forestShapeDataArray)
	{
		int count = forestShapeDataArray.Count();
		if (count < 1)
			return;

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WBProgressDialog progress = new WBProgressDialog("Generating forest generator points data...", worldEditor);

		BaseContainerList points;
		array<ForestGeneratorShapeImportData> collidedShapes;
		ForestGeneratorShapeImportData otherForestShapeData;

		float prevProgress, currProgress;
		foreach (int i, ForestGeneratorShapeImportData forestShapeData : forestShapeDataArray)
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

					m_API.CreateObjectArrayVariableMember(points[p - 1], null, "Data", "ForestGeneratorPointData", 0);

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
						m_API.SetVariableValue(forestShapeData.source, containerPath, "m_bSmallOutline", "false");
						m_API.SetVariableValue(forestShapeData.source, containerPath, "m_bMiddleOutline", "false");
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
	protected void AttachChildren(notnull array<ref ForestGeneratorShapeImportData> forestShapeDataArray)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WBProgressDialog progress = new WBProgressDialog("Attaching children...", worldEditor);
		int count = forestShapeDataArray.Count();

		float prevProgress, currProgress;
		foreach (int i, ForestGeneratorShapeImportData shapeData : forestShapeDataArray)
		{
			ResourceName dataPrefab = GetPrefab(shapeData.id);
			if (dataPrefab.IsEmpty()) // id not found? go random
				dataPrefab = GetPrefab();

			if (!dataPrefab.IsEmpty())
				m_API.CreateEntity(dataPrefab, "", 0, shapeData.source, vector.Zero, vector.Zero);

			currProgress = i / count;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}
		}
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

class ForestGeneratorShapeImportData
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
