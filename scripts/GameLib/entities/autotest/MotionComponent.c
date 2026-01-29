// Script File

[EntityEditorProps(category: "GameLib/Scripted/Autotest", description:"MotionAutoTest", dynamicBox: true)]
class MotionAutoTestClass: GenericEntityClass
{
}

class Measurement
{
	float m_Min = 100000000000;
	float m_Max = 0;
	int m_CountSamples = 0;
	float m_CurrentSum = 0;
	
	void AddValue(float value)
	{
		if (value < m_Min)
		{
			m_Min = value;
		}
		
		if (value >= m_Max)
		{
			m_Max = value;
		}
		
		m_CurrentSum += value;
		m_CountSamples++;
	}
	
	float ComputeAverage()
	{
		if (m_CountSamples <= 0)
			return 0;
		return m_CurrentSum / (float)m_CountSamples;
	}
}

[EntityEditorProps(category: "GameLib/Scripted/Autotest", description:"MotionZonePoint", dynamicBox: true)]
class MotionZonePointClass: GenericEntityClass
{
}

class MotionZonePoint : GenericEntity
{
	void MotionZonePoint(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		
		parent.AddChild(this, -1);
	}
}

[EntityEditorProps(category: "GameLib/Scripted/Autotest", description:"MotionZone", dynamicBox: true)]
class MotionZoneClass: GenericEntityClass
{
}

class MotionZone : GenericEntity
{
	vector m_Min;
	vector m_Max;
	
	void MotionZone(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
		
		m_Min = Vector(0, 0, 0);
		m_Max = Vector(0, 0, 0);
	}
	
	vector GetMin()
	{
		return m_Min;
	}
	
	vector GetMax()
	{
		return m_Max;
	}

	void ~MotionZone()
	{		
	}
	
	void Initialize()
	{
		IEntity ent1st = MotionZonePoint.Cast(GetChildren());
		IEntity ent2nd = ent1st.GetSibling();
		
		vector p1 = ent1st.GetOrigin();
		vector p2 = ent2nd.GetOrigin();
		
		m_Min[0] = Math.Min(p1[0], p2[0]);
		m_Min[1] = Math.Min(p1[1], p2[1]);
		m_Min[2] = Math.Min(p1[2], p2[2]);
		
		m_Max[0] = Math.Max(p1[0], p2[0]);
		m_Max[1] = Math.Max(p1[1], p2[1]);
		m_Max[2] = Math.Max(p1[2], p2[2]);
	}
	
	override void EOnInit(IEntity owner)
	{
		Initialize();
	}
}

class MotionAutoTest: GenericEntity
{
	[Attribute("Camera1", UIWidgets.EditBox, "Name of camera entity", "")]
	private string m_cameraEntityName; //< name of camera entity which will be used to take screenshots

	[Attribute("60", UIWidgets.Slider, "Which FPS is considered as minimal", "0 240 1")]
	private float m_FPSLimit; //< this time is waited for the screenshot to be captured and saved on disk
	
	[Attribute("1.8", UIWidgets.Slider, "Distance between surface and camera on y-axis", "0 20 0.1")]
	private float m_HeightAboveSurface;
	[Attribute("5", UIWidgets.Slider, "Movement speed", "0 100 1")]
	private float m_MovementSpeed;

	[Attribute("true", UIWidgets.CheckBox, "If true a path is generated otherwise the path is created from child waypoints")]
	private bool m_GeneratePath;
	[Attribute("false", UIWidgets.CheckBox, "If true the generated path is drawn as a line - for debug purposes")]
	private bool m_ShowGeneratePath;
	[Attribute("0.15", UIWidgets.Slider, "The scale of the generated path that is centered in the map center", "0 1 0.01")]
	private float m_GeneratedPathScale; 
	[Attribute("10", UIWidgets.Slider, "Number of sectors for the generated path", "1 100 1")]
	private float m_GeneratedPathSectorsCount;

	private IEntity m_camera;
	private Screenshot_Waypoint m_waypoint;
	private string m_directory;
	
	private TextWidget m_FPSWidget;
	
	private ref array<Shape> m_CreatedLines;
	
	private float m_fTimeToTravel = -1;
	private float m_fTimeTraveled = 0;
	
	private int m_iCurrentPointsI;
	private ref array<vector> m_Points;
	protected float m_timeFromStart;
	protected float m_timeLastStatsSnapshot;
	
	private ref Measurement m_MeasureFPS;
	private ref Measurement m_MeasureMemory;
	
