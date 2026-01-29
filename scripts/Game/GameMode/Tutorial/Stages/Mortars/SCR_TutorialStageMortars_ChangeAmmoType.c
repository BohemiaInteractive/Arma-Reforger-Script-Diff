[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_ChangeAmmoTypeClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_ChangeAmmoType : SCR_BaseTutorialStage
{
	SCR_BallisticTableDisplay m_BallisticTableDisplay;
	ResourceName m_sProjectilePrefab = "{DD6844AB03FDA84F}Prefabs/Weapons/Ammo/Ammo_Shell_81mm_Practice_M879.et";
	float m_fProjectileInitSpeedCoef = 2.932;
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", 2);
		
		IEntity mortar = GetGame().GetWorld().FindEntityByName("Mortar");
		if (!mortar)
			return;
		
		SCR_BaseHUDComponent hudComp = SCR_BaseHUDComponent.Cast(mortar.FindComponent(SCR_BaseHUDComponent));
		if (!hudComp)
			return;
		
		array <BaseInfoDisplay> infoDisplays = {};
		hudComp.GetInfoDisplays(infoDisplays);
		if (!infoDisplays || infoDisplays.IsEmpty())
			return;
		
		SCR_MortarInfo mortarInfo;
		foreach (BaseInfoDisplay display : infoDisplays)
		{
			mortarInfo = SCR_MortarInfo.Cast(display);
			if (mortarInfo)
				break;
		}
		
		if (!mortarInfo)
			return;
		
		mortarInfo.GetInfoDisplays(infoDisplays);
		
		foreach (BaseInfoDisplay display : infoDisplays)
		{
			m_BallisticTableDisplay = SCR_BallisticTableDisplay.Cast(display);
			if (m_BallisticTableDisplay)
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (!m_BallisticTableDisplay)
			return false;
		
		SCR_BaseBallisticConfig config = m_BallisticTableDisplay.GetSelectedBallisticConfig();
		if (!config)
			return false;
		
		return config.GetProjectilePrefab() == m_sProjectilePrefab && config.GetProjectileInitSpeedCoef() == m_fProjectileInitSpeedCoef;
	}
};