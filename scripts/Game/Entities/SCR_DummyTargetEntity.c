[EntityEditorProps(category: "GameScripted/Shapes", description: "Procedural box", color: "255 0 0 255", style: "box", dynamicBox: true)]
class SCR_DummyTargetEntityClass: SCR_GenericBoxEntityClass
{
};

/*!
Procedural box entity.
*/
class SCR_DummyTargetEntity : SCR_GenericBoxEntity
{
	[Attribute("10", UIWidgets.Slider, "", "0 100 0.1")]
	private float m_fDefaultTimeBetweenPoints;	
	
	private BaseWorld m_World;
	private IEntitySource m_Source;
	
	private ref array<vector> m_aPathPoints = {};
	private ref array<float> m_aPathSpeedValues = {};
	
	private vector m_vCurrentPathPoint;
	private vector m_vNextPathPoint;
	private vector m_targetWorldPosition;
	
	private int m_iCurrentPathPointIndex;
	private int m_iNextPathPointIndex = m_iCurrentPathPointIndex+1;
	
	private ref SimplePreload m_Preload;
	
	override protected void EOnInit(IEntity owner)
	{
		m_World = GetWorld();

		if (GetChildren())
		{
			PolylineShapeEntity polyline = PolylineShapeEntity.Cast(GetChildren());
			GetPathFromPolyline(polyline, m_aPathPoints);

			int num = m_Source.GetNumChildren();
			IEntitySource pnt = m_Source.GetChild(0);
			BaseContainerList points = pnt.GetObjectArray("Points");

			if (points != null)
			{
				for (int i = 0; i < points.Count(); ++i)
				{
					BaseContainer point = points.Get(i);
					BaseContainerList data_array = point.GetObjectArray("Data");

					for (int j = 0; j < data_array.Count(); ++j)
					{
						BaseContainer data = data_array.Get(j);
						bool hasPointData = false;
						if (data.GetClassName() == "SCR_DummyTargetEntity")
						{
							float travel_time;
							data.Get("m_fTimeToTravel", travel_time);
							m_aPathSpeedValues.Insert(travel_time);
						}
					}
				}
			}

			m_vCurrentPathPoint = m_aPathPoints[m_iCurrentPathPointIndex];
			m_vNextPathPoint = m_aPathPoints[m_iNextPathPointIndex];	
			owner.SetOrigin(m_vCurrentPathPoint);			 		
		}		
	}
	
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{	
		vector target_position = owner.GetOrigin();
		
		if (VectorEqualApprox(target_position, m_vNextPathPoint, 1))
		{
			m_iCurrentPathPointIndex++;
			if (m_iCurrentPathPointIndex == m_aPathPoints.Count())
				m_iCurrentPathPointIndex = 0;

			m_iNextPathPointIndex++;
			if (m_iNextPathPointIndex == m_aPathPoints.Count())
				m_iNextPathPointIndex = 0;

			m_vCurrentPathPoint = m_aPathPoints[m_iCurrentPathPointIndex];
			m_vNextPathPoint = m_aPathPoints[m_iNextPathPointIndex];

		}

		// get the speed values
		float cur_time_to_travel = m_fDefaultTimeBetweenPoints;
		if (m_aPathSpeedValues.Count() > 0)
		{
			int id = m_iCurrentPathPointIndex;
			if (id >= m_aPathSpeedValues.Count())
			{
				id = m_aPathSpeedValues.Count()-1;
			}
			cur_time_to_travel = m_aPathSpeedValues[id];
		}

		// calculate the target speed
		float dist = vector.Distance(m_vCurrentPathPoint, m_vNextPathPoint);
		float velocity = dist / cur_time_to_travel;
		float cur_camera_speed = velocity * timeSlice;

		vector movement_dir = vector.Direction(m_vCurrentPathPoint, m_vNextPathPoint).Normalized();
		// resulting camera movement:
		target_position = target_position + movement_dir * cur_camera_speed;
			
		this.SetOrigin(target_position);		
	}
	
	
	private void GetPathFromPolyline(PolylineShapeEntity polyline, out array <vector> path)
	{
		vector offset = polyline.GetOrigin();
		array <vector> temp_path = {};
		polyline.GetPointsPositions(temp_path);

		for (int i = 0; i < temp_path.Count(); ++i)
		{
			temp_path[i] = temp_path[i] + offset;
		}
		path = temp_path;
	}

	private bool VectorEqualApprox(vector v1, vector v2, float epsilon = 1)
	{
		return (Math.AbsFloat(v1[0] - v2[0]) < epsilon)
			&& (Math.AbsFloat(v1[1] - v2[1]) < epsilon)
			&& (Math.AbsFloat(v1[2] - v2[2]) < epsilon);
	}
	
	void SCR_DummyTargetEntity(IEntitySource src, IEntity parent)
	{
		m_Source = src;
		SetEventMask(EntityEvent.FRAME | EntityEvent.INIT);
	}
};