	private ref array<ref Measurement> m_MeasurementMemory;
	
	[Attribute("MotionZone", UIWidgets.EditBox, "Motion Zone Object Name", "")]
	private string m_MotionZone;
	
	private MotionZone m_MotionZoneInstance;
	
	void MotionAutoTest(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		m_CreatedLines = new array<Shape>();
		m_MeasureFPS = new Measurement();
		m_MeasureMemory = new Measurement();
		m_MeasurementMemory = new array<ref Measurement>();
		int mc = MemoryStatsSnapshot.GetStatsCount();
		for (int i = 0; i < mc; i++)
		{
			m_MeasurementMemory.Insert(new Measurement());
		}
		
		m_timeLastStatsSnapshot = 0;

		System.GetCLIParam("autotest-output-dir", m_directory);

		if (m_directory.Length() == 0)
		{
			m_directory = "$logs:" + GetName();
		}
	}

	void ~MotionAutoTest()
	{		
		
		for (int i = 0; i < m_CreatedLines.Count(); i++)
		{
			Shape s = m_CreatedLines[i];
			delete s;
		}
		
		delete m_CreatedLines;
		delete m_Points;		
		
	}
	
	void UpdateMeasurements()
	{
		int f = System.GetFPS();
		
		m_MeasureFPS.AddValue(f);
		m_MeasureMemory.AddValue(System.MemoryAllocationKB());
		
		if (m_timeFromStart - m_timeLastStatsSnapshot > 2)
		{
			m_timeLastStatsSnapshot = m_timeFromStart;

			auto snapshot = new MemoryStatsSnapshot();
			int count = snapshot.GetStatsCount();
			for (int i = 0; i < count; i++)
			{
				int value = snapshot.GetStatValue(i);
				m_MeasurementMemory[i].AddValue(value);
			}
		}
	}
	
	void CreateLines(array<vector> points, int color)
	{
		for (int i = 0; i < points.Count(); i++)
		{
			if (i + 1 < points.Count())
			{
				vector ppp[2];
				ppp[0] = points[i];
				ppp[1] = points[i + 1];
				
				m_CreatedLines.Insert(Shape.CreateLines(color, ShapeFlags.VISIBLE, ppp, 2));
			}
		}
	}
	
	private void GeneratePathFromWaypoints()
	{
		m_Points = new array<vector>();
		IEntity ee = GetChildren();				

		while (ee)
		{
			if (ee.GetSibling())
			{
				m_Points.Insert(ee.GetOrigin());
			}
			else
			{
				if (ee)
				{
					m_Points.Insert(ee.GetOrigin());
				}
			}

			Print("??? -> " + ee.GetName());
			ee = ee.GetSibling();
		}	
	}
	
	private void GeneratePath()
	{
		if (m_MotionZoneInstance)
		{
			Print("Bouding Area of motion zone |" + m_MotionZoneInstance.m_Min + "|" + m_MotionZoneInstance.m_Max);
			
			Initialize(m_GeneratedPathScale, m_GeneratedPathSectorsCount, m_MotionZoneInstance.m_Min, m_MotionZoneInstance.m_Max);
		}
		else
		{
			vector mmin;
			vector mmax;
			GetWorld().GetBoundBox(mmin, mmax);
		
			Print("World Bouding box |" + mmin + "|" + mmax);
			
			Initialize(m_GeneratedPathScale, m_GeneratedPathSectorsCount, mmin, mmax);
		}
	}
	
	override void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		m_camera = g_Game.FindEntity(m_cameraEntityName);

		if (!m_camera)
		{
			Print("Camera not found 2!", LogLevel.ERROR);
		}
		
		m_MotionZoneInstance = MotionZone.Cast(g_Game.FindEntity(m_MotionZone));
		
		if (m_MotionZoneInstance != null)
		{
			m_MotionZoneInstance.Initialize();
		}
		
		
		if (m_GeneratePath)
		{		
			GeneratePath();
		}
		else
		{
			GeneratePathFromWaypoints();
		}
	
		TransformCameraToWaypoint();
		
