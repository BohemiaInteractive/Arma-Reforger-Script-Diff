//-----------------------------------------------------------------------
[EntityEditorProps(category: "GameLib/Scripted/Generator", description:"RoadGeneratorEntity", dynamicBox: true, visible: false)]
class RoadGeneratorEntityClass: GeneratorBaseEntityClass
{
}

//-----------------------------------------------------------------------
class RoadGeneratorEntity : GeneratorBaseEntity
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "If enabled, adjust terrain height map to road", category: "Terrain")]
	private bool AdjustHeightMap; //!< Read by c++ RoadTerrainLayer

	[Attribute(defvalue: "0", desc: "Priority of terrain heightmap adjust", category: "Terrain")]
	private int AdjustHeightMapPriority; //!< Read by c++ RoadTerrainLayer

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "If enabled, generate RoadEntity", category: "Road")]
	private bool GenerateRoad;
	
	[Attribute(defvalue: "2", uiwidget: UIWidgets.EditBox, desc: "Width of the clearance by the road", category: "Road")]
	private float RoadClearance; //!< Used by ForestGeneratorEntity

	[Attribute(defvalue: "10", uiwidget: UIWidgets.EditBox, desc: "Width of the road", category: "Road", params: "0.1 100.0 0.1")]
	private float RoadWidth; //!< Read by c++ RoadTerrainLayer

	[Attribute(defvalue: "2", uiwidget: UIWidgets.EditBox, desc: "Distance between road edge and start of fall-off", category: "Terrain")]
	private float FalloffStartWidth; //!< Read by c++ RoadTerrainLayer

	[Attribute(defvalue: "20", uiwidget: UIWidgets.EditBox, desc: "Width of the road fall-off", category: "Terrain")]
	private float RoadFalloffWidth; //!< Read by c++ RoadTerrainLayer

