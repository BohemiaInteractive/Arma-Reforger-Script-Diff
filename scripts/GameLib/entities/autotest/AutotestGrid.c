// Script File

[EntityEditorProps(category: "GameLib/Scripted/Autotest", description:"AutotestGrid", dynamicBox: true)]
class AutotestGridClass: GenericEntityClass
{
}

class SimpleTimeSignal
{
	private float m_LastTime;
	private float m_TimeInterval;
	
	void SimpleTimeSignal(float timeInterval)
	{
		m_LastTime = 0;
		m_TimeInterval = timeInterval;
	}
	
	bool Update(float currentTime)
	{
		if (currentTime - m_LastTime > m_TimeInterval)
		{
			m_LastTime = currentTime;
			
			return true;
		}
		
		return false;
	}
	
	void Restart(float currentTime)
	{
		m_LastTime = currentTime;
	}
	
	void RestartAndSignal()
	{
		m_LastTime = -m_TimeInterval;
	}
}

class AutotestGrid: GenericEntity
{
	[Attribute("Camera1", UIWidgets.EditBox, "Name of camera entity", "")]
	private string m_cameraEntityName; //< name of camera entity which will be used to take screenshots

	[Attribute("60", UIWidgets.Slider, "Which FPS is considered as minimal", "0 240 1")]
	private float m_FPSLimit; //< this time is waited for the screenshot to be captured and saved on disk
	
	[Attribute("1.8", UIWidgets.Slider, "Distance between surface and camera on y-axis", "0 20 0.1")]
	private float m_HeightAboveSurface;
	
	[Attribute("5", UIWidgets.Slider, "Time in seconds to make a 360 degree rotation, to load all of the resources", "1 100 1")]
	private float m_FirstRotationTime;

	[Attribute("5", UIWidgets.Slider, "Time in seconds to make a 360 degree rotation, to make the measurement", "1 100 1")]
	private float m_SecondRotationTime;

	[Attribute("1", UIWidgets.Slider, "Time in seconds to start the autotest, n seconds after the initialization of this object", "1 20 1")]
	private float m_StartAfterInitTime;

	[Attribute("10", UIWidgets.Slider, "Grid size on x axis", "1 100 1")]
	private float m_GridX;

	[Attribute("10", UIWidgets.Slider, "Grid size on z axis", "1 100 1")]
	private float m_GridZ;
		
	private int m_GridPosition;

	private IEntity m_camera;
	private string m_directory;
	
	private TextWidget m_FPSWidget;
	
	protected float m_timeFromStart;
	protected float m_timeLastStatsSnapshot;

	private ref SimpleTimeSignal m_StartTtimeSignal;	
	private ref SimpleTimeSignal m_FirstRotationSignal;
	private ref SimpleTimeSignal m_SecondRotationSignal;
	
	//-1 - init - wait for init duration to pass
	//0 - ready for 1st rotation
	//1 - doing 1st rotation
	//2 - ready for 2nd rotation
	//3 - doing 2nd rotation
	private int m_CurrentRotation;
	
	private ref Measurement m_MeasureFPS;
	private ref Measurement m_MeasureMemory;
	
	private ref array<ref Measurement> m_MeasurementMemory;
	
	[Attribute("MotionZone", UIWidgets.EditBox, "Motion Zone Object Name", "")]
	private string m_MotionZone;
	
	private MotionZone m_MotionZoneInstance;
	
	private float m_CurrentRotationT;
	private float m_CurrentDuration;
	
	private ref AutotestBase m_AutotestBase;
	
	void AutotestGrid(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		m_MeasureFPS = new Measurement();
		m_MeasureMemory = new Measurement();
		m_MeasurementMemory = new array<ref Measurement>();
		int mc = MemoryStatsSnapshot.GetStatsCount();
		for (int i = 0; i < mc; i++)
		{
			m_MeasurementMemory.Insert(new Measurement());
		}
		
		m_GridPosition = 0;
		m_timeFromStart = 0;
		m_timeLastStatsSnapshot = 0;
		m_CurrentRotation = -1;
		m_CurrentRotationT = 0;
		
		m_StartTtimeSignal = new SimpleTimeSignal(m_StartAfterInitTime);		
		m_FirstRotationSignal = new SimpleTimeSignal(m_FirstRotationTime);
		m_SecondRotationSignal = new SimpleTimeSignal(m_SecondRotationTime);

		System.GetCLIParam("autotest-output-dir", m_directory);

		if (m_directory.Length() == 0)
		{
			m_directory = "$logs:" + GetName();
		}
		
		m_AutotestBase = new AutotestBase();
		AutotestBase.SetLastProcessingTime_Treshold(100/1000);
	}

	void ~AutotestGrid()
	{				
	}
	
