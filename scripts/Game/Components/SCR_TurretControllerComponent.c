[ComponentEditorProps(category: "GameScripted/Turret", description: "Scripted turret controller", icon: HYBRID_COMPONENT_ICON)]
class SCR_TurretControllerComponentClass : TurretControllerComponentClass
{
}

class SCR_TurretControllerComponent : TurretControllerComponent
{
	//------------------------------------------------------------------------------------------------
	override void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player)
	{
		if (player && IsWeaponADS())
		{
			BaseSightsComponent sights = GetCurrentSights();
			if (sights)
			{
				SCR_SightsZoomFOVInfo fovInfo = SCR_SightsZoomFOVInfo.Cast(sights.GetFOVInfo());
				if (fovInfo && fovInfo.GetStepsCount() > 1)
					am.ActivateContext("TurretWeaponMagnificationContext");
			}
		}
	}
}
