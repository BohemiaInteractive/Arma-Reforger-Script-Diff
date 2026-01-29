[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_SecondTargetClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_SecondTarget : SCR_BaseTutorialStage
{
	protected int m_iMisses;
	protected int m_iResponse;
	protected float m_fLastShellDistanceSq;

	//------------------------------------------------------------------------------------------------
	static void SpawnShell()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (!gameMode)
			return;
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.Cast(gameMode.FindComponent(SCR_TutorialGamemodeComponent));
		
		if (!tutorial)
			return;
		
		tutorial.SpawnAsset("MortarShell", "{DD6844AB03FDA84F}Prefabs/Weapons/Ammo/Ammo_Shell_81mm_Practice_M879.et");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (!eventName.IsEmpty() || m_iResponse == 0)
			return;
		
		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", m_iResponse);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		m_TutorialComponent.RemovePlayerMapMarkers();
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", 13);
		
		SpawnShell();
		CreateMarkerCustom("FIRESUPPORT_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
		CreateMarkerCustom(GetGame().GetWorld().FindEntityByName("SecondTarget"), SCR_EScenarioFrameworkMarkerCustom.CIRCLE, SCR_EScenarioFrameworkMarkerCustomColor.RED);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		IEntity shell = GetGame().GetWorld().FindEntityByName("MortarShell");
		
		if (shell)
		{
			IEntity target = GetGame().GetWorld().FindEntityByName("SecondTarget");
			
			if (!target)
				return false;
			
			m_fLastShellDistanceSq = vector.DistanceSq(shell.GetOrigin(), target.GetOrigin());
		}
		else
		{
			if (m_fLastShellDistanceSq < 100 * 100)
			{
				SCR_VoiceoverSystem.GetInstance().PlaySequence("Target2Hit");
				m_iResponse = 15;
				return true;
			}
			
			SCR_HintManagerComponent.HideHint();
			SCR_TutorialStageMortars_SecondTarget.SpawnShell();
			m_iMisses++;

			switch (m_iMisses)
			{
				case 1:
				{
					SCR_VoiceoverSystem.GetInstance().PlaySequence("Target2Miss1");
					m_iResponse = 16;
					break;
				}
				
				case 2:
				{
					SCR_VoiceoverSystem.GetInstance().PlaySequence("Target2Miss2");
					m_iResponse = 17;
					break;
				}
				
				default:
				{
					SCR_VoiceoverSystem.GetInstance().PlaySequence("Target2Miss3");
					m_iResponse = 18;
					break;
				}
			}
		}

		return false;
	}
}