	vector GridPositionToWorldPosition(int gridPosition)
	{
		float dX = m_MotionZoneInstance.GetMax()[0] - m_MotionZoneInstance.GetMin()[0];
		float dZ = m_MotionZoneInstance.GetMax()[2] - m_MotionZoneInstance.GetMin()[2];
		
		float nX = dX / m_GridX;
		float nZ = dZ / m_GridZ;
		
		int t = Math.AbsInt(gridPosition / m_GridX);
		int cX = t;
		int cZ = gridPosition - t * m_GridX;
		
		vector ret = Vector(m_MotionZoneInstance.GetMin()[0] + nX * cX + 0.5 * nX, 0, m_MotionZoneInstance.GetMin()[2] + nZ * cZ + 0.5 * nZ);
		ret[1] = m_HeightAboveSurface + GetYDistance(ret[0], ret[2]);
		
		return ret;
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
	
	override void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		m_camera = g_Game.FindEntity(m_cameraEntityName);

		if (!m_camera)
		{
			Print("Camera not found 2!", LogLevel.ERROR);
		}
		else
		{
			m_camera.SetOrigin(Vector(1269.97, 32.95, 1156.55));
		}
		
		m_MotionZoneInstance = MotionZone.Cast(g_Game.FindEntity(m_MotionZone));
		
		if (m_MotionZoneInstance != null)
		{
			m_MotionZoneInstance.Initialize();
		}
	
		m_FPSWidget = TextWidget.Cast(g_Game.GetWorkspace().CreateWidgetInWorkspace(WidgetType.TextWidgetTypeID, 16, 16, 512, 128, WidgetFlags.VISIBLE, new Color(0.0, 0.0, 0.0, 1.0), 1024));
		m_FPSWidget.SetExactFontSize(64);
		
/*		
		for (int i = 0; i < 100; i++)
		{
			vector v = GridPositionToWorldPosition(i);
			Print("" + i  + " | " + v);
		}
*/
	}
	
	private void MakeSummeryFile(string filename)
	{
		FileHandle descrFile = FileIO.OpenFile(filename, FileMode.WRITE);

		if(descrFile)
		{
			int sizeX = g_Game.GetWorkspace().GetWidth();
			int sizeY = g_Game.GetWorkspace().GetHeight();
			descrFile.WriteLine(string.Format("GRID AUTO TEST"));
			descrFile.WriteLine(string.Format("Resolution: %1 x %2 px", sizeX, sizeY));
#ifdef WORKBENCH
			descrFile.WriteLine(string.Format("Entering playmode time: %1 s", g_Game.GetLoadTime() / 1000));
#else
			descrFile.WriteLine(string.Format("Load time: %1 s", g_Game.GetLoadTime() / 1000));
#endif
			descrFile.WriteLine(string.Format("Duration: %1 s", m_timeFromStart));
			
			descrFile.WriteLine(string.Format("FPS average (s): %1", m_MeasureFPS.ComputeAverage()));
			descrFile.WriteLine(string.Format("FPS min (s): %1", m_MeasureFPS.m_Min));
			descrFile.WriteLine(string.Format("FPS max (s): %1", m_MeasureFPS.m_Max));
			
			int count = MemoryStatsSnapshot.GetStatsCount();
			for (int i = 0; i < count; i++)
			{
				string name = MemoryStatsSnapshot.GetStatName(i);
				descrFile.WriteLine(string.Format("Measurement '%1' average: %2", name, (int)m_MeasurementMemory[i].ComputeAverage()));
				descrFile.WriteLine(string.Format("Measurement '%1' min: %2", name, (int)m_MeasurementMemory[i].m_Min));
				descrFile.WriteLine(string.Format("Measurement '%1' max: %2", name, (int)m_MeasurementMemory[i].m_Max));
			}
			
			descrFile.Close();
			Print("Summary file successfully saved into " + filename);
		}
	}
	
