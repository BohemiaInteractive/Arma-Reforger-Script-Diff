[EntityEditorProps(category: "GameLib/Generic", description:"Crossroad", dynamicBox: true)]
class CrossroadEntityClass: GenericEntityClass
{
}

class CrossroadEntity : GenericEntity
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "If enabled, adjust terrain height map to entity", category: "Terrain")]
	private bool AdjustHeightMap; 
	
	[Attribute(defvalue: "0", desc: "Priority of terrain heightmap adjust", category: "Terrain")]
	private int AdjustHeightMapPriority;
	
	[Attribute(defvalue: "2", uiwidget: UIWidgets.EditBox, desc: "Distance between edge and start of fall-off", category: "Terrain")]
	private float FalloffStartWidth;
	
	[Attribute(defvalue: "20", uiwidget: UIWidgets.EditBox, desc: "Width of the fall-off", category: "Terrain")]
	private float FalloffWidth;
	
	protected void CrossroadEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask( EntityEvent.INIT );
	}
	
	//-----------------------------------------------------------------------
	protected void ~CrossroadEntity()
	{
	#ifdef WORKBENCH
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api)
		{
			api.RemoveTerrainFlatterEntity(this);
		}
	#endif
	}
	
	override protected void EOnInit(IEntity owner)
	{
		Init();
	}
	
#ifdef WORKBENCH	
	ref array<vector> m_Bones = new array<vector>;
	
	void Init()
	{
		vector mat[4];
		auto boneNames = new array<string>;
		Animation anim = GetAnimation();
		anim.GetBoneNames(boneNames);
		foreach(string bone: boneNames)
		{
			int idx = anim.GetBoneIndex(bone);
			anim.GetBoneMatrix(idx, mat);
			m_Bones.Insert(mat[3]);
		}
		
		UpdateTerrain();
	}
	
	void UpdateTerrain()
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api) return;
		
		if (AdjustHeightMap)
		{
			vector mins;
			vector maxs;
			GetWorldBounds(mins, maxs);
			
			api.AddTerrainFlatterEntity(this, mins, maxs, AdjustHeightMapPriority, FalloffStartWidth, FalloffWidth);
		}
		else
		{
			api.RemoveTerrainFlatterEntity(this);
		}
	}

	override bool _WB_CanAnchorSnap(IEntitySource thisSrc, int thisAnchor, IEntitySource otherSrc, int otherAnchor, bool isReceiver) 
	{
		if (isReceiver && otherSrc.GetClassName() == "SplineShapeEntity")
		{
			BaseContainerList points = otherSrc.GetObjectArray("Points");
			if (otherAnchor == 0 || otherAnchor == points.Count() - 1)
				return true; 
		}
		
		return false;
	}
	
	vector GetTangent(ShapeEntity shape, int pIdx)
	{
		array<vector> points = new array<vector>();
		shape.GetPointsPositions(points);
		
		int nPoints = points.Count();
		if (nPoints < 2)
			return vector.Zero;
	
		
		if (pIdx == 0)
		{
			vector res = (points[1] - points[0]);
			res.Normalize();
			return res;
		}
	
		if (pIdx >= nPoints - 1)
		{
			vector res = (points[nPoints - 1] - points[nPoints - 2]);
			res.Normalize();
			return res;
		}
		
		return vector.Zero;
	}
		
	override void _WB_OnAnchorSnapped(IEntitySource thisSrc, int thisAnchor, IEntitySource otherSrc, int otherAnchor, bool isReceiver)
	{
		if (isReceiver && otherSrc.GetClassName() == "SplineShapeEntity")
		{
			BaseContainerList points = otherSrc.GetObjectArray("Points");
			WorldEditorAPI api = _WB_GetEditorAPI();
			ShapeEntity shape = ShapeEntity.Cast(api.SourceToEntity(otherSrc));
			int boneIdx = thisAnchor + 1;
			vector mat[4];
			
			if (points && points.Count() > otherAnchor && shape)
			{	
				// get crossroad point tangent
				
				GetWorldTransform(mat);
				
				// apply bone transform
				vector crossroadTangent = (m_Bones[boneIdx] - m_Bones[0]).Normalized().Multiply3(mat);
				
				// get spline point tangent			
				shape.GetWorldTransform(mat);
				vector splineTangent = GetTangent(shape, otherAnchor).Multiply3(mat);
				
				float len = Math.Max(0.2, 1.0 - Math.AbsFloat(vector.Dot(-splineTangent, crossroadTangent)));
				vector finalSplineTangent = crossroadTangent.InvMultiply3(mat) * len * 300;
				
				api.BeginEntityAction("Snap to crossroad");
				// adjust spline control point
				BaseContainer point = points[otherAnchor];
				BaseContainerList pointData = point.GetObjectArray("Data");
				if (pointData.Count() == 0)
				{
					api.CreateObjectArrayVariableMember(point, null, "Data", "SplinePointData", 0);
				}

				array<ref ContainerIdPathEntry> containerPath = {ContainerIdPathEntry("Points", otherAnchor), ContainerIdPathEntry("Data", 0)};
				if (otherAnchor == 0)
				{
					api.SetVariableValue(otherSrc, containerPath, "OutTangent", finalSplineTangent.ToString(false));		
				}
				else
				{
					api.SetVariableValue(otherSrc, containerPath, "InTangent", (-finalSplineTangent).ToString(false));		
				}
								
				//adjust bone
				
				api.EndEntityAction();
			}
		}
	}
	
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		UpdateTerrain();
		return false;
	}	
	
	override int _WB_GetAnchorCount(IEntitySource src)
	{
		int cnt = m_Bones.Count();
		if (cnt > 2)
			return m_Bones.Count() - 1;
		return 0;
	}
	
	override void _WB_GetAnchor(inout vector position, IEntitySource src, int index)
	{
		position = m_Bones[index + 1];
	}
#else
	void Init()
	{
	}
#endif
}
