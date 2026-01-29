[BaseContainerProps()]
class SCR_SoundBellTime
{
	[Attribute("", UIWidgets.EditBox, "")]
	float m_fBellTime;
	
	[Attribute("", UIWidgets.EditBox, "")]
	int m_iRepetitionCount;
}

[ComponentEditorProps(category: "GameScripted/Sound", description: "Bell Sound Component")]
class SCR_BellSoundComponentClass : SoundComponentClass
{
}

class SCR_BellSoundComponent : SoundComponent
{		
	[Attribute("", UIWidgets.Object, "Time settings")]
	ref array<ref SCR_SoundBellTime> m_aSoundBellTime;
	
	private static GameSignalsManager m_GameSignalManager;
	private int m_iTimeOfDaySignalIdx;
	
	private const float TRIGGER_INTERVAL = 850;
		
	protected float m_fTimer;
	protected float m_fTimeOfDayValueLast;	
	
	protected int m_iBellTimeIdx = -1;
	protected int m_iSoundBellTimeCount;
	protected int m_iRepetitionCount;
	
	private float m_fWorldTimeLast;

	//------------------------------------------------------------------------------------------------		
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{	
		// UpdateSoundJob only once pref frame
		float worldTime = owner.GetWorld().GetWorldTime();
		
		if (m_fWorldTimeLast == worldTime)
			return;
				
		float dt = worldTime - m_fWorldTimeLast;
		float timeOfDay = m_GameSignalManager.GetSignalValue(m_iTimeOfDaySignalIdx) * 24;
				
		if (m_iRepetitionCount != 0)
		{
			m_fTimer += dt;
			
			if (m_fTimer > TRIGGER_INTERVAL)
			{			
				//Play sound							
				string eventName;
				
				if (m_iRepetitionCount == 1)
				{
					eventName = SCR_SoundEvent.SOUND_BELL_END;
				}
				else if (m_iRepetitionCount%2 == 0)
				{
					eventName = SCR_SoundEvent.SOUND_BELL_A;
				}
				else
				{
					eventName = SCR_SoundEvent.SOUND_BELL_B;
				}

				SoundEvent(eventName);
				
				m_iRepetitionCount--;
				m_fTimer = 0;
			}			
		}
		else
		{	
			// Get m_iBellTimeIdx
			if (m_iBellTimeIdx == - 1)
			{		
				m_iBellTimeIdx = 0;
				
				for (int i = 0; i < m_iSoundBellTimeCount; i++)
				{
					if (m_aSoundBellTime[i].m_fBellTime > timeOfDay)
					{
						m_iBellTimeIdx = i;
						break;
					}
				}		
			}
			
			// OnFrame check if sound should be triggered
			if (m_aSoundBellTime[m_iBellTimeIdx].m_fBellTime < timeOfDay && m_aSoundBellTime[m_iBellTimeIdx].m_fBellTime >= m_fTimeOfDayValueLast)
			{
				m_iRepetitionCount = m_aSoundBellTime[m_iBellTimeIdx].m_iRepetitionCount;
				m_iBellTimeIdx = -1;
			}				
		}
		
		// Set Last values
		m_fTimeOfDayValueLast = timeOfDay;		
		m_fWorldTimeLast = worldTime;
	}
	
	//------------------------------------------------------------------------------------------------		
	override void OnUpdateSoundJobBegin(IEntity owner)
	{
		m_fTimeOfDayValueLast = m_GameSignalManager.GetSignalValue(m_iTimeOfDaySignalIdx) * 24;
		m_iRepetitionCount = 0;
		m_iBellTimeIdx = -1;	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
						
		// Set Init
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{		
		// Get m_aSoundBellTime count
		m_iSoundBellTimeCount = m_aSoundBellTime.Count();
		
		// Disable OnFrame if no time is defined
		if (m_iSoundBellTimeCount == 0)
		{
			SetScriptedMethodsCall(false);
			return;
		}
		
		// Get GameSignalsManager
		m_GameSignalManager = GetGame().GetSignalsManager();

		m_iTimeOfDaySignalIdx = m_GameSignalManager.AddOrFindSignal("TimeOfDay");
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_BellSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);	
	}
}
