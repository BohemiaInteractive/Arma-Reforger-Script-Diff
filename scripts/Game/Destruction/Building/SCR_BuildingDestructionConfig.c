[BaseContainerProps(configRoot: true)]
class SCR_BuildingDestructionConfig
{
	[Attribute(desc: "Add typenames inheriting from IEntity to exclude them from entity query during final destruction phase")]
	protected ref array<string> m_aExcludedEntityQueryTypes;

	ref array<typename> m_aExcludedEntityQueryTypenames;

	//------------------------------------------------------------------------------------------------
	void SCR_BuildingDestructionConfig()
	{
		typename t;
		foreach (string typeName : m_aExcludedEntityQueryTypes)
		{
			if (!m_aExcludedEntityQueryTypenames)
				m_aExcludedEntityQueryTypenames = {};

			t = typeName.ToType();
			if (!t)
			{
#ifdef WORKBENCH
				Print("Error: Excluded Entity Query Types contains typeName = " + typeName + " which doesnt exist!", LogLevel.ERROR);
#endif
				continue;
			}

			m_aExcludedEntityQueryTypenames.Insert(t);
		}
	}
}
