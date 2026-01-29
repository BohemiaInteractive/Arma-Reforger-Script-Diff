// TODO(koudelkaluk): clean up this mess, get rid of unnecessary files saved for Confluence upload
[EntityEditorProps(category: "GameScripted/Utility", description: "Modified Heatmap autotest", dynamicBox: true)]
class SCR_HeatMap_AutotestClass: GenericEntityClass
{
};

class HeatmapData
{
	float m_fps;
	float m_fps_n;	
	float m_fps_e;	
	float m_fps_s;	
	float m_fps_w;
	
	float m_x;
	float m_y;
	float m_z;	
	
	/*
	void HeatmapData(float fps, float x, float y, float z)
	{
		m_fps = fps;
		m_x = x;
		m_y = y;
		m_z = z;
	}
	*/
	
	void HeatmapData(float fps_avg, float fps_n, float fps_e, float fps_s, float fps_w, float x, float y, float z)
	{
		m_fps = fps_avg;
		m_fps_n = fps_n;
	 	m_fps_e = fps_e;
	 	m_fps_s = fps_s;
		m_fps_w = fps_w;
		m_x = x;
		m_y = y;
		m_z = z;
	}
};

class SCR_HeatMap_Autotest: GenericEntity
{	
	[Attribute("ScriptCamera1", UIWidgets.EditBox, "Name of camera entity", "")]
	private string m_cameraEntityName;

	[Attribute("", UIWidgets.EditBox, "Name of Confluence page", "")]
	private string m_pageName;

	[Attribute("Unnamed test", UIWidgets.EditBox, "Title of particular test", "")]
	private string m_testName;

	[Attribute("Heatmap title", UIWidgets.EditBox, "Title of heatmap", "")]
	private string m_heatmapName;

	[Attribute("300", UIWidgets.Slider, "Measurements step", "10 2000 10")]
	private int m_step;

	[Attribute("1", UIWidgets.Slider, "Wait time", "0 20 0.1")]
	private float m_stepWaitTime;

	[Attribute("80", UIWidgets.Slider, "Camera height", "1 300 10")]
	private float m_cameraHeight;

	[Attribute("10", UIWidgets.Slider, "Number of slowest points for lowest FPS graph", "10 50 1")]
	private int m_lowFrameCount;

	[Attribute("", UIWidgets.EditBox)]
	private string mapName;

	
	[Attribute("false", UIWidgets.CheckBox, "Specify own world bounds if needed.")]
	private bool OverwriteWorldBounds;
	
	[Attribute("0", UIWidgets.Auto)]
	private int WorldStartX;

	[Attribute("0", UIWidgets.Auto)]
	private int WorldStartZ;

	[Attribute("0", UIWidgets.Auto)]
	private int WorldEndX;

	[Attribute("0", UIWidgets.Auto)]
	private int WorldEndZ;
	
	
	[Attribute("0", UIWidgets.Auto)]
	private int startX;

	[Attribute("0", UIWidgets.Auto)]
	private int startZ;

	[Attribute("0", UIWidgets.Auto)]
	private int endX;

	[Attribute("0", UIWidgets.Auto)]
	private int endZ;

	[Attribute("120", UIWidgets.EditBox)]
	private int fpsRange;

	[Attribute("14", UIWidgets.EditBox)]
	private float timeOfTheDay;
	
	[Attribute("", UIWidgets.Auto)]
	private string mapID;
	
	[Attribute("false", UIWidgets.CheckBox, "Skips actual measuring and only feed the json with fake data (meant for testing).")]
	private bool SimulateHttpPost;
	
	[Attribute("true", UIWidgets.CheckBox, "Send collected json data to WebApp via HttpPost.")]
	private bool SendHttpPost;
	
	private ref map<EPlatform, string> platforms = new map<EPlatform, string>();
	
	private ref RestCallback m_jsonCallback = new RestCallback();
	
	private const string HEATMAP_URL = "https://ar-heatmap.bistudio.com/";
	
	protected bool m_bDoneGenerating = false;	//< Used to stop the benchmark after processing and sending data
	protected bool m_bDoneGeneratingMessage = false;	//< true if message about generating done was already sent
	
	protected float m_timer; 			//< Used for counting how long camera stays on one place

	private IEntity m_camera;			//< Entity used as camera, this one is auto moved by autotest

	private int m_x = startX;			//< Next camera x position
	private int m_y;					//< Camera y position
	private int m_z = startZ;			//< Next camera y position
	
