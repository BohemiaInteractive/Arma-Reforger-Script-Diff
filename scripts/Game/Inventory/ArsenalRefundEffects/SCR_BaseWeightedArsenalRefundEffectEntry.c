//! Effects that support the weighted variant of Arsenal Refund effects. Only works for SCR_BaseWeightedArsenalRefundEffect
[BaseContainerProps(), BaseContainerCustomCheckIntTitleField("m_bEnabled", "DO NOT USE BASE CLASS", "(Disabled) DO NOT USE BASE CLASS", 1)]
class SCR_BaseWeightedArsenalRefundEffectEntry : SCR_BaseArsenalRefundEffect
{
	[Attribute("10", desc: "Only works as part of the SCR_WeightedListArsenalRefundEffect class not in the SCR_MultiExecuteListArsenalRefundEffect or directly in the SCR_ArsenalRefundEffectComponent!\n\nThe heigher the weight the higher the chance is that the entry is randomly selected \n\nWARNING: All weights are counted up together. ake make sure all totals do not exceed max int value: '2147483647'", params: "1 9999")]
	int m_iWeight;
}