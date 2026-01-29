[BaseContainerProps()]
class SCR_AIGroupSubordinationCheck : SCR_AIGroupConditionCheck
{
	//------------------------------------------------------------------------------------------------
	override bool CheckCondition(SCR_AIGroup group)
	{
		if (!super.CheckCondition(group))
			return false;

		return !group.IsSlave();
	}
}