	private int endX_trimed = endX;
	private int endZ_trimed = endZ;

	private int m_worldSize;			//< How big is world we are scanning
	private int m_worldSizeX;
	private int m_worldSizeZ;
	private vector m_minBounds, m_maxBounds;

	protected ref MeasurementFile m_heatmapfile; //< Heatmap output data

	private ref array<ref HeatmapData> m_geojson_data = new array<ref HeatmapData>();
	private ref array<int> image_file = new array<int>();

	private float min_fps = 1000;
	private float max_fps = 0;
	private float avg_fps = 0;

	protected ref AutotestRegister m_register;

	private bool area_only; // do we want to measure only specified area?

	private float fps_four_directions[4];
	private int prev_dir = -1;

	private int directions[4] = {
		0,
		90,
		180,
		270
	};
	
	private int colors[6] = {
		0x0000ff,
		0x00ffff,
		0x00ff00,
		0xffff00,
		0xff0000,
		0xff0000
	};

	void SCR_HeatMap_Autotest(IEntitySource src, IEntity parent)
	{
		m_timer = -5;
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
    }
	
	void PrintToConsole(string text = "", LogLevel logLevel = LogLevel.NORMAL)
	{
		Print("[FPS_Autotest] " + text, logLevel);
	}
	
	override void EOnInit(IEntity owner)
	{		
		if(SCR_Global.IsEditMode())
			return;
		ChimeraWorld world = GetGame().GetWorld();
		
		if (world && world.GetTimeAndWeatherManager())
		{
			world.GetTimeAndWeatherManager().SetTimeOfTheDay(timeOfTheDay);
			world.GetTimeAndWeatherManager().SetIsDayAutoAdvanced(false);
		}
		

		//area_only = (endX > 0 || endZ > 0 || startX > 0 || startZ > 0 || !(startX > endX) || !(startZ > endZ));
		area_only = (!(startX > endX) || !(startZ > endZ) || !((endX - startX) >= m_step) || !((endZ - startZ) >= m_step));
		//! Find camera entity and move it to world origin m_cameraHeight above ground
		m_camera = GetGame().FindEntity(m_cameraEntityName);
		

		if (m_camera)
		{
			float groundY = m_camera.GetWorld().GetSurfaceY(m_x, m_z);
			m_y = groundY + m_cameraHeight;
			m_camera.SetOrigin(Vector(m_x, m_y, m_z));
			
			vector new_angles = m_camera.GetAngles(); // rotate the camera in 4 directions
			new_angles[1] = 0;
			m_camera.SetAngles(new_angles);
		}
		else
		{
			PrintToConsole("Camera not found.", LogLevel.ERROR);
			m_bDoneGenerating = true;
			return;
		}

		if(OverwriteWorldBounds)
		{
			m_minBounds[0] = WorldStartX;
			m_minBounds[2] = WorldStartZ;
			m_maxBounds[0] = WorldEndX;
			m_maxBounds[2] = WorldEndZ;
		}
		
		//! Detect world size
		if (GetGame().GetWorldEntity())
		{
			if (!OverwriteWorldBounds)
				GetGame().GetWorldEntity().GetWorldBounds(m_minBounds, m_maxBounds);
			
			/*
			vector m_minBounds2, m_maxBounds2;
			GetGame().GetWorldEntity().GetWorldBounds(m_minBounds, m_maxBounds);
			GetGame().GetWorldEntity().GetBounds(m_minBounds2, m_maxBounds2);
			PrintFormat("GetWorldBounds: [%1, %2] [%3, %4]", m_minBounds[0], m_minBounds[2], m_maxBounds[0], m_maxBounds[2]);
			PrintFormat("GetBounds: [%1, %2] [%3, %4]", m_minBounds2[0], m_minBounds2[2], m_maxBounds2[0], m_maxBounds2[2]);
			*/	
			
			m_worldSize = Math.Max(m_maxBounds[0] - m_minBounds[0], m_maxBounds[2] - m_minBounds[2]);

			if (area_only)
			{
				m_worldSizeX = endX;
				m_worldSizeZ = endZ;
			}
			else 
			{
				m_worldSizeX = m_worldSize;
				m_worldSizeZ = m_worldSize;

				endX = m_worldSizeX;
				endZ = m_worldSizeZ;
			}
		}
		else
		{
			PrintToConsole("Cannot get the world entity.", LogLevel.ERROR);
			m_bDoneGenerating = true;
			return;			
		}
		
		// Round the end so it is in m_step multiple.
		endX_trimed = startX + m_step * ((endX - startX) / m_step);
		endZ_trimed = startZ + m_step * ((endZ - startZ) / m_step);
		
		platforms.Insert(EPlatform.WINDOWS, "62e0e57519098fc05e1b28fe");
		platforms.Insert(EPlatform.XBOX_ONE, "62e0e58f19098fc05e1b28ff");
		platforms.Insert(EPlatform.XBOX_SERIES_X, "632863b8211b59093a5afb79");
		platforms.Insert(EPlatform.XBOX_SERIES_S, "632863cb211b59093a5afb7a");
		platforms.Insert(EPlatform.PS5, "66cdc91e0a90565192210ae0");
		
		m_timer = 0;
		
		// Set example callbacks
		m_jsonCallback.SetOnSuccess(OnSendDataSuccess);
		m_jsonCallback.SetOnError(OnSendDataSuccess);
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{		
		m_timer += timeSlice;
		if (m_timer < 0)
			return;
		
		if (m_bDoneGenerating)
		{
			if(!m_bDoneGeneratingMessage)
			{
				m_bDoneGeneratingMessage = true;
				PrintToConsole("Measuring step is done or was skipped.");
			}
			if(!SendHttpPost)
				GetGame().RequestClose();
			return;
		}
		
		// If simulating, then the whole measuring is skipped and fake data are generated
		if (SimulateHttpPost)
		{
			PrintToConsole("Simulating the measuring step by feeding it random data for.");
			max_fps = 0;
			min_fps = 0;
			int stepsX = ((endX_trimed-startX)/m_step)+1;
			int stepsZ = ((endZ_trimed-startZ)/m_step)+1;
			for(int i = 0; i < stepsX; i++)
			{					
				for(int j = 0; j < stepsZ; j++)
				{						
					int fps_avg = 0;
					for(int k = 0; k < 4; k++)
					{
						fps_four_directions[k] = Math.RandomInt(60,121);
						fps_avg += fps_four_directions[k];
					}
					fps_avg = fps_avg / 4;
					HeatmapData new_geojson_value = new HeatmapData(fps_avg, fps_four_directions[0], fps_four_directions[1], fps_four_directions[2], fps_four_directions[3],
																	startX+i*m_step, 0, startZ+j*m_step);					
					m_geojson_data.Insert(new_geojson_value);
				}
			}
			GenerateGeoJSON();
			m_bDoneGenerating = true;
			return;
		}

		if (!m_camera)
		{
			PrintToConsole("Camera entity cannot be found!", LogLevel.ERROR);
			GetGame().RequestClose();
			return;
		}

		int cur_fps = System.GetFPS();
		int cur_dir = ((m_timer / m_stepWaitTime - Math.Floor(m_timer / m_stepWaitTime)) * 4);

		if (cur_dir != prev_dir)
		{
			vector new_angles = m_camera.GetAngles(); // rotate the camera in 4 directions
			new_angles[1] = directions[cur_dir];//new_angles[1] + 90;
			m_camera.SetAngles(new_angles);
			prev_dir = cur_dir;
		}

		fps_four_directions[cur_dir] = cur_fps;

		if (m_timer > m_stepWaitTime) //< When camera was still long enough
		{
			m_timer -= m_stepWaitTime;

			if (!m_register)
			{
				m_register = new AutotestRegister();
				m_register.Init(m_pageName);
			}

			if (!m_heatmapfile)
			{
				PrintToConsole("Registering measurement file (HTML): 0_fps_test_descr");
				ref MeasurementFile descrFile = m_register.OpenMeasurementFile("0_fps_test_descr", "", MeasurementType.HTML);
				if(!descrFile || !descrFile.IsValid())
				{
					PrintToConsole("Cannot create measurement file (HTML).", LogLevel.ERROR);
					m_bDoneGenerating = true;
					return;
				}
				descrFile.AddData(string.Format("<h1>" + m_heatmapName + "</h1><p>Camera step: %1m<br/>Step wait time: %2s<br/>Camera height above ground: %3m<br/>Number of worst FPS places: %4</p>", m_step, m_stepWaitTime, m_cameraHeight, m_lowFrameCount));
				
				PrintToConsole("Registering measurement file (HeatMap): 1_fps_heatmap");
				m_heatmapfile = m_register.OpenMeasurementFile("1_fps_heatmap", m_testName, MeasurementType.HeatMap);
				if(!m_heatmapfile || !m_heatmapfile.IsValid())
				{
					PrintToConsole("Cannot create measurement file (HeatMap).", LogLevel.ERROR);
					m_bDoneGenerating = true;
					return;
				}
			}

			int fps_avg = (fps_four_directions[0] + fps_four_directions[1] + fps_four_directions[2] + fps_four_directions[3]) / 4;

			image_file.Insert(fps_avg);
			m_heatmapfile.AddData(string.Format("%1,%2,%3", m_x, m_z, fps_avg));  //< output new data to heatmap
			HeatmapData new_geojson_value = new HeatmapData(fps_avg, fps_four_directions[0], fps_four_directions[1], fps_four_directions[2], fps_four_directions[3], m_x, m_y, m_z);
			m_geojson_data.Insert(new_geojson_value);
			
			if (m_x + m_step <= m_worldSizeX)
			{
				if (fps_avg > max_fps)
					max_fps = fps_avg;
				if (fps_avg < min_fps)
					min_fps = fps_avg;
				//! Prepare position for next camera move
				m_x = m_x + m_step;
			}
			else
			{
				if (m_z + m_step <= m_worldSizeZ)
				{
					m_x = startX;
					m_z = m_z + m_step;
				}
				else
				{
					PrintToConsole("All points measured.");
					CalculateAvgFPS();
					GenerateDDS();
					GenerateGeoJSON();
					GenerateCSV();
					m_bDoneGenerating = true;
				}
			}
			
			if (m_camera)
			{
				//! Move camera
				float groundY = m_camera.GetWorld().GetSurfaceY(m_x, m_z);
				m_y = groundY + m_cameraHeight;
				m_camera.SetOrigin(Vector(m_x, m_y, m_z));
			}
		}
	}

	private void CalculateAvgFPS()
	{
		int measured_points = m_geojson_data.Count();
		for (int i = 0; i < measured_points; ++i)
		{
			avg_fps += m_geojson_data[i].m_fps / measured_points;
		}

		if (measured_points == 0)
			return;

		//avg_fps /= m_geojson_data.Count()*4;
		PrintToConsole("Avg FPS: " + avg_fps);
	}

	private void ReverseArray(inout array<int> arr)
	{
		array<int> temp = new array<int>();
		for (int i = arr.Count()-1; i >= 0; --i)
		{
			temp.Insert(arr[i]);
		}
		arr = temp;
	}

	private void GenerateDDS()
	{
#ifdef WORKBENCH
		PrintToConsole("Generating DDS file.");
		int width = (endX_trimed - startX) / m_step + 1;//2;
		int height = (endZ_trimed - startZ) / m_step;//2;

		int offset_x = startX / m_step;
		int offset_z = startZ / m_step;

		int image_size = m_worldSize / m_step;

		int id = 0;
		int id_z = 0;

		array <int> image = new array <int>;

		for (int i = 0; i < image_size * image_size; ++i)
		{
			image.Insert(ARGB(255, 0, 0, 0));
		}

		for (int y = 0; y < height; ++y)
		{
			array <int> row = new array <int>;
			for (int i = 0; i < image_size; ++i)
			{
				row.Insert(ARGB(255, 0, 0, 0));
			}

			int id_x = 0;

			for (int x = 0; x < width; ++x)
			{
				float range;
				int col;

				int fps = image_file[id];

				float level = Math.Clamp(fps / fpsRange, 0, 1);
				int phase = Math.Floor(level / 0.25);

				range = Math.Clamp((level - phase * 0.25) / 0.25, 0, 1);

				int from = colors[phase];
				int to = colors[phase+1];

				Color col1 = Color.FromInt(from);
				Color col2 = Color.FromInt(to);

				col1.Lerp(col2, range);
				col = col1.PackToInt();

				int index = offset_x + x;

				if (index < row.Count())
					row[index] = col;

				id_x++;
				id++;
			}

			ReverseArray(row);
			for (int i = 0; i < row.Count(); ++i)
			{
				int index = (offset_z + id_z) * image_size + i;
				if (index < image.Count())
					image[index] = row[i];
			}
			id_z++;
		}

		ReverseArray(image);
		const string ddsFile = "$logs:heatmap.dds";
		if(!TexTools.SaveImageData(ddsFile, image_size, image_size, image))
		{
			PrintToConsole("Failed to save the data into the file: " + ddsFile, LogLevel.ERROR);
			return;
		}
		PrintToConsole("Data sucessfully saved into dds file: " + ddsFile);
#endif
	}

	private string GetGeoJSONLine(HeatmapData val) //vector val)
	{
		/*
		string new_value = string.Format(
			"{ \"properties\": { \"FPS\": \"%1\" }, \"geometry\": { \"coordinates\": [ %2, %3, %4] } }",
			Math.Floor(val.m_fps), val.m_x, val.m_z, val.m_y);
		*/
		// TODO use this when WebApp is ready
		
		string new_value = string.Format(
			"{  \"type\": \"Point\", \"properties\": { \"FPS_AVG\": \"%1\", \"FPS_N\": \"%2\", \"FPS_E\": \"%3\", \"FPS_S\": \"%4\", \"FPS_W\": \"%5\" }, \"geometry\": { \"coordinates\": [ %6, %7, %8] } }",
			Math.Floor(val.m_fps), Math.Floor(val.m_fps_n), Math.Floor(val.m_fps_e), Math.Floor(val.m_fps_s), Math.Floor(val.m_fps_w),
			val.m_x, val.m_z, val.m_y);
		
		return new_value;
	}

	private string GetCSVLine(HeatmapData val) //vector val)
	{
		string new_value = string.Format(
			"%1,%2,%3",
			val.m_x, val.m_z, Math.Floor(val.m_fps) );

		return new_value;
	}

	private void GenerateGeoJSON()
	{
		FileHandle file;
		
		PrintToConsole("Creating the GeoJSON file.");
		
		string directoryName = "$logs:/" + mapName;
		if(!FileIO.MakeDirectory(directoryName))
		{
			PrintToConsole("Failed to make the directory \"" + directoryName + "\".", LogLevel.ERROR);
			PrintToConsole("Failed to generate the GeoJSON file.");
			return;
		}
		string fileName = FormatTimestamp() + ".geojson";
		string filePath = directoryName + "/" + fileName;
		file = FileIO.OpenFile(filePath, FileMode.WRITE);
		if(!file || !file.IsOpen())
		{
			PrintToConsole("Failed to create/open the file \"" + filePath + "\". Cannot continue with generating and sending the GeoJSON file.", LogLevel.ERROR);
			return;
		}

		WriteDataGJSON(file);
		file.Close();		
		file = FileIO.OpenFile(filePath, FileMode.READ);
		if(!file || !file.IsOpen())
		{
			PrintToConsole("Failed to open the file \"" + filePath + "\". Cannot continue with sending the GeoJSON file.", LogLevel.ERROR);
			return;
		}
		
		string data;
		file.Read(data, file.GetLength());
		file.Close();
		
		RestContext restContext = GetGame().GetRestApi().GetContext(HEATMAP_URL);
		
		if (!restContext)
		{
			PrintToConsole("Failed to get RestAPI context. Unable to send data", LogLevel.ERROR);
			return;
		}
		
		PrintToConsole("Attempting to POST json data");
		restContext.SetHeaders("Content-Type,application/json");
		int postReturnVal;
		if(SendHttpPost)
			postReturnVal = restContext.POST(m_jsonCallback, "api/fps-statistics", data);
		PrintToConsole("Returned value of the POST: " + postReturnVal.ToString());
		
		restContext = null;
		
		PrintToConsole("Creating the GeoJSON done.");
	}

	private void GenerateCSV()
	{
		FileHandle file;
		
		PrintToConsole("Creating a CSV file");
		
		string directoryName = "$logs:/" + mapName;
		if(!FileIO.MakeDirectory(directoryName))
		{
			PrintToConsole("Failed to make the directory \"" + directoryName + "\".", LogLevel.ERROR);
			PrintToConsole("Failed to generate the CSV file.");
			return;
		}		
		string fileName = FormatTimestamp() + ".csv";
		string filePath = directoryName + "/" + fileName;
		file = FileIO.OpenFile(directoryName, FileMode.WRITE);
		if(!file || !file.IsOpen())
		{
			PrintToConsole("Failed to create/open the file \"" + filePath + "\". Cannot continue with generating the CSV file.", LogLevel.ERROR);
			return;
		}		
		WriteDataCSV(file);
		file.Close();
		
		PrintToConsole("Creating a CSV done");
	}

	private void WriteDataGJSON(FileHandle file)
	{
		if(!file || !file.IsOpen())
		{
			PrintToConsole("No file to write GeoJson data to found.", LogLevel.ERROR);
			return;
		}
		
		PrintToConsole("Adding data to geojson");
			
		string platform;
		platforms.Find(System.GetPlatform(), platform);
		
		if (!platform)
		{
			platform = "Invalid";
		
			PrintToConsole("SCR_FPS_Autotest::Invalid platform, GJSON file will fail to post to database", LogLevel.ERROR);
		}
		
		file.WriteLine("{");
		file.WriteLine(string.Format("\"date\": \"%1\",", SCR_DateTimeHelper.GetDateTimeLocal()));
		file.WriteLine(string.Format("\"platform\": \"%1\",", platform));
		file.WriteLine(string.Format("\"map\": \"%1\",", mapID));
		file.WriteLine(string.Format("\"heatmapName\": \"%1\",", m_heatmapName));
		file.WriteLine("\"dimension\": {");
		file.WriteLine(string.Format("  \"startX\": %1,", m_minBounds[0]));
		file.WriteLine(string.Format("  \"startZ\": %1,", m_minBounds[2]));
		file.WriteLine(string.Format("  \"endX\": %1,", m_maxBounds[0]));
		file.WriteLine(string.Format("  \"endZ\": %1,", m_maxBounds[2]));
		//file.WriteLine(string.Format("  \"startX\": %1,", startX));
		//file.WriteLine(string.Format("  \"startZ\": %1,", startZ));
		//file.WriteLine(string.Format("  \"endX\": %1,", endX_trimed));
		//file.WriteLine(string.Format("  \"endZ\": %1,", endZ_trimed));
		file.WriteLine(string.Format("  \"stepSize\": %1},", m_step));
		file.WriteLine("\"featurecollection\": {");
		file.WriteLine("\"type\": \"FeatureCollection\",");
		file.WriteLine("\"name\": \"fps\",");
		file.WriteLine(string.Format("\"minfps\": %1,", Math.Floor(min_fps)));
		file.WriteLine(string.Format("\"maxfps\": %1,", Math.Floor(max_fps)));
		file.WriteLine(string.Format("\"avgfps\": %1,", Math.Floor(avg_fps)));
		file.WriteLine("\"crs\": { \"type\": \"name\", \"properties\": { \"name\": \"urn:ogc:def:crs:EPSG::32632\" } },");
		file.WriteLine("\"features\": [");

		for (int i = 0; i < m_geojson_data.Count(); ++i)
		{
			string line_end = ",";
			if (i == m_geojson_data.Count()-1)
				line_end = string.Empty;
			
			file.WriteLine(GetGeoJSONLine(m_geojson_data[i]) + line_end);
		}

		file.WriteLine("]\n}\n}");
		PrintToConsole("Data successfully written to the GeoJSON file.");
	}

	private void WriteDataCSV(FileHandle file)
	{
		if(!file || !file.IsOpen())
		{
			PrintToConsole("No file to write CSV data to found.", LogLevel.ERROR);
			return;
		}
		
		PrintToConsole("Adding data to CSV");
		
		file.WriteLine("x,z,fps");

		for (int i = 0; i < m_geojson_data.Count(); ++i)
			file.WriteLine(GetCSVLine(m_geojson_data[i]));

	}
	private string FormatTimestamp()
	{
		int year, month, day;
		System.GetYearMonthDay(year, month, day);
		string smonth, sday;
		if (month < 10)
			smonth = string.Format("0%1", month);
		else
			smonth = string.Format("%1", month);

		if (day < 10)
			sday = string.Format("0%1", day);
		else
			sday = string.Format("%1", day);

		return string.Format("%1%2%3", year, smonth, sday);
	}
	
    private void OnSendDataError(RestCallback cb)
    {
		if (cb.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			Print("[FPS_Autotest] Sending data has timed out", LogLevel.WARNING);
		}
		else
		{
			Print("[FPS_Autotest] Sending data has failed with errorCode[" + cb.GetHttpCode() + "]", LogLevel.WARNING);
		}
		
		GetGame().RequestClose();
    };
 
    private void OnSendDataSuccess(RestCallback cb)
    {
        Print("[FPS_Autotest] Data sent successfully. Size: " + cb.GetData().Length());
		GetGame().RequestClose();
    };
};