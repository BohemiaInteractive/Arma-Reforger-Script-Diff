class SCR_StaticDataPageConfig : SCR_BaseDataPageConfig
{
	[Attribute("{21EBD81BCE6E763B}UI/layouts/Gadgets/BallisticTable/BallisticTable_US_MortarManual.layout", UIWidgets.ResourceNamePicker, "Layout name", "layout", category: "Layouts")]
	protected ResourceName m_sLayoutName;

	//------------------------------------------------------------------------------------------------
	override void DrawPageData(notnull Widget root, notnull IEntity surfaceEntity, notnull SCR_DataDisplayGadgetComponent drawingGadget, int pageID)
	{
		Widget content = root.GetChildren();
		if (content)
		{
			content.RemoveFromHierarchy();
			delete content;
		}

		content = GetGame().GetWorkspace().CreateWidgets(m_sLayoutName, root);
		if (!content)
			return;

		TextWidget pageNumber = TextWidget.Cast(root.FindAnyWidget(drawingGadget.PAGE_NUMBER_NAME));
		if (pageNumber)
			pageNumber.SetText(string.Format(drawingGadget.PAGE_NUMBER_FORMAT, (pageID + 1), drawingGadget.GetNumberOfPages()));
	}
}