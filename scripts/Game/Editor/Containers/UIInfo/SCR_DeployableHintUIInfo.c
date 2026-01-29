[BaseContainerProps(configRoot: true)]
class SCR_DeployableHintUIInfo : SCR_InventoryItemHintUIInfo
{
	[Attribute(defvalue: "0", desc: "Id of a variant for which this hint should fetch the list of required items", params: "0 inf")]
	protected int m_iVariantId;

	[Attribute(defvalue: "0", desc: "Never show this hint")]
	protected bool m_bHide;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		return !m_bHide && super.CanBeShown(item, focusedSlot);
	}

	//------------------------------------------------------------------------------------------------
	override bool SetItemHintNameTo(InventoryItemComponent item, TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		textWidget.SetTextWrapping(false);
		return super.SetItemHintNameTo(item, textWidget);
	}

	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		IEntity owner = item.GetOwner();
		SCR_MultiPartDeployableItemComponent deployableComp = SCR_MultiPartDeployableItemComponent.Cast(owner.FindComponent(SCR_MultiPartDeployableItemComponent));
		if (!deployableComp)
			return string.Empty;

		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(deployableComp.GetComponentData(owner));
		if (!data)
			return string.Empty;

		SCR_DeployableVariantContainer container = data.GetVariantContainer(m_iVariantId);
		if (!container)
			string.Empty;

		array<ref SCR_RequiredDeployablePart> requirements = container.GetRequiredElementsRaw();
		if (!requirements || requirements.IsEmpty())
			return string.Empty;

		int numberOfParts;
		string element;
		foreach(int i, SCR_RequiredDeployablePart partReq : requirements)
		{
			element += UIConstants.RICH_TEXT_LINE_BREAK + UIConstants.RICH_TEXT_LIST_POINT + " " + partReq.GetPartName();
			numberOfParts = partReq.GetNumberOfRequiredPrefabs();
			if (numberOfParts > 1)
				element += WidgetManager.Translate(" " + UIConstants.VALUE_MUTLIPLIER_SHORT, numberOfParts.ToString());
		}

		return WidgetManager.Translate(GetName(), GetDescription(), element);
	}
}