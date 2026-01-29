[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_ShellTravellingClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_ShellTravelling : SCR_BaseTutorialStage
{
	protected int m_iMisses;
	protected int m_iResponse;
	protected float m_fLastShellDistanceSq;
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (!eventName.IsEmpty() || m_iResponse == 0)
			return;
		
		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", m_iResponse);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", 8);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		IEntity shell = GetGame().GetWorld().FindEntityByName("MortarShell");
		
		if (shell)
		{
			IEntity target = GetGame().GetWorld().FindEntityByName("FirstTarget");
			
			if (!target)
				return false;
			
			m_fLastShellDistanceSq = vector.DistanceSq(shell.GetOrigin(), target.GetOrigin());
		}
		else
		{
			if (m_fLastShellDistanceSq < 100 * 100)
			{
				SCR_VoiceoverSystem.GetInstance().PlaySequence("Target1Hit");
				m_iResponse = 9;
				return true;
			}
			
			SCR_HintManagerComponent.HideHint();
			SCR_TutorialStageMortars_SecondTarget.SpawnShell();
			m_iMisses++;

			switch (m_iMisses)
			{
				case 1:
				{
					SCR_VoiceoverSystem.GetInstance().PlaySequence("Target1Miss1");
					m_iResponse = 10;
					break;
				}
				
				case 2:
				{
					SCR_VoiceoverSystem.GetInstance().PlaySequence("Target1Miss2");
					SCR_HintManagerComponent.ShowHint(m_StageInfo.GetHint(1));
					m_iResponse = 11;
					break;
				}
				
				default:
				{
					SCR_VoiceoverSystem.GetInstance().PlaySequence("Target1Miss3");
					m_iResponse = 12;
					break;
				}
			}
		}

		return false;
	}
};