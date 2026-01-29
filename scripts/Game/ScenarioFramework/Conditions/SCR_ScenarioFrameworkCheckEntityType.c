[BaseContainerProps()]
class SCR_ScenarioFrameworkCheckEntityType : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Checked entity")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Checked class names.")]
	ref array<string> m_aClassNames;
	
	[Attribute(defvalue: "1", desc: "Allow Inherited types")]
	bool m_bAllowInherited;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		if (!m_aClassNames || m_aClassNames.IsEmpty())
			return false;
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return false;
		
		IEntity checkedEntity = entityWrapper.GetValue();
		if (!checkedEntity)
			return false;
		
		foreach (string classname : m_aClassNames)
		{
			if (m_bAllowInherited && checkedEntity.IsInherited(classname.ToType()))
				return true;
			
			if (checkedEntity.Type() == classname.ToType())
				return true;
		}

		return false;
	}
}