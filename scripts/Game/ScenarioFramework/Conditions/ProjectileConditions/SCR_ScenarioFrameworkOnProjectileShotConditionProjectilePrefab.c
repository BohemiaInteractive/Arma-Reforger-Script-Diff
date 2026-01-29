[BaseContainerProps()]
class SCR_ScenarioFrameworkOnProjectileShotConditionProjectilePrefab : SCR_ScenarioFrameworkOnProjectileShotConditionBase
{
	[Attribute(defvalue: "", desc: "Prefab Resource Name", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sPrefabResourceName;
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkOnProjectileShotConditionProjectilePrefab.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!entity)
			return false;
		
		return entity && entity.GetPrefabData().GetPrefabName() == m_sPrefabResourceName;
	}
}