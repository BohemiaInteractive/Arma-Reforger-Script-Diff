[EntityEditorProps(category: "GameLib/Scripted/Autotest", description:"FPS_autotest", dynamicBox: true)]
class FPS_AutotestClass: GenericEntityClass
{
}

class FPS_Autotest: GenericEntity
{
	[Attribute("ScriptCamera1", UIWidgets.EditBox, "Name of camera entity", "")]
	private string m_cameraEntityName;
	
	[Attribute("", UIWidgets.EditBox, "Name of a page", "")]
	private string m_pageName;
	
	[Attribute("300", UIWidgets.Slider, "Measurements step", "10 2000 10")]
	private int m_step;

	[Attribute("1", UIWidgets.Slider, "Wait time", "0 20 0.1")]
	private float m_stepWaitTime;
	
	[Attribute("80", UIWidgets.Slider, "Camera height", "10 300 10")]
	private float m_cameraHeight;
	
	[Attribute("10", UIWidgets.Slider, "Number of slowest points for lowest FPS graph", "10 50 1")]
	private int m_lowFrameCount;
		
	////////////////////////////////////////////////////////////////////////////////////
	protected float m_timer; 			//< Used for counting how long camera stais on one place

	private IEntity m_camera;			//< Entity used as camera, this one is auto moved by autotest

	private int m_x = 0;				//< Next camera x position
	private int m_z = 0;				//< Next camera y position
	private int m_worldSize;			//< How big is world we are scanning
	
	private float m_avgFPS = -1;		//< Accumulated avarage FPS in current test
	private int m_avgFpsIdx = 0;		//< This is just persistent index of autotest for tracking long term changes in results

	protected ref MeasurementFile m_heatmapfile; //< Test heatmap output file
	protected ref MeasurementFile m_graphfile;	 //< Test long term graph output file
	
	private ref set<int> m_lowestFPSs;
	////////////////////////////////////////////////////////////////////////////////////
    
	
	protected ref AutotestRegister m_register;
	
	void FPS_Autotest(IEntitySource src, IEntity parent)
	{
		m_timer = 0.0;
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		m_lowestFPSs = new set<int>();
    }
	
	void ~FPS_Autotest()
	{

	}

	override void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		//! Find camera entity and move it to world origin m_cameraHeight above ground
		m_camera = g_Game.FindEntity(m_cameraEntityName);	
		if(m_camera)
		{
			float groundY = m_camera.GetWorld().GetSurfaceY(m_x, m_z);
			m_camera.SetOrigin(Vector(m_x, groundY + m_cameraHeight, m_z));
		}
		
		//! Detect world size
		if(g_Game.GetWorldEntity())
		{	
			vector min, max;
			g_Game.GetWorldEntity().GetWorldBounds(min, max);
			m_worldSize = Math.Max(max[0] - min[0], max[2] - min[2]);
		}
	}
    
	override void EOnFrame(IEntity owner, float timeSlice) //!EntityEvent.FRAME
	{
		g_Game.GetInputManager().ActivateContext("BlockInputContext");
		
		if(m_timer > m_stepWaitTime) //< When camera was still long enough
		{
			m_timer = 0;
			if(m_camera)
			{
				//! Move camera
				float groundY = m_camera.GetWorld().GetSurfaceY(m_x, m_z);
				m_camera.SetOrigin(Vector(m_x, groundY + m_cameraHeight, m_z));
			}
			
			
			//! Find world file path
			if(!m_register)
			{
				m_register = new AutotestRegister();
				m_register.Init(m_pageName);
			}

			////////////////////////////////////////////////////////////////////////////////////
			//! First simulation open result files and write test headers
			if(!m_heatmapfile)
			{
				ref MeasurementFile descrFile = m_register.OpenMeasurementFile("0_fps_test_descr", "", MeasurementType.HTML);
				descrFile.AddData(string.Format("<h1>Test description</h1><p>Camera step: %1m<br/>Step wait time: %2s<br/>Camera height above ground: %3m<br/>Number of worst FPS places: %4</p>", m_step, m_stepWaitTime, m_cameraHeight, m_lowFrameCount));
				
				m_heatmapfile = m_register.OpenMeasurementFile("1_fps_heatmap", "FPS heatmap", MeasurementType.HeatMap);
				
				ref MeasurementFile fileUploadDescr = m_register.OpenMeasurementFile("1_fps_heatmap_sourcefile", "Heatmap source data", MeasurementType.File);
				fileUploadDescr.AddData(m_heatmapfile.GetFilePath());
				
				m_register.UploadLogFileWithResults();
			}
			if(!m_graphfile)
			{							
				m_graphfile = m_register.OpenMeasurementFile("2_fps_graph", "FPS graph", MeasurementType.GraphLine, false);
				m_graphfile.SetGraphHeader("Measurement date, Avarage FPS");
			
				
				////////////////////////////////////////////////////////////////////////////////////
				//! Keep track of index of autotest run
				string idxLine = m_register.LoadPersistentData("2_fps_graph.idx");				
				if(idxLine.Length() == 0)
					m_avgFpsIdx = 0;
				else
					m_avgFpsIdx = idxLine.ToInt() + 1;
							
				m_register.SavePersistentData("2_fps_graph.idx", string.Format("%1", m_avgFpsIdx));
				////////////////////////////////////////////////////////////////////////////////////
			}
			////////////////////////////////////////////////////////////////////////////////////
					
			int fps = System.GetFPS();
			if (m_avgFPS == -1) //< First simulation
				m_avgFPS = fps;
			else
				m_avgFPS = (m_avgFPS + fps) / 2.0;
			
			m_heatmapfile.AddData(string.Format("%1,%2,%3", m_x, m_z, fps));  //< output new data to heatmap
	
			m_lowestFPSs.Insert(fps);
			if(m_lowestFPSs.Count() > m_lowFrameCount)
				m_lowestFPSs.Remove(m_lowestFPSs.Count() - 1);
			
			if (m_x <= m_worldSize) 
			{
				//! Prepare position for next camera move
				m_x = m_x + m_step;
			} 
			else 
			{
				m_x = 0;
				if (m_z <= m_worldSize) 
				{
					m_z = m_z + m_step;
				} 
				else 
				{
					//! End of measurement				
					string timeStamp = FormatTimestamp();
					m_graphfile.AddData(string.Format("%1,%2", timeStamp, m_avgFPS));  //< output avarage FPS in this test
					
					ref MeasurementFile lowFpsFile = m_register.OpenMeasurementFile("3_low_fps_graph", "Avg of worst FPS places", MeasurementType.GraphLine, false);
					lowFpsFile.SetGraphHeader("Measurement date, Avarage FPS");
					
					int lowestFpsSum = 0;
					for(int i = 0; i < m_lowestFPSs.Count(); ++i)
						lowestFpsSum += m_lowestFPSs[i];
					
					lowFpsFile.AddData(string.Format("%1,%2", timeStamp, lowestFpsSum / m_lowestFPSs.Count()));
					
					g_Game.RequestClose(); //< End mission
				}
			}
		}		
		m_timer += timeSlice;
	}

	string FormatTimestamp()
	{
		int year, month, day;
		System.GetYearMonthDay(year, month, day);
		string smonth, sday;
		if(month < 10)
			smonth = string.Format("0%1", month);
		else
			smonth = string.Format("%1", month);

		if(day < 10)
			sday = string.Format("0%1", day);
		else
			sday = string.Format("%1", day);

		return string.Format("%1%2%3-%4", year, smonth, sday, m_avgFpsIdx);
	}
}