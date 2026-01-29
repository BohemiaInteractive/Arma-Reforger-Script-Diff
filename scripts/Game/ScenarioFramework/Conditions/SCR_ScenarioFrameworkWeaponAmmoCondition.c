[BaseContainerProps()]
class SCR_ScenarioFrameworkWeaponAmmoCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Entity to check.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(defvalue: "0", desc: "Activation Percentage", params: "0 100 1")]
	protected int m_iRequiredPercentage;
	
	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);

			return false;
		}
		
		IEntity weaponEntity = entityWrapper.GetValue();
		if (!weaponEntity)
			return false;
		
		BaseWeaponManagerComponent weaponMan = BaseWeaponManagerComponent.Cast(weaponEntity.FindComponent(BaseWeaponManagerComponent));
		if (!weaponMan)
			return false;
		
		int totalAmmo;
		
		array<IEntity> weapons = {};
		weaponMan.GetWeaponsList(weapons);
		
		BaseWeaponComponent weaponComp;
		array<BaseMuzzleComponent> outMuzzles;
		foreach (IEntity weapon : weapons)
		{
			weaponComp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
			if (!weaponComp)
				continue;
			
			outMuzzles = {};
			weaponComp.GetMuzzlesList(outMuzzles);
			
			foreach (BaseMuzzleComponent muzzle : outMuzzles)
			{
				totalAmmo += muzzle.GetAmmoCount();
			}
		}
		
		return (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (totalAmmo < m_iRequiredPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (totalAmmo <= m_iRequiredPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (totalAmmo == m_iRequiredPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (totalAmmo >= m_iRequiredPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (totalAmmo > m_iRequiredPercentage)) 
			);
	}
}