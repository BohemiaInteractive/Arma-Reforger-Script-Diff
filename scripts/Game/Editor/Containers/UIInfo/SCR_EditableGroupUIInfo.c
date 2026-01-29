[BaseContainerProps()]
class SCR_EditableGroupUIInfo : SCR_EditableEntityUIInfo
{
	[Attribute("0", desc: "Will use Name that can be set dirrectly in UI info instead of using dynamic name by m_MilitarySymbol")]
	protected bool m_bUseUIInfoName;
	
	[Attribute()]
	protected ref SCR_MilitarySymbol m_MilitarySymbol;
	
	protected SCR_MilitarySymbol m_MilitarySymbolInstance; //--- No 'ref'

	//------------------------------------------------------------------------------------------------
	//! Get military symbol.
	//! \return Military symbol
	SCR_MilitarySymbol GetMilitarySymbol()
	{
		if (m_MilitarySymbolInstance)
			return m_MilitarySymbolInstance;
		else
			return m_MilitarySymbol;
	}

	//------------------------------------------------------------------------------------------------
	//! Set info instance.
	//! Use only when the UI info is on entity instance, not prefab!
	//! \param symbol Military symbol
	//! \param name
	void SetInstance(SCR_MilitarySymbol symbol, LocalizedString name)
	{
		Name = name;
		m_MilitarySymbolInstance = symbol;
	}

	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		if (m_bUseUIInfoName)
			return super.GetName();
		
		if (!m_MilitarySymbolInstance)
		{
			SCR_GroupIdentityCore core = SCR_GroupIdentityCore.Cast(SCR_GroupIdentityCore.GetInstance(SCR_GroupIdentityCore));
			if (core)
			{
				SCR_GroupNameConfig nameManager = core.GetNames();
				if (nameManager)
				{					
					string name = nameManager.GetGroupName(GetMilitarySymbol());				
					return name;
				}
			}
		}
		return super.GetName();
	}

	//------------------------------------------------------------------------------------------------
	//! Get Entity budget costs
	override bool GetEntityBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Group budget costs
	//! Use when you need to get a set budget values for the group and don't want to relay on default logic where AI budget is deducted by individually spawned AI.
	bool GetGroupBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets)
	{
		return super.GetEntityBudgetCost(outBudgets);
	}

	//------------------------------------------------------------------------------------------------
	//--- Override without 'protected' keyword
	override void CopyFrom(SCR_UIName source)
	{
		SCR_EditableGroupUIInfo editableGroupSource = SCR_EditableGroupUIInfo.Cast(source);
		if (editableGroupSource)
			m_MilitarySymbol = editableGroupSource.m_MilitarySymbol;
		
		super.CopyFrom(source);
	}
}
