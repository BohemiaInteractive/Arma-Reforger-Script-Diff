[ComponentEditorProps(category: "GameScripted/Sound", description: "Signal Debug Component")]
class SCR_SignalDebugComponentClass : ScriptComponentClass
{
}

class SCR_SignalDebugComponent : ScriptComponent
{	
	[Attribute("", uiwidget: UIWidgets.Object, "Signals names")]
	protected ref array<string> m_aSignalNames;
	
	[Attribute("false", uiwidget: UIWidgets.EditBox, "Print signals registered on sound component into log")]
	protected bool m_bPrintToLog;
	
	[Attribute("false", uiwidget: UIWidgets.EditBox, "Print signals registered on sound component into log")]
	protected bool m_bPrintSignalsOnSoundComponent;
	
	protected ref array<int> m_aSignalsID = new array<int>;
	protected int m_aSignalNamesCount;
	
	protected ref array<string> m_aSCSignalNames = new array<string>;
		
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected SoundComponent m_SoundComponent;

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_bPrintToLog)
		{
			Print("------------------");
		}
							
		for (int i = 0; i < m_aSignalNamesCount; i++)
		{
			string text = m_aSignalNames[i] + " :" + m_SignalsManagerComponent.GetSignalValue(m_aSignalsID[i]).ToString();
			
			DbgUI.Text(text);
			
			if (m_bPrintToLog)
			{
				Print(text);
			}
		}			
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{		
		// Get Components								
		GenericEntity genEnt = GenericEntity.Cast(owner);		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(genEnt.FindComponent(SignalsManagerComponent));		
		m_SoundComponent = SoundComponent.Cast(genEnt.FindComponent(SoundComponent));		
		
		if (!m_SignalsManagerComponent || !m_SoundComponent)
		{
			ClearEventMask(owner, EntityEvent.FRAME);
			return;
		}
		else
		{
			// Set OnFrame
			SetEventMask(owner, EntityEvent.FRAME | EntityEvent.INIT);
			
			// Get signals IDs
			foreach (string name: m_aSignalNames)
			{
				m_aSignalsID.Insert(m_SignalsManagerComponent.AddOrFindSignal(name));
			}
			
			m_aSignalNamesCount = m_aSignalNames.Count();
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{				
		if (m_bPrintSignalsOnSoundComponent)
		{
			m_SoundComponent.GetSignalNames(m_aSCSignalNames);
			
			Print("------- Registered signals on sound component -------");
			
			foreach (string name : m_aSCSignalNames)
			{
				Print(name);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_SignalDebugComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}
}