	override void EOnFrame(IEntity owner, float timeSlice) //!EntityEvent.FRAME
	{	

		//DO FPS
		int fps = System.GetFPS();
		m_FPSWidget.SetText("FPS " + fps);
		if (fps < m_FPSLimit)
			m_FPSWidget.SetColor(new Color(1.0, 0.0, 0.0, 1.0));
		else
			m_FPSWidget.SetColor(new Color(0.0, 1.0, 0.0, 1.0));
		
		if (m_CurrentRotation >= 0 && !g_Game.IsPreloadFinished())
			return;
		
		vector destination;
		vector direction;

		vector output[2];
		
		destination = output[0];
		direction = output[1];
		
		bool signal0 = m_StartTtimeSignal.Update(m_timeFromStart);
		bool signal1 = m_FirstRotationSignal.Update(m_timeFromStart);
		bool signal2 = m_SecondRotationSignal.Update(m_timeFromStart);
		
		if (signal0 && m_CurrentRotation == -1)
		{
			m_CurrentRotation = 0;
			m_FirstRotationSignal.RestartAndSignal();
			
			vector p = GridPositionToWorldPosition(m_GridPosition);
			m_camera.SetOrigin(p);	
			g_Game.BeginPreload(GetWorld(), p, 500);
			Print("-- init time done");
		}
		else
		if (signal1 && m_CurrentRotation == 0)
		{
			//start 1st rotation
			m_CurrentRotation = 1;
			m_CurrentRotationT = 0;
			m_CurrentDuration = m_FirstRotationTime;
			Print("-- start 1st rotation");
		}
		else
		if (signal1 && m_CurrentRotation == 1)
		{
			//end 1st rotation
			m_CurrentRotation = 2;
			m_CurrentDuration = 1;
			Print("-- end 1st rotation");
			m_SecondRotationSignal.RestartAndSignal();
		}
		else
		if (signal2 && m_CurrentRotation == 2)
		{
			//start 2nd rotation
			m_CurrentRotation = 3;
			m_CurrentRotationT = 0;
			m_CurrentDuration = m_SecondRotationTime;
			Print("-- start 2nd rotation");
		}
		else
		if (signal2 && m_CurrentRotation == 3)
		{
			m_GridPosition++;
			if (m_GridPosition >= m_GridX * m_GridZ)
			{
				Print("-- END of test");
				string summeryFilename = string.Format("%1/%2", m_directory, "summary_grid.txt");
				Print("Autotest Finished; result in " + summeryFilename);
				FileIO.MakeDirectory(m_directory);
				MakeSummeryFile(summeryFilename);
				g_Game.RequestClose();
			}
			else
			{
				m_CurrentRotation = 0;
				m_CurrentDuration = 1;

				vector p = GridPositionToWorldPosition(m_GridPosition);
				m_camera.SetOrigin(p);	
				g_Game.BeginPreload(GetWorld(), p, 500);
				
				m_FirstRotationSignal.RestartAndSignal();

				//end 2nd rotation a move to next grid point
				Print("-- end 2nd rotation a move to next grid point");
			}
		}		
		
		if (m_CurrentRotation > 0)
		{
			RotateCamera(timeSlice, m_CurrentDuration);
		}
		
		if (m_CurrentRotation == 3)
		{
			UpdateMeasurements();			
		}
		
		if (AutotestBase.IsLastProcessingTime_Signal())
		{
			//print into log
			Print("!!! Warning Frame update too long");
			
			AutotestBase.ResetLastProcessingTime_Signal();
		}
		
		m_timeFromStart += timeSlice;
	}
	
	private void RotateCamera(float timeSlice, float duration)
	{
		vector matF[4];
		vector matB[4];
			
		float d = duration / 4;
	
		if (m_CurrentRotationT <= 1 * d)
		{
			Math3D.DirectionAndUpMatrix(vector.Forward, vector.Up, matF);
			Math3D.DirectionAndUpMatrix(vector.Right, vector.Up, matB);			
		}
		else
		if (m_CurrentRotationT <= 2 * d)
		{
			Math3D.DirectionAndUpMatrix(vector.Right, vector.Up, matF);
			Math3D.DirectionAndUpMatrix(-vector.Forward, vector.Up, matB);			
		}
		else
		if (m_CurrentRotationT <= 3 * d)
		{
			Math3D.DirectionAndUpMatrix(-vector.Forward, vector.Up, matF);
			Math3D.DirectionAndUpMatrix(-vector.Right, vector.Up, matB);			
		}
		else
		{
			Math3D.DirectionAndUpMatrix(-vector.Right, vector.Up, matF);
			Math3D.DirectionAndUpMatrix(vector.Forward, vector.Up, matB);
		}
		
		float quatF[4];
		Math3D.MatrixToQuat(matF, quatF);
		
		float quatB[4];
		Math3D.MatrixToQuat(matB, quatB);
	
		float outQuat[4];
		
		float t = m_CurrentRotationT;
		if (m_CurrentRotationT >= 3.0 * d)
		{
			t = (t - 3.0 * d) / d;
		}
		else
		if (m_CurrentRotationT >= 2.0 * d)
		{
			t = (t - 2.0 * d) / d;
		}
		else
		if (m_CurrentRotationT >= 1.0 * d)
		{
			t = (t - 1.0 * d) / d;
		}
		else
		{
			t = t / d;
		}
		
		Math3D.QuatLerp(outQuat, quatF, quatB, t);
		
		m_CurrentRotationT += timeSlice;
		
		vector angles = Math3D.QuatToAngles(outQuat);
		
		m_camera.SetYawPitchRoll(angles);
	}
		
	private string QuatToString(float q[4])
	{
		return "(" + q[0] + "," + q[1] + "," + q[2] + "," + q[3] + ")";
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
	
}