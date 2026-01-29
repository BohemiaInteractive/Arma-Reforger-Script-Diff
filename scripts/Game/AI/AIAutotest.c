[EntityEditorProps(category: "GameLib/Scripted/Autotest", description: "AIAutotest", dynamicBox: true)]
class AIAutotestClass: GenericEntityClass
{
};

class AIAutotest: GenericEntity
{
	[Attribute("AI Autotests", UIWidgets.EditBox, "Name of Confluence page", "")]
	private string m_pageName;

	[Attribute("Unnamed test", UIWidgets.EditBox, "Title of particular test", "")]
	private string m_testName;
	
	[Attribute("PlayerCharacter", UIWidgets.EditBox, "First entity to test completeness", "")]
	protected string m_entity1Name;
	
	[Attribute("", UIWidgets.EditBox, "Second entity to test completeness", "")]
	protected string m_entity2Name;
	
	[Attribute("-1", UIWidgets.EditBox, "Distance to test between entities", "")]
	protected float m_entityDistanceTest;
	
	[Attribute("-1", UIWidgets.EditBox, "Safety timeout, test will end after", "")]
	protected float m_timeout;
	
	[Attribute("false", UIWidgets.CheckBox, "If timeout is not failing test", "")]
	private bool m_testSucceedsAlways;
	
	[Attribute("", UIWidgets.EditBox, "AISpawnerGroup to check before starting measurement")]
	protected string m_SpawnerName;
		
	protected GenericEntity m_FirstEnt;
	protected GenericEntity m_SecondEnt;
	private float m_avgFPS;
	private float m_lowFPS;
	private float m_highFPS;
	private string m_worldFilePath;
	protected float m_Timer;
	private int m_Index;
	protected string m_WorldName;
	
	protected ref MeasurementFile m_ResultFile;
	protected ref MeasurementFile m_graphfile;
	
	protected ref AutotestRegister m_register;
	
	protected AISpawnerGroup m_SpawnerGroup;
	
    void AIAutotest(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		
		m_WorldName = "";
    }
	
	override void EOnInit(IEntity owner) //!EntityEvent.INIT
	{
		m_FirstEnt = null;
		m_SecondEnt = null;
		m_avgFPS = 0;
		m_lowFPS = 1000;
		m_highFPS = -1;	
		m_Timer = -1;
		m_Index = 1;
	}
    
	bool ShouldEnd()
	{
		if (m_timeout > 0 && m_Timer > m_timeout)
			return true;
		
		if (m_FirstEnt && m_SecondEnt && m_entityDistanceTest > 0)
		{
			vector diff = (m_FirstEnt.GetOrigin() - m_SecondEnt.GetOrigin()); 
			float dist = diff.Length();
			if (dist < m_entityDistanceTest)
				return true;
		}
		return false;
	}
	
	bool HasPassed()
	{
		if (m_testSucceedsAlways)
			return true;
		
		if (m_timeout > 0 && m_Timer < m_timeout)
			return true;
		
		return false;
	}
	
	void InitTestResultFiles()
	{	
		if (GetGame())
		{
			m_WorldName = GetGame().GetWorldFile();
			int lastslash = m_WorldName.LastIndexOf("/");
			m_WorldName = m_WorldName.Substring(lastslash, m_WorldName.Length() - lastslash);
		}
		//! Find world file path
		if(!m_register)
		{
			m_register = new AutotestRegister();
			m_register.Init(m_pageName);
		}		
		if (!m_ResultFile)
		{
			MeasurementFile descrFile = m_register.OpenMeasurementFile("000_ai_test_descr", "", MeasurementType.HTML);
			descrFile.AddData("<h1>AI Autotests</h1>");			

			m_Index = m_register.GetTestIndex() + 1;
			string name = string.Format("%1_test", m_WorldName);
			m_ResultFile = m_register.OpenMeasurementFile(name, "", MeasurementType.HTML, true);
		}
	}
	
	string FormatTimestamp()
	{
		int year, month, day, hour, minute, sec;
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, sec);
		string smonth, sday, shour;
		if(month < 10)
			smonth = string.Format("0%1", month);
		else
			smonth = string.Format("%1", month);

		if(day < 10)
			sday = string.Format("0%1", day);
		else
			sday = string.Format("%1", day);
		
		if (hour < 10)
			shour = string.Format("0%1", hour);
		else
			shour = string.Format("%1", hour);

		return string.Format("%1%2%3-%4", year, smonth, sday, shour);
	}
	
	string FormatIndex(int index)
	{
		string sindex;
		if (index < 10)
			sindex = string.Format("00%1", index);
		else if (index < 100)
			sindex = string.Format("0%1", index);
		else
			sindex = string.Format("%1", index);
		return sindex;
	}
	
	string FormatAutotestInfo()
	{
		return string.Format("avg %1 min %2 max %3", m_avgFPS, m_lowFPS, m_highFPS);
	}
	
	void CreateGraph(string graphName)
	{
		m_graphfile = m_register.OpenMeasurementFile(graphName, "", MeasurementType.GraphLine, false);
		m_graphfile.SetGraphHeader("Measurement index, Average FPS");
		m_graphfile.AddData(string.Format("%1,%2", FormatTimestamp(), m_avgFPS));
	}
	
	override void EOnFrame(IEntity owner, float timeSlice) //!EntityEvent.FRAME
	{	
		if (timeSlice <= 0)
			return;
		
		if (m_SpawnerName.Length() > 0 && !m_SpawnerGroup)
		{
			m_SpawnerGroup = AISpawnerGroup.Cast(owner.GetWorld().FindEntityByName(m_SpawnerName));
			return;
		}
		else if (m_SpawnerGroup)
		{
			if (!m_SpawnerGroup.IsSpawningFinished())
				return;
		}
		
		if (m_Timer < 0)
			m_Timer = 0;
		else
		{
			m_Timer += timeSlice;
		}
		
		if (!m_ResultFile)
			InitTestResultFiles();		
		
		m_avgFPS = (m_avgFPS + (1 / timeSlice)) / 2.0;
		float currFPS2 = System.GetFPS();
		float currFPS = 1/timeSlice;
		
		if (currFPS < m_lowFPS)
			m_lowFPS = currFPS;
		if (currFPS > m_highFPS)
			m_highFPS = currFPS;
		
		if (!m_FirstEnt && m_entity1Name.Length() > 0)
		{
			m_FirstEnt = GenericEntity.Cast(GetGame().FindEntity(m_entity1Name));
		}
		if (!m_SecondEnt && m_entity2Name.Length() > 0)
		{
			m_SecondEnt = GenericEntity.Cast(GetGame().FindEntity(m_entity2Name));
		}
		
		if (ShouldEnd())
		{			
			Print(string.Format("FPS: avg %1 min %2 max %3", m_avgFPS, m_lowFPS, m_highFPS));
			Print(string.Format("Time spent: %1 s", m_Timer));

			string result = "<span style='color: rgb(255,64,64);'>Fail</span>";
			if (HasPassed()) 
				result = "<span style='color: rgb(64,255,64);'>Success</span>";
			if (m_ResultFile)
			{
				m_ResultFile.AddData("<table><tr><th>Test</th><th>Result</th><th>FPS</th><th>Time Spent</th></tr>");
				m_ResultFile.AddData(string.Format("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4s</td></tr>",
					m_testName, result, FormatAutotestInfo(), m_Timer));
				m_ResultFile.AddData("</table>");
			}
			string graphName = string.Format("%1_test_graph", m_WorldName);
			CreateGraph(graphName);
			
			GetGame().RequestClose(); //< End mission
		}
	}
};