		m_FPSWidget = TextWidget.Cast(g_Game.GetWorkspace().CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 16, 16, 512, 128, WidgetFlags.VISIBLE, new Color(0.0, 0.0, 0.0, 1.0), 1024));
		m_FPSWidget.SetExactFontSize(64);
	}
	
	private void MakeSummeryFile(string filename)
	{
		FileHandle descrFile = FileIO.OpenFile(filename, FileMode.WRITE);

		if(descrFile)
		{
			int sizeX = g_Game.GetWorkspace().GetWidth();
			int sizeY = g_Game.GetWorkspace().GetHeight();
			descrFile.WriteLine(string.Format("MOTION AUTO TEST"));
			descrFile.WriteLine(string.Format("Resolution (px): %1x%2", sizeX, sizeY));
#ifdef WORKBENCH
			descrFile.WriteLine(string.Format("Entering playmode time (s): %1", g_Game.GetLoadTime() / 1000));
#else
			descrFile.WriteLine(string.Format("Load time (s): %1", g_Game.GetLoadTime() / 1000));
#endif
			descrFile.WriteLine(string.Format("Duration (s): %1", m_timeFromStart));
			
			descrFile.WriteLine(string.Format("FPS average: %1", m_MeasureFPS.ComputeAverage()));
			descrFile.WriteLine(string.Format("FPS min: %1", m_MeasureFPS.m_Min));
			descrFile.WriteLine(string.Format("FPS max: %1", m_MeasureFPS.m_Max));
			
			int count = MemoryStatsSnapshot.GetStatsCount();
			for (int i = 0; i < count; i++)
			{
				string name = MemoryStatsSnapshot.GetStatName(i);
				descrFile.WriteLine(string.Format("%1 average: %2", name, (int)m_MeasurementMemory[i].ComputeAverage()));
				descrFile.WriteLine(string.Format("%1 min: %2", name, (int)m_MeasurementMemory[i].m_Min));
				descrFile.WriteLine(string.Format("%1 max: %2", name, (int)m_MeasurementMemory[i].m_Max));
			}
			
			descrFile.WriteLine(string.Format("Timestamp: %1", GetCurrentTimestamp()));
			
			descrFile.Close();
			Print("Summary file successfully saved into " + filename);
		}
	}
	
	private void MakeCSVFile(string filename)
	{
		FileHandle descrFile = FileIO.OpenFile(filename, FileMode.WRITE);
		
		if (!descrFile)
			return;
		
		int sizeX = g_Game.GetWorkspace().GetWidth();
		int sizeY = g_Game.GetWorkspace().GetHeight();
		
		string CSVFormatString = "Resolution (px),Load time (s),Duration (s),FPS average (s),FPS min (s),FPS max (s)";
		string CSVResultString = string.Format("%1x%2,%3,%4,%5,%6,%7", sizeX, sizeY, g_Game.GetLoadTime() * 0.001, m_timeFromStart, m_MeasureFPS.ComputeAverage(), m_MeasureFPS.m_Min, m_MeasureFPS.m_Max);
		
		int count = MemoryStatsSnapshot.GetStatsCount();
		for (int i = 0; i < count; i++)
		{
			CSVFormatString += string.Format(",%1 average,%1 min,%1 max", MemoryStatsSnapshot.GetStatName(i));
			CSVResultString += string.Format(",%1,%2,%3", (int)m_MeasurementMemory[i].ComputeAverage(), (int)m_MeasurementMemory[i].m_Min, (int)m_MeasurementMemory[i].m_Max);
		}
		
		CSVFormatString += ",Timestamp";
		CSVResultString += string.Format(",%1", GetCurrentTimestamp());
		
		descrFile.WriteLine(CSVFormatString);
		descrFile.WriteLine(CSVResultString);
		
		descrFile.Close();
		Print("CSV file successfully saved into " + filename);
	}
	
	private string GetCurrentTimestamp()
	{
		int year, month, day;
		System.GetYearMonthDay(year, month, day);
		return string.Format("%1-%2-%3", year.ToString(4), month.ToString(2), day.ToString(2));
	}
	
	override void EOnFrame(IEntity owner, float timeSlice) //!EntityEvent.FRAME
	{	
		g_Game.GetInputManager().ActivateContext("BlockInputContext");
		
		//DO FPS
		int fps = System.GetFPS();
		m_FPSWidget.SetText("FPS " + fps);
		if (fps < m_FPSLimit)
			m_FPSWidget.SetColor(new Color(1.0, 0.0, 0.0, 1.0));
		else
			m_FPSWidget.SetColor(new Color(0.0, 1.0, 0.0, 1.0));
		
		vector destination;
		vector direction;

		vector output[2];
		UpdatePosition(timeSlice, output);
		
		destination = output[0];
		direction = output[1];

		m_camera.SetYawPitchRoll(direction);
		
		m_camera.SetOrigin(destination);		
		
		if (m_iCurrentPointsI > m_Points.Count())
		{
			FileIO.MakeDirectory(m_directory);
			string summeryFilename = string.Format("%1/%2", m_directory, "summary.txt");
			MakeSummeryFile(summeryFilename);
			string csvFilename = string.Format("%1/%2", m_directory, "summary.csv");
			MakeCSVFile(csvFilename);
			Print("Autotest Finished; result in " + m_directory);
			g_Game.RequestClose();
		}
		
		UpdateMeasurements();
		
		m_timeFromStart += timeSlice;
	}
	
	float dCc = 0;
	float prevQuatCc[4];
	float currentQuatCc[4];
	bool onceOnly = false;
	
	private void UpdatePosition(float timeSlice, vector output[2])
	{
		UpdateState();

		float quat[4];
		vector pos = ComputePosition();
		vector orientation = ComputeOrientation(quat);

		output[0] = pos;
		output[1] = orientation;
		
		if (onceOnly == false)
		{
			onceOnly = true;
			Math3D.QuatCopy(quat, prevQuatCc);
			Math3D.QuatCopy(quat, currentQuatCc);
		}
		
		UpdateTime(timeSlice);
		
		//handle camera rotation based on terrain inclination
		vector posNext = ComputePosition();
		
		vector dir = posNext - pos;
		float d = dir.NormalizeSize();

		vector mat[4];
		Math3D.DirectionAndUpMatrix(dir, vector.Up, mat);
		
		float quat2[4];
		Math3D.MatrixToQuat(mat, quat2);

		if (CompareQuat(currentQuatCc, quat2) == false)
		{
			float outQuat2[4];
			Math3D.QuatLerp(outQuat2, prevQuatCc, currentQuatCc, dCc);
			
			dCc = 0;
			Math3D.QuatCopy(outQuat2, prevQuatCc);
			Math3D.QuatCopy(quat2, currentQuatCc);
		}
		
		float outQuat[4];
		Math3D.QuatLerp(outQuat, prevQuatCc, currentQuatCc, dCc);
		
		output[1] = Math3D.QuatToAngles(outQuat);
		
		dCc += 0.75 * timeSlice;

		Math.Clamp(dCc, 0, 1);
	}
	
	//returns true if q1 "equals" q2
	private bool CompareQuat(float q1[4], float q2[4])
	{
		if (
			Math.AbsFloat(q1[0] - q2[0]) < 0.0001 &&
			Math.AbsFloat(q1[1] - q2[1]) < 0.0001 &&
			Math.AbsFloat(q1[2] - q2[2]) < 0.0001 &&
			Math.AbsFloat(q1[3] - q2[3]) < 0.0001
		)
		{
			return true;
		}
		
		return false;
	}
	
	private void UpdateState()
	{
		if (m_fTimeToTravel >= 0 && m_fTimeTraveled > m_fTimeToTravel)
		{
			m_iCurrentPointsI++;
		}
		
		if (m_iCurrentPointsI + 1 >= m_Points.Count())
		{
			return;
		}

		if (m_fTimeToTravel < 0 || m_fTimeTraveled >= m_fTimeToTravel)
		{
			vector start = m_Points[m_iCurrentPointsI];
			vector end = m_Points[m_iCurrentPointsI + 1];

			float distance = (end - start).Length();
	
			m_fTimeToTravel = distance / m_MovementSpeed;
			m_fTimeTraveled = 0;
			//Print("??? CHECKPOINT " + m_iCurrentPointsI + "/" + m_Points.Count());
		}
	}
	
	private void UpdateTime(float timeDelta)
	{
		m_fTimeTraveled += timeDelta;
	}
	
	private vector ComputePosition()
	{		
		if (m_iCurrentPointsI + 1 >= m_Points.Count())
		{
			//Print("!!! CHECKPOINT " + m_iCurrentPointsI + "/" + m_Points.Count());
			return m_Points[m_Points.Count() - 1];
		}
		
		vector start = m_Points[m_iCurrentPointsI];
		vector end = m_Points[m_iCurrentPointsI + 1];
		
		vector dir = (end - start);
		float t = m_fTimeTraveled / m_fTimeToTravel;
		
		vector ret = start + t * dir;
		ret[1] = GetYDistance(ret[0], ret[2]) + m_HeightAboveSurface;
		
		return ret;
	}
	
	private vector ComputeOrientation(out float quatOrientationOut[4])
	{
		vector start;
		vector end;

		if (m_iCurrentPointsI + 1 >= m_Points.Count())
		{
			start = m_Points[m_Points.Count() - 2];
			end = m_Points[m_Points.Count() - 1];
		}
		else
		{
			start = m_Points[m_iCurrentPointsI];
			end = m_Points[m_iCurrentPointsI + 1];
		}
		
		vector dir = (end - start);
		float t = m_fTimeTraveled / m_fTimeToTravel;
		vector currentPosition = start + t * dir;
		
		float d = dir.NormalizeSize();
		vector mat[4];
		Math3D.DirectionAndUpMatrix(dir, vector.Up, mat);				
		
		float quat[4];
		Math3D.MatrixToQuat(mat, quat);
		quatOrientationOut = quat;

		vector retDirAngles = Math3D.QuatToAngles(quat);

		float dCurrent = (end - currentPosition).Length();
		
		if (dCurrent < 0.1 * d && m_iCurrentPointsI < m_Points.Count() && m_iCurrentPointsI +2 < m_Points.Count())
		{			
			start = m_Points[m_iCurrentPointsI + 1];
			end = m_Points[m_iCurrentPointsI + 2];
			
			dir = (end - start);
			dir.Normalize();

			Math3D.DirectionAndUpMatrix(dir, vector.Up, mat);				
			
			float quat2[4];
			Math3D.MatrixToQuat(mat, quat2);
			quatOrientationOut = quat2;
			
			float ttt = (dCurrent / (0.1 * d));
			
			float outQuat[4];
			Math3D.QuatLerp(outQuat, quat, quat2, 1 - ttt);
			
			retDirAngles = Math3D.QuatToAngles(outQuat);
		}
		
		return retDirAngles;
	}
	
	private string QuatToString(float q[4])
	{
		return "(" + q[0] + "," + q[1] + "," + q[2] + "," + q[3] + ")";
	}
	
	private void Initialize(float scale, int sectorCount, vector rectangleMin, vector rectangleMax)
	{
		m_Points = new array<vector>();
		
		float xx = scale * rectangleMax[0] - rectangleMin[0];
		float zz = scale * rectangleMax[2] - rectangleMin[2];
				
		float shiftx = 0.5 * ((rectangleMax[0] - rectangleMin[0]) - xx);
		float shiftz = 0.5 * ((rectangleMax[2] - rectangleMin[2]) - zz);
		
		float zstep = zz;
		if (sectorCount > 1)
		{
			zstep = zz / (sectorCount - 1);
		}
		
		for (int i = 0; i < sectorCount; i++)
		{
			float z = i * zstep;
			if ((i % 2) == 0)
			{
				vector v = Vector(shiftx, 0, shiftz) + Vector(rectangleMin[0], 0.0, rectangleMin[2] + z);
				v[1] = GetYDistance(v[0], v[2]) + m_HeightAboveSurface;				
				m_Points.Insert(v);
				
				v = Vector(shiftx, 0, shiftz) + Vector(rectangleMin[0] + xx, 0.0, rectangleMin[2] + z);
				v[1] = GetYDistance(v[0], v[2]) + m_HeightAboveSurface;
				m_Points.Insert(v);
			}
			else
			{
				vector v = Vector(shiftx, 0, shiftz) + Vector(rectangleMin[0] + xx, 0.0, rectangleMin[2] + z);
				v[1] = GetYDistance(v[0], v[2]) + m_HeightAboveSurface;				
				m_Points.Insert(v);
				
				v = Vector(shiftx, 0, shiftz) + Vector(rectangleMin[0], 0.0, rectangleMin[2] + z);
				v[1] = GetYDistance(v[0], v[2]) + m_HeightAboveSurface;
				m_Points.Insert(v);
			}
		}
		
		if (m_ShowGeneratePath == true)
		{
			CreateLines(m_Points, ARGB(255, 255, 255, 255));
		}
	
		m_iCurrentPointsI = 0;
	}
	
	private float GetYDistance(float x, float z)
	{
		float wy = GetWorld().GetSurfaceY(x, z);
		float oh = 0;		
		if (GetWorld().IsOcean())
		{
			oh = GetWorld().GetOceanBaseHeight();
			oh = Math.Ceil(Math.AbsFloat(oh));
		}
		
		float hh = wy;
		if (oh > wy)
		{
			hh = oh;
		}
		
		return hh;
	}
	
	private void TransformCameraToWaypoint()
	{
		if (m_waypoint && m_camera)
		{			
			vector mat[4];
			m_waypoint.GetTransform(mat);
			m_camera.SetTransform(mat);
			m_waypoint.EOnEnter();
		}
	}
}