#ifdef WORKBENCH	
	private ShapeEntity m_ShapeEntity; //!< Read by c++ RoadTerrainLayer
	private IEntitySource m_ShapeSrc;
	private IEntitySource m_RoadGenSrc;

	//-----------------------------------------------------------------------
	float GetRoadClearance()
	{
		return RoadClearance;
	}
	
	//-----------------------------------------------------------------------
	float GetRoadWidth()
	{
		return RoadWidth;
	}
	
	//-----------------------------------------------------------------------
	protected void RoadGeneratorEntity(IEntitySource src, IEntity parent)
	{
		if (_WB_GetEditorAPI())
		{
			SetEventMask(EntityEvent.INIT);
	
			m_RoadGenSrc = src;
			
			IEntitySource parentSrc = m_RoadGenSrc.GetParent();
			ShapeEntity shapeEnt = ShapeEntity.Cast(parent);
			if (parentSrc && shapeEnt)
			{
				OnShapeInit(parentSrc, shapeEnt);
			}
		}		
	}
	
	//-----------------------------------------------------------------------
	protected void ~RoadGeneratorEntity()
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		
		if (api)
		{
			api.RemoveTerrainFlatterEntity(this, false);
		}
	}

	//-----------------------------------------------------------------------
	override bool _WB_CanSelect(IEntitySource src)
	{
		return false;
	}
	
	//-----------------------------------------------------------------------
	override void _WB_OnDelete(IEntitySource src)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api)
		{
			api.RemoveTerrainFlatterEntity(this, true);
		}
	}
	
	//-----------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api.AreGeneratorEventsEnabled())
			return false;
		
		BaseContainerTools.WriteToInstance(this, src);

		// if there is no legit spline entity
		if( !m_ShapeEntity )
		{
			Print("RoadGeneratorEntity requires a SplineShapeEntity!", LogLevel.ERROR);
			return false;
		}
		
		array<vector> updateMins = new array<vector>();
		array<vector> updateMaxes = new array<vector>();
		
		m_ShapeEntity.GetAllInfluenceBBoxes(m_ShapeSrc, updateMins, updateMaxes);
		
		bool bForceUpdate = false;
				
		if (key == "AdjustHeightMap")
		{
			if (AdjustHeightMap)
			{
				bForceUpdate = true;	
			}
			else
			{
				api.RemoveTerrainFlatterEntity(this, true);
			}
		}
		
		UpdateTerrain(m_ShapeEntity, bForceUpdate, updateMins, updateMaxes);
		UpdateRoad(m_ShapeSrc);
		
		return true;
	}
	
	//-----------------------------------------------------------------------
	override void _WB_OnCreate(IEntitySource src)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		
		if (api && m_ShapeEntity)
		{
			IEntitySource shapeEntitySrc = src.GetParent();
			array<vector> updateMins = new array<vector>();
			array<vector> updateMaxes = new array<vector>();
			m_ShapeEntity.GetAllInfluenceBBoxes(m_ShapeSrc, updateMins, updateMaxes);
			
			UpdateRoad(m_ShapeSrc, false);
			UpdateTerrain(m_ShapeEntity, false, updateMins, updateMaxes);
		}
	}
	
	//-----------------------------------------------------------------------
	protected override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		m_ShapeEntity = shapeEntity;
		m_ShapeSrc = shapeEntitySrc;
		
		WorldEditorAPI api = _WB_GetEditorAPI();
		
		if (api)
		{
			array<vector> updateMins = new array<vector>();
			array<vector> updateMaxes = new array<vector>();
			shapeEntity.GetAllInfluenceBBoxes(m_ShapeSrc, updateMins, updateMaxes);
			
			UpdateTerrain(m_ShapeEntity, false, updateMins, updateMaxes);
		}
	}

	//-----------------------------------------------------------------------
	protected override void OnShapeTransformInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		UpdateTerrain(shapeEntity, false, mins, maxes);
		UpdateRoad(shapeEntitySrc);		
		
	}

	//-----------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		UpdateTerrain(shapeEntity, false, mins, maxes);
		UpdateRoad(shapeEntitySrc);		
	}

	//-----------------------------------------------------------------------
	
	protected void UpdateRoad(IEntitySource shapeEntitySrc, bool requireRoad = true)
	{
		if (GenerateRoad)
		{
			UpdateRoadEntity(_WB_GetEditorAPI(), shapeEntitySrc, m_RoadGenSrc, RoadWidth, requireRoad);
		}
	}

	//-----------------------------------------------------------------------
	protected void UpdateTerrain(ShapeEntity shapeEntity, bool bForceUpdate, array<vector> updateMins, array<vector> updateMaxes)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();

		// update terrain
		if (!api || !AdjustHeightMap)
			return;
		
		vector mins = vector.One * 100000;
		vector maxs = -mins;

		array<vector> roadPoints = new array<vector>;
		vector mat[4];

		m_ShapeEntity.GenerateTesselatedShape(roadPoints);
		m_ShapeEntity.GetWorldTransform(mat);

		for (int i = 0; i < roadPoints.Count(); i++)
		{
			vector pos = roadPoints[i].Multiply4(mat);
			mins[0] = Math.Min(mins[0], pos[0]);
			mins[1] = Math.Min(mins[1], pos[1]);
			mins[2] = Math.Min(mins[2], pos[2]);

			maxs[0] = Math.Max(maxs[0], pos[0]);
			maxs[1] = Math.Max(maxs[1], pos[1]);
			maxs[2] = Math.Max(maxs[2], pos[2]);
		}

		api.AddTerrainFlatterEntity(this, mins, maxs, AdjustHeightMapPriority, RoadWidth * 0.5 + FalloffStartWidth, RoadFalloffWidth, bForceUpdate, updateMins, updateMaxes);
	}

	//-----------------------------------------------------------------------
	// this method modify entities, so it is static works just with sources
	static void UpdateRoadEntity(WorldEditorAPI api, IEntitySource splineSrc, IEntitySource genSrc, float roadWidth, bool requireRoad)
	{
		if (api.UndoOrRedoIsRestoring() || !api.IsModifyingData())
			return;
		
		// spline
		SplineShapeEntity splineEntity = SplineShapeEntity.Cast(api.SourceToEntity(splineSrc));
		if (!splineEntity)
		{
			Print("RoadGeneratorEntity requires a SplineShapeEntity!", LogLevel.ERROR);
			return;
		}

		// road
		IEntitySource roadSrc;
		if (genSrc)
		{
			IEntitySource child = null;
			int numChildren = genSrc.GetNumChildren();
			for (int i = 0; i < numChildren; i++)
			{
				child = genSrc.GetChild(i);
				if (child.GetClassName() == "RoadEntity")
				{
					roadSrc = child;
					break;
				}
			}
		}
		if (!roadSrc)
		{
			if (requireRoad)
			{
				Print("RoadGeneratorEntity requires RoadEntity as child in hierarchy to be able to generate the road!", LogLevel.ERROR);
			}
			return;
		}

		// All good, update the road entity!
		
		//optimization. Entity roadSrc will not be re-initialized until the api.EndEditSequence(roadSrc) bellow will be called
		api.BeginEditSequence(roadSrc);

		// set some properties
		bool isClosed;
		splineSrc.Get("IsClosed", isClosed);
		
		if (isClosed)
			api.SetVariableValue(roadSrc, null, "IsClosedSpline", "true");
		else
			api.SetVariableValue(roadSrc, null, "IsClosedSpline", "false");
		api.SetVariableValue(roadSrc, null, "Width", roadWidth.ToString());
		api.SetVariableValue(roadSrc, null, "Points", "@@@"); // clear obsolete Points property
		
		// Copy the points:

		// spline world transform
		vector wTrans[4];
		splineEntity.GetWorldTransform(wTrans);

		// spline anchors positions
		array<vector> positions = new array<vector>;
		splineEntity.GetPointsPositions(positions);
		auto roadPoints = roadSrc.GetObjectArray("SplinePoints");

		// for each anchor of the spline...
		int i=0;
		for (; i<positions.Count(); i++)
		{
			// insert point to the road if needed
			if (i >= roadPoints.Count())
			{
				api.CreateObjectArrayVariableMember(roadSrc, null, "SplinePoints", "ShapePoint", i);
				roadPoints = roadSrc.GetObjectArray("SplinePoints"); // needs to be updated after the source change
			}

			auto roadPoint = roadPoints.Get(i);

			auto containerPath = new array<ref ContainerIdPathEntry>();
			containerPath.Insert(new ContainerIdPathEntry("SplinePoints", i));

			// get the position, transform it to the road's space
			positions[i] = positions[i].Multiply4(wTrans);
			positions[i] = api.SourceToEntity(roadSrc).CoordToLocal(positions[i]);


			// set the position if changed
			vector oldPos;
			roadPoint.Get("Position", oldPos);
			if (oldPos != positions[i]) // just to save some unneeded change on the undostack, possible TODO compare with some tolerance
				api.SetVariableValue(roadSrc, containerPath, "Position", positions[i].ToString(false));

			// update point data
			// (should be empty if there are not explicit tangents in the source point, contain one entry with the tangents otherwise)
			int numPointData = roadPoint.GetObjectArray("Data").Count();
			if (splineEntity.HasPointExplicitTangents(i))
			{
				// ensure there is just one item of time SplinePointData
				bool createSplineData = numPointData < 1 || roadPoint.GetObjectArray("Data").Get(0).GetClassName() != "SplinePointData";
				if (createSplineData)
					api.CreateObjectArrayVariableMember(roadSrc, containerPath, "Data", "SplinePointData", 0);
				for (int j=numPointData-1; j>0; j--)
					api.RemoveObjectArrayVariableMember(roadSrc, containerPath, "Data", j);
				roadPoints	= roadSrc.GetObjectArray("SplinePoints"); // needs to be updated after the source change
				roadPoint	= roadPoints.Get(i); // ditto

				// update the tangents if needed

				vector tIn, tOut;
				splineEntity.GetTangents(i, tIn, tOut);

				vector tInOld, tOutOld;
				auto oldTangents = roadPoint.GetObjectArray("Data").Get(0);
				oldTangents.Get("InTangent", tInOld);
				oldTangents.Get("OutTangent", tOutOld);

				if (tIn != tInOld || tOut != tOutOld) // just to save some unneeded change on the undostack, possible TODO: compare with some tolerance
				{
					containerPath.Insert(new ContainerIdPathEntry("Data", 0));
					api.SetVariableValue(roadSrc, containerPath, "InTangent", tIn.ToString(false));
					api.SetVariableValue(roadSrc, containerPath, "OutTangent", tOut.ToString(false));
					roadPoints	= roadSrc.GetObjectArray("SplinePoints"); // needs to be updated after the source change
					roadPoint	= roadPoints.Get(i); // ditto
				}

			}
			else
			{
				// no explicit tangents -> clear the point data
				for (int j=numPointData-1; j>=0; j--)
					api.RemoveObjectArrayVariableMember(roadSrc, containerPath, "Data", j);
			}
		}

		// remove the abundant points in road entity (if any)
		while (i < roadPoints.Count())
			api.RemoveObjectArrayVariableMember(roadSrc, null, "SplinePoints", roadPoints.Count() - 1);
			
		api.EndEditSequence(roadSrc);
	}
	#endif
}

