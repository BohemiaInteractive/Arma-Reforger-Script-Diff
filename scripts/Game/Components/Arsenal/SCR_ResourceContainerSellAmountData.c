[BaseContainerProps(), BaseContainerCustomEnumWithValue(SCR_ESupplieDeductionMethod, "m_eCostReductionMethod", "m_fDeductedAmount", "1", "%1 - Missing supplies cost deduction: %2")]
class SCR_ResourceContainerSellAmountData : SCR_ArsenalAlternativeData
{
	[Attribute(defvalue: SCR_ESupplieDeductionMethod.FLAT.ToString(), desc: "Determins how the price should be reduced based on the amount of missing supplies.\nFlat reduction if current amount of supplies in the item is below or equal specified percentage.\nVALUE calculates how much to subtract by subtracting max supply amount from current supply amount. Uses DeductedAmount as a multiplier to fine tune the cost of missing supplies (normalPrice - missingAmount * DeductedAmount)\nPERCENTAGE_OF_MAX reduces the refund price by DeductedAmount for each amount of Percentage that is missing.", uiwidget: UIWidgets.ComboBox, enumType: SCR_ESupplieDeductionMethod)]
	protected SCR_ESupplieDeductionMethod m_eCostReductionMethod;
	
	[Attribute(defvalue: "1", desc: "Amount of supplies which is going to be deducted from default refund value based on selected method.", params: "0 inf")]
	protected float m_fDeductedAmount;

	[Attribute(defvalue: "0.5", desc: "Percentage of max supplies. Usage dpends on the method", params: "0 1 0.01")]
	protected float m_fPercentage;

	//------------------------------------------------------------------------------------------------
	override int GetRefundAmount(int defaultRefundAmount, IEntity soldItem = null)
	{
		if (!soldItem)
			return defaultRefundAmount;

		SCR_ResourceComponent resourceComp = SCR_ResourceComponent.FindResourceComponent(soldItem);
		if (!resourceComp)
			return defaultRefundAmount;

		SCR_ResourceContainer container = resourceComp.GetContainer(EResourceType.SUPPLIES);
		if (!container)
			return defaultRefundAmount;

		const float max = container.GetMaxResourceValue();
		if (max == 0)
			return defaultRefundAmount;

		const float current = container.GetResourceValue();
		switch (m_eCostReductionMethod)
		{
			case SCR_ESupplieDeductionMethod.FLAT:
			{
				float percentage = current / max;
				if (percentage <= m_fPercentage)
					return Math.Max(0, defaultRefundAmount - m_fDeductedAmount);

				break;
			}

			case SCR_ESupplieDeductionMethod.VALUE:
			{
				float missingAmount = max - current;
				if (missingAmount > 0)
					return Math.Max(0, defaultRefundAmount - missingAmount * m_fDeductedAmount);

				break;
			}

			case SCR_ESupplieDeductionMethod.PERCENTAGE_OF_MAX:
			{
				float penaltyMultiplier = Math.Ceil((1 - current / max) / m_fPercentage);
				if (penaltyMultiplier > 0)
					return defaultRefundAmount - penaltyMultiplier * m_fDeductedAmount;
			}
		}

		return defaultRefundAmount;
	}
}

enum SCR_ESupplieDeductionMethod
{
	FLAT,
	VALUE,
	PERCENTAGE_OF_MAX
}