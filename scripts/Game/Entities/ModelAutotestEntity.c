// #define DEBUG_PRINT
[EntityEditorProps(category: "GameScripted/Utility", description: "Model autotest entity", color: "0 0 255 255")]
class SCR_ModelAutotestEntityClass: SCR_GridSpawnerEntityClass
{
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ModelAutotestCamera
{
	[Attribute("UndefinedCamera", UIWidgets.EditBox, "The name of this camera.", "")]
	private string m_sCameraLabel;
	[Attribute("-30 25 -30", UIWidgets.EditBox, "Camera position in world coordinates (metres)", "")]
	private vector m_vCameraPosition;
	[Attribute("0 45 0", UIWidgets.EditBox, "Camera rotation in world coordinates (degrees)", "")]
	private vector m_vCameraRotation;
	[Attribute("5", UIWidgets.Slider, "The interval before this camera is done.", "0 100 1")]
	private float m_fCameraDuration;
	[Attribute("false", UIWidgets.CheckBox, "Whether camera is set from the center of spawned objects, or in global coordinates.", "")]
	private bool m_bRelativeToObjectsCenter;
	
	// array to hold frames per second captured by this camera
	private ref array<float> m_aFramesPerSecond = null;
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether camera should be set in global world coordinates or relative to spawned objects space. (boolean)
	bool GetIsRelativeToCenter()
	{
		return m_bRelativeToObjectsCenter;	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the label (name) of this camera. (string)
	string GetLabel()
	{
		return m_sCameraLabel;
	}

	//------------------------------------------------------------------------------------------------
	//! Return the position of this camera in world space. (metres; x,y,z)
	vector GetPosition()
	{
		return m_vCameraPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return the rotation of this camera in world space. (degrees; pitch, yaw, roll)
	vector GetRotation()
	{
		return m_vCameraRotation;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return the duration of how long this camera should be active. (seconds)
	float GetDuration()
	{
		return m_fCameraDuration;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this every frame where you want to record data for the camera
	void LogFramesPerSecond(float timeSlice)
	{
		if (!m_aFramesPerSecond)
			m_aFramesPerSecond = new array<float>();
		
		m_aFramesPerSecond.Insert((float)System.GetFPS());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ClearFramesPerSecond()
	{
		if (!m_aFramesPerSecond)
		{
			return;
		}
		
		m_aFramesPerSecond.Clear();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return Returns true if any frame data was captured, false if no data is found.
	bool GetFramesPerSecondsData(out float minFps, out float maxFps, out float averageFps)
	{
		// no data return false
		
		if (!m_aFramesPerSecond)
			return false;
		int fpsArrayLength = m_aFramesPerSecond.Count();
		if (fpsArrayLength <= 0)
			return false;
		
		float min = 99999999.0;
		float max = -99999999.0;
		float total = 0.0;
		float avg = 0.0;
		
		// find max, min and add to total fps
		for (int i = 0; i < fpsArrayLength; i++)
		{
			float currentFrame = m_aFramesPerSecond[i];
			
			if (currentFrame > max)
				max = currentFrame;
			
			if (currentFrame < min)
				min = currentFrame;
			
			total += currentFrame;
		}
		// calculate avg from total fps / fps captures count
		avg = total / (float)fpsArrayLength;
		
		// output results
		minFps = min;
		maxFps = max;
		averageFps = avg;
		
		return true;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Set engine's camera transformation to the one specified by this instance.
	void SetAsCamera(BaseWorld world, vector center)
	{
		// target cam matrix
		vector camMat[4];
		
		// create a matrix from euler angles
		vector camRot = m_vCameraRotation;
		vector eulerAngles = Vector(camRot[1], camRot[0], camRot[2]);
		vector rotMat[3];
		Math3D.AnglesToMatrix(eulerAngles, rotMat);
		
		// fill matrix with rotation data
		camMat[0] = rotMat[0];
		camMat[1] = rotMat[1];
		camMat[2] = rotMat[2];
		
		// decide camera world position, fill matrix
		vector centerOffset = vector.Zero;		
		if (m_bRelativeToObjectsCenter)
		{
			centerOffset = center;
			centerOffset[1] = 0;
		}		
		camMat[3] = m_vCameraPosition + centerOffset;
		
		// set engine camera's transform
		int cameraIndex = world.GetCurrentCameraId();
		world.SetCameraEx(cameraIndex, camMat);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ModelAutotestCamera()
	{
		if (m_aFramesPerSecond)
		{
			m_aFramesPerSecond.Clear();
			m_aFramesPerSecond = null;
		}
	}
};


//------------------------------------------------------------------------------------------------
class SCR_ModelAutotestEntity: SCR_GridSpawnerEntity
{
	//! Name of page for this autotest to be used on Confluence.
	[Attribute("AR Models Autotest", UIWidgets.EditBox, "Target result page name in Confluence", "")]
	private string m_sConfluenceParentPageName;
	//! A list of all cameras to be used in this autotest.
	[Attribute("", UIWidgets.Object, "Camera definitions for each model spawned", "")]
	private ref array<ref SCR_ModelAutotestCamera> m_aCameras;

	//! Time to wait before first test is initialized
	private float m_fWarmupDelay = 2.5;
	//! Time passed since level start
	private float m_fTimePassed = 0.0;
	//! Time to wait after an object is loaded
	
	private bool m_bWarmedUp = false;	// true after warmup delay is passed
	private bool m_bRespawningObjects = false; // set to true during load or despawn of an object
	private bool m_bTestStarted = false; // set to true if proper object is spawned
	private bool m_bCameraSet = false;
	
	//! The currently measured object's index
	private int m_iCurrentObjectIndex = 0;
	//! Current camera index
	private int m_iCurrentCameraIndex = 0;	
	//! Max camera index
	private int m_iCamerasCount = 0;	
	//! Current camera
	private SCR_ModelAutotestCamera m_CurrentAutotestCamera = null;
	
	//! Center of the grid - is get from Spawn() function
	private vector m_vCameraPositionCenter = vector.Zero;
		
	//! Autotest register related
	private ref AutotestRegister m_AutotestRegister = null;
	private int m_iAutotestIndex = 0;
	
	
	//------------------------------------------------------------------------------------------------
	//! Returns path to the persistent file that holds index of current autotest within the same date.
	private string GetPersistentDataPath()
	{
		string indexFileName = m_sConfluenceParentPageName;
		indexFileName.Replace(" ", "");
		return string.Format("%1.idx", indexFileName);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Returns formatted timestamp as yyyy-mm-dd-autotestIndex.
	private string GetFormattedTimestamp()
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
	
	   m_AutotestRegister.SavePersistentData(GetPersistentDataPath(), string.Format("%1", m_iAutotestIndex));
	
	   return string.Format("%1%2%3-%4", year, smonth, sday, m_iAutotestIndex);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Validate whether settings for autotest and all required references or values are valid.
	//! \return Returns true if settings are valid, false otherwise.
	private bool ValidateAutotestSettings()
	{
		// Not in game
		if (!GetGame().GetWorldEntity())
			return false;
		
		
		// Array null? Error, return.
		if (!m_aCameras)
		{
			Print("No autotest cameras defined in ModelAutotestEntity! Returning to edit mode!", LogLevel.ERROR);
			GetGame().RequestClose();
			return false;
		}
		
		if (m_aCameras.Count() <= 0)
		{
			Print("No autotest cameras defined in ModelAutotestEntity! Returning to edit mode!", LogLevel.ERROR);
			GetGame().RequestClose();
			return false;			
		}
		
		// Camera null? Error, return.
		// also init camera if not null		
		if (!m_CurrentAutotestCamera)
		{
			SetCamera(GetGame().GetWorldEntity().GetWorld(), 0, m_vCameraPositionCenter);
		}
		else
		{
			Print("Specified autotest camera was invalid! Returning to edit mode!", LogLevel.ERROR);
			GetGame().RequestClose();
			return false;
		}

		// Invalid name page
		if (!m_sConfluenceParentPageName || m_sConfluenceParentPageName == string.Empty)
		{
			Print("Specified target confluence page is invalid! Returning to edit mode!", LogLevel.ERROR);
			GetGame().RequestClose();
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetCamera(BaseWorld world, int index, vector center)
	{
		m_CurrentAutotestCamera = m_aCameras[index];
		m_CurrentAutotestCamera.SetAsCamera(world, center);
		m_bCameraSet = true;
		OnSetCamera(world, index);
	}
		
	//------------------------------------------------------------------------------------------------
	private void WriteMeasurementData()
	{
		string name;
		ref MeasurementFile measurementFile = null;	
		if (GetObjectName(m_iCurrentObjectIndex, name))
		{
			int len = name.Length();				
			int lastBracket = name.IndexOf("}")+1;
			
			string formattedName = name.Substring(lastBracket, len-lastBracket);
			string fileName = formattedName;				
			// Replace directories slashes with # for filename
			fileName.Replace("/", "#");
			fileName.Replace("\\", "#");
			if (!measurementFile)
				measurementFile = m_AutotestRegister.OpenMeasurementFile(fileName, formattedName, MeasurementType.GraphLine, false);	
		
			string header = "date,";
			for (int i = 0; i < m_aCameras.Count(); i++)
			{
				if (i == m_aCameras.Count()-1)
					header += "@"+m_aCameras[i].GetLabel();
				else
					header += "@"+m_aCameras[i].GetLabel()+",";
			}
		
			
			string data = GetFormattedTimestamp()+",";
			for (int i = 0; i < m_iCamerasCount; i++)
			{
				float min, max, avg;	
				m_aCameras[i].GetFramesPerSecondsData(min, max, avg);
				m_aCameras[i].ClearFramesPerSecond();
			
				if (i == m_iCamerasCount - 1)
					data += avg.ToString();
				else
					data += avg.ToString() + ",";
				
			}
			measurementFile.SetGraphHeader(header);
			measurementFile.AddData(data);
			measurementFile = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when a new camera is selected.
	protected event void OnSetCamera(BaseWorld world, int cameraIndex)
	{
		Print(string.Format("| Model Autotest | : Camera set to \"%1\"!", m_aCameras[cameraIndex].GetLabel()));
		// Override me!
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when a new model is selected.
	protected event void OnSetModel(int modelIndex)
	{
		string mdlName = string.Empty;
		if (!GetObjectName(m_iCurrentObjectIndex, mdlName))
			mdlName = m_iCurrentObjectIndex.ToString();
		Print(string.Format("| Model Autotest | : Model set to \"%1\"!", mdlName)); 
		// Override me!
	}

	//------------------------------------------------------------------------------------------------
	//! Called just before the autotest is started. (Serves as a replacement for Init)
	protected event void OnAutotestInitialized()
	{
		// Override me!
	}

	//------------------------------------------------------------------------------------------------
	//! Called every frame. (Serves as a replacement for OnFrame)
	protected event void OnAutotestFrame(float timeSlice)
	{
		// Override me!
	}
	
	//------------------------------------------------------------------------------------------------
	private override void EOnFrame(IEntity owner, float timeSlice)
	{
		// camera is out of bounds, get next object
		if (m_iCurrentCameraIndex >= m_iCamerasCount)
		{
			// 	TODO:
			//	output data to file
			WriteMeasurementData();
			m_bTestStarted = false;
			m_iCurrentObjectIndex++;
			m_iCurrentCameraIndex = 0;
			m_bTestStarted = false;
		}
	
		if (!m_bCameraSet)
		{
			SetCamera(owner.GetWorld(), m_iCurrentCameraIndex, m_vCameraPositionCenter);
		}
	
		// test not started, get next object and
		if (!m_bTestStarted)
		{
			if (!Spawn(m_iCurrentObjectIndex, m_vCameraPositionCenter))
			{
				// TODO:
				// objects oob, quit test
				GetGame().RequestClose();
			}
			
			OnSetModel(m_iCurrentObjectIndex);
			m_bWarmedUp = false;
			m_bTestStarted = true;
		}
	
		OnAutotestFrame(timeSlice);
	
		// objects are spawned
		if (m_bTestStarted)
		{
			// warmup first
			if (!m_bWarmedUp)
			{
				m_fTimePassed += timeSlice;
				if (m_fTimePassed <= m_fWarmupDelay)
				{
					return;
				}
				else
				{
					int testNr = m_iCamerasCount*m_iCurrentObjectIndex + m_iCurrentCameraIndex+1;
					Print(string.Format("| Model Autotest | : Test number %1 started!", testNr));
					m_bWarmedUp = true;
					m_fTimePassed = 0.0;
				}
			}
		
			float currentCameraTime = m_CurrentAutotestCamera.GetDuration();
			
			if (m_fTimePassed < currentCameraTime)
			{
				m_CurrentAutotestCamera.LogFramesPerSecond(timeSlice);
				m_fTimePassed+=timeSlice;
			}
			else
			{
				int testNr = m_iCamerasCount*m_iCurrentObjectIndex + m_iCurrentCameraIndex+1;
				Print(string.Format("| Model Autotest | : Test number %1 finished!", testNr));
			
				float min, max, avg;
				m_aCameras[m_iCurrentCameraIndex].GetFramesPerSecondsData(min, max, avg);
				Print(string.Format("Test number %4 Results: Min FPS: %1 | Max FPS: %2 || Average FPS: %3", min, max, avg, testNr));
				Print("-----------------------------------------------------------------");
				m_fTimePassed = 0.0;
				m_bWarmedUp = false;
				m_bCameraSet = false;
				m_iCurrentCameraIndex++;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float GetCurrentTestDuration()
	{
		if (m_bTestStarted && m_bWarmedUp)
		{
			return m_fTimePassed;
		}
		else
			return 0.0;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetCurrentTestMaxDuration()
	{
		if (!m_CurrentAutotestCamera)
			return 0.0;
		else
			return m_CurrentAutotestCamera.GetDuration();
	}
	
	//------------------------------------------------------------------------------------------------
	private override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!ValidateAutotestSettings())
			return;
		
		// Initialize camera count and array for fps
		m_iCamerasCount = m_aCameras.Count();		
		
		// Creates autotest register
		m_AutotestRegister = new AutotestRegister();
		m_AutotestRegister.Init(m_sConfluenceParentPageName);		
		
		// Autotest run index within the current date.
		string idxLine = m_AutotestRegister.LoadPersistentData(GetPersistentDataPath());
	    if(idxLine.Length() == 0)
	        m_iAutotestIndex = 0;
	    else
	        m_iAutotestIndex = idxLine.ToInt() + 1;
		
		// Initialize the autotest itself!
		Print(string.Format("| Model Autotest | :  Starting in %1s", m_fWarmupDelay));
	
		OnAutotestInitialized();		
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_ModelAutotestEntity()
	{
		for (int i = 0; i < m_aCameras.Count(); i++)
		{
			m_aCameras[i] = null;
		}
	}
};
