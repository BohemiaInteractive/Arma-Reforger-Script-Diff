//------------------------------------------------------------------------------------------------
//! Base for any condition containing multiple sub-conditions
[BaseContainerProps()]
class SCR_AvailableActionsGroupCondition : SCR_AvailableActionCondition
{	
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_AvailableActionCondition> m_aConditions;
};
