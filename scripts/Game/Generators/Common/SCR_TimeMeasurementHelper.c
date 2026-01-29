//! This class can add multiple measurements - useful to measure specific parts of a loop
//! \code
//! SCR_TimeMeasurementHelper benchmark = new SCR_TimeMeasurementHelper();
//! Debug.BeginTimeMeasure();
//! foreach (SCR_Object obj : m_aObjects)
//! {
//!		benchmark.BeginMeasure("opAandC");
//! 	obj.OperationA();
//!		benchmark.EndMeasure("opAandC");	// names are case-sensitive
//!
//! 	benchmark.BeginMeasure("opB");
//! 	obj.OperationB();
//! 	benchmark.EndMeasure("opB");
//!
//!		benchmark.BeginMeasure("opAandC");
//! 	obj.OperationC();
//!		benchmark.EndMeasure("opAandC");	// time will be -added- to this measurement
//! }
//! Debug.EndTimeMeasure("Operation A, B and C total benchmark");
//! Print("Operation A && C: " + benchmark.GetMeasure("opAandC") + " ms", LogLevel.NORMAL);
//! Print("Operation B only: " + benchmark.GetMeasure("opB") + " ms", LogLevel.NORMAL);
//! // or
//! benchmark.PrintAllMeasures();
//! \code
class SCR_TimeMeasurementHelper
{
	protected ref map<string, ref SCR_TimeMeasurementHelper_Info> m_mData;

	//------------------------------------------------------------------------------------------------
	//! Begin a measurement with a name. If a measurement with this name is already started, its begin time is replaced
	//! \param[in] name case-sensitive measurement name
	void BeginMeasure(string name)
	{
		SCR_TimeMeasurementHelper_Info info = m_mData.Get(name);
		if (!info)
		{
			info = new SCR_TimeMeasurementHelper_Info();
			m_mData.Insert(name, info);
		}

		info.m_fStart = System.GetTickCount();
	}

	//------------------------------------------------------------------------------------------------
	//! End a started measurement
	//! If the measurement has not been started, a warning is printed
	//! \param[in] name case-sensitive, must be identical to BeginMeasure's provided name
	void EndMeasure(string name)
	{
		SCR_TimeMeasurementHelper_Info info = m_mData.Get(name);
		if (!info)
		{
			Print("No measurement named \"" + name + "\" was found", LogLevel.WARNING);
			return;
		}

		info.m_fTotal += System.GetTickCount(info.m_fStart);
	}

	//------------------------------------------------------------------------------------------------
	//! Get a measurement by name
	//! If the measurement has never been ended, its value is 0
	//! If the measurement has not been started, a warning is printed and -1 is returned
	//! \param[in] name the case-sensitive measurement name
	//! \return measured time in ms, 0 if the measurement has not been ended, -1 if the measurement does not exist
	float GetMeasure(string name)
	{
		SCR_TimeMeasurementHelper_Info info = m_mData.Get(name);
		if (!info)
		{
			Print("No measurement named \"" + name + "\" was found", LogLevel.WARNING);
			return -1;
		}

		return info.m_fTotal;
	}

	//------------------------------------------------------------------------------------------------
	//! Print all measurements to the console, ended or not
	void PrintAllMeasures()
	{
		foreach (string name, SCR_TimeMeasurementHelper_Info info : m_mData)
		{
			Print("Measure \"" + name + "\": " + info.m_fTotal + " ms", LogLevel.NORMAL);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Delete all stored measurements, ended or not
	void Reset()
	{
		m_mData.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_TimeMeasurementHelper()
	{
		m_mData = new map<string, ref SCR_TimeMeasurementHelper_Info>();
	}
}

// databag
class SCR_TimeMeasurementHelper_Info
{
	int m_fStart;
	int m_fTotal;
}
