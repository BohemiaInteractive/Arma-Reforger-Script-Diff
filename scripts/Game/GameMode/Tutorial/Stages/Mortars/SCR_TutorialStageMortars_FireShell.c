[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_FireShellClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_FireShell : SCR_BaseTutorialStage
{
	protected bool m_bShellFired;

	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", 7);
		
		RegisterWaypoint("WP_MORTAR_GETIN", "", "EXPLOSIVES");
		
		IEntity mortar = GetGame().GetWorld().FindEntityByName("Mortar");
		
		if (!mortar)
			return;
		
		SCR_MortarMuzzleComponent muzzleComponent = SCR_MortarMuzzleComponent.Cast(mortar.FindComponent(SCR_MortarMuzzleComponent));
		
		if (!muzzleComponent)
			return;
		
		SCR_MuzzleEffectComponent muzzleEffectComp = SCR_MuzzleEffectComponent.Cast(muzzleComponent.FindComponent(SCR_MuzzleEffectComponent));
		
		if (!muzzleEffectComp)
			return;
		
		muzzleEffectComp.GetOnWeaponFired().Insert(OnMortarFired);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMortarFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		m_bShellFired = (projectileEntity == GetGame().GetWorld().FindEntityByName("MortarShell"));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bShellFired;
	}
};