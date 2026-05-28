class SCR_VisualisedBallisticConfig : SCR_BaseBallisticConfig
{
	[Attribute("{6E4CC0DD94FAB365}UI/layouts/Gadgets/BallisticTable/BallisticTable.layout", UIWidgets.ResourceNamePicker, "Layout name", "layout", category: "Layouts")]
	protected ResourceName m_sLayoutName;

	[Attribute("{F75FE2331AF70BF8}UI/layouts/Gadgets/BallisticTable/ContentRowLayout.layout", UIWidgets.ResourceNamePicker, "Layout name", "layout", category: "Layouts")]
	protected ResourceName m_sRowPrefab;

	[Attribute("{801F5CED215A1CFF}UI/layouts/Gadgets/BallisticTable/Content.layout", UIWidgets.ResourceNamePicker, "Layout name", "layout", category: "Layouts")]
	protected ResourceName m_sCellPrefab;

	[Attribute(desc: "Lowest range for which data will be shown", params: "0 inf")]
	protected int m_iMinRange;

	[Attribute(desc: "Highest range for which data will be shown", params: "0 inf")]
	protected int m_iMaxRange;

	[Attribute(desc: "Change in elevation down range", params: "1 inf")]
	protected int m_iElevationChangeDownRange;

	[Attribute("0", desc: "Should value of 0 be replaced with '-' sign", category: "Layouts")]
	protected bool m_bReplaceZerosWithDash;

	[Attribute("0", desc: "Weights that will be applied to the grid of contents and thus impact how much space each cell will have in relation to other cells", category: "Layouts")]
	protected ref array<float> m_aGridFillWeights;

	[Attribute("%1 m", desc: "Format for displaying average dispersion.")]
	protected LocalizedString m_sAverageDispersionFormat;

	protected bool m_bDataExtensionCompleted;

	const string EMPTY_CELL_TEXT = "-";
	const string ROW_HOLDER_NAME = "BallisticContentLayout";
	const string CONTENT_HOLDER_NAME = "ContentHolder";
	const string CONTENT_LABEL_NAME = "Label";
	const string TITLE_LABEL_NAME = "Title";
	const string AMMO_TYPE_NAME = "AmmoTypeIcon";
	const string DISPERSION_VALUE_CONTAINER_NAME = "DispersionValue";
	const string STATIC_WIDGET_INDICATOR = "STATIC";

	//------------------------------------------------------------------------------------------------
	//! Check if such data doesnt already exist and only then it will generate values for this config
	//! \return if data for this config is available in SCR_BallisticData.s_aBallistics
	override bool GenerateBallisticData()
	{
		if (!super.GenerateBallisticData())
			return false;

		if (m_bDataExtensionCompleted)
			return true;

		SCR_BallisticData ballisticData = SCR_BallisticData.s_aBallistics[m_iBallisticDataId];
		if (!ballisticData)
			return false;

		int numberOfEntries = ballisticData.GetNumberOfEntries();
		if (numberOfEntries < 1)
			return false;

		Resource resource = Resource.Load(m_sProjectilePrefab);
		if (!resource.IsValid())
			return false;

		BaseResourceObject resourceObj = resource.GetResource();
		if (!resourceObj)
			return false;

		IEntitySource entitySrc = resourceObj.ToEntitySource();
		if (!entitySrc)
			return false;

		SCR_ProjectileWindTable windageData = SCR_ProjectileWindTable.GetProjectileWindageData(entitySrc);
		if (!windageData)
			return false;

		array<float> values;
		array<float> windageValues;
		float distance, travelTime, aimHeight, angle, angleRad;
		for (int i; i < numberOfEntries; i++)
		{
			ballisticData.GetValues(i, values);
			if (!values || values.IsEmpty())
				continue;

			distance = values[0] + m_iElevationChangeDownRange * 0.5;
			aimHeight = BallisticTable.GetHeightFromProjectileSource(distance, travelTime, entitySrc, m_fProjectileInitSpeedCoef, m_bDirectFireMode);
			angleRad = SCR_Math.ConvertToRadians(values[1], m_eUnitType);

			if (travelTime > 0 && aimHeight > 0)
			{
				angle = Math.Atan2(aimHeight, distance); //radians
				angle = SCR_Math.ConvertFromRadians(angle, m_eUnitType);
				angle = Math.Round(Math.AbsFloat(angle - values[1]));
				travelTime = Math.AbsFloat(travelTime - values[2]);
				travelTime = travelTime.ToString(0,1).ToFloat();
			}
			else
			{
				angle = 0;
				travelTime = 0;
			}

			values.Insert(angle);
			values.Insert(travelTime);

			windageValues = windageData.GetDataByAngle(angleRad, initSpeedCoef: m_fProjectileInitSpeedCoef);
			if (windageValues && !windageValues.IsEmpty())
			{
				values.Insert(Math.Round(windageValues[2]));
				values.Insert(Math.Round(windageValues[5] * 10) * 0.1);
				values.Insert(Math.Round(SCR_Math.ConvertFromRadians(windageValues[3] * 0.001, m_eUnitType)));
				values.Insert(Math.Round(windageValues[4]));
			}

			ballisticData.SetValues(i, values);
		}

		m_bDataExtensionCompleted = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void DrawPageData(notnull Widget root, notnull IEntity surfaceEntity, notnull SCR_DataDisplayGadgetComponent drawingGadget, int pageID)
	{
		ClearDataPage(root);
		LoadDataPage(root, drawingGadget, pageID);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to add widgets with ballistic data for currently selected page
	protected void LoadDataPage(notnull Widget root, notnull SCR_DataDisplayGadgetComponent drawingGadget, int pageID)
	{
		Widget rowHolder = root.FindAnyWidget(ROW_HOLDER_NAME);
		if (!rowHolder)
			return;

		if (!IsGenerated())
			if (!GenerateBallisticData())
				return;

		SCR_BallisticData ballisticData = SCR_BallisticData.s_aBallistics[m_iBallisticDataId];
		if (!ballisticData)
			return;

		TextWidget title = TextWidget.Cast(root.FindAnyWidget(TITLE_LABEL_NAME));
		if (title)
			title.SetText(m_sDisplayedText);

		ImageWidget ammoTypeIcon = ImageWidget.Cast(root.FindAnyWidget(AMMO_TYPE_NAME));
		if (ammoTypeIcon)
		{
			if (m_sAmmoTypeQuadName.IsEmpty())
			{
				ammoTypeIcon.SetVisible(false);
			}
			else
			{
				ammoTypeIcon.SetVisible(true);
				ammoTypeIcon.LoadImageFromSet(0, m_sAmmoTypeImageSet, m_sAmmoTypeQuadName);
				ammoTypeIcon.SetSize(m_iAmmoTypeImageSize, m_iAmmoTypeImageSize);
			}
		}

		HorizontalLayoutWidget dispersionContainer = HorizontalLayoutWidget.Cast(root.FindAnyWidget(DISPERSION_VALUE_CONTAINER_NAME));
		if (dispersionContainer)
		{
			TextWidget dispersionValue = TextWidget.Cast(dispersionContainer.FindAnyWidget(CONTENT_LABEL_NAME));
			if (dispersionValue)
				dispersionValue.SetTextFormat(m_sAverageDispersionFormat, m_fStandardDispersion.ToString(0, 0));
		}

		Widget contentHolder;
		array<float> values;
		string cellText;
		int id, lastEntry, numberOfValues, maxNumberOfColumns = m_aGridFillWeights.Count();
		ballisticData.GetBallisticValuesForClosestRange(m_iMaxRange, lastEntry);
		ballisticData.GetBallisticValuesForClosestRange(m_iMinRange, id);

		for (int i = id; i <= lastEntry; i++)
		{
			contentHolder = AddRow(rowHolder, i);
			if (!contentHolder)
				continue;

			numberOfValues = ballisticData.GetValues(i, values);
			if (numberOfValues < 1)
				continue;

			for (int valueId; valueId < maxNumberOfColumns; valueId++)
			{
				if (valueId >= numberOfValues || m_bReplaceZerosWithDash && float.AlmostEqual(values[valueId], 0))
					cellText = EMPTY_CELL_TEXT;
				else if (valueId == 2)	//time of flight
					cellText = values[valueId].ToString(0, 1);
				else
					cellText = values[valueId].ToString();

				AddCell(contentHolder, cellText, valueId);
			}
		}

		TextWidget pageNumber = TextWidget.Cast(root.FindAnyWidget(drawingGadget.PAGE_NUMBER_NAME));
		if (pageNumber)
			pageNumber.SetText(string.Format(drawingGadget.PAGE_NUMBER_FORMAT, (pageID + 1), drawingGadget.GetNumberOfPages()));
	}

	//------------------------------------------------------------------------------------------------
	//! Clears the content of the ROW_HOLDER_NAME from any non STATIC elements
	//! F.e. to be used when we will want to load different data when we change page
	protected void ClearDataPage(notnull Widget root)
	{
		Widget rowHolder = root.FindAnyWidget(ROW_HOLDER_NAME);
		if (!rowHolder)
		{
			Widget contentContaier = root.GetChildren();
			if (contentContaier)
			{
				contentContaier.RemoveFromHierarchy();
				delete contentContaier;
			}

			contentContaier = GetGame().GetWorkspace().CreateWidgets(m_sLayoutName, root);
			if (!contentContaier)
				return;

			rowHolder = contentContaier.FindAnyWidget(ROW_HOLDER_NAME);
			if (!rowHolder)
				return;
		}

		Widget child = rowHolder.FindAnyWidgetById(0);
		Widget nextChild;
		while (child)
		{
			nextChild = child.GetSibling();
			if (!child.GetName().Contains(STATIC_WIDGET_INDICATOR))
				child.RemoveFromHierarchy();

			child = nextChild;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Meant to add a single row
	protected Widget AddRow(notnull Widget parentWidget, int rowID = 0)
	{
		Widget layout = GetGame().GetWorkspace().CreateWidgets(m_sRowPrefab, parentWidget);
		if (!layout)
			return null;

		layout.SetZOrder(rowID);
		GridLayoutWidget contentHolder = GridLayoutWidget.Cast(layout.FindAnyWidget(CONTENT_HOLDER_NAME));
		if (!contentHolder)
		{
			layout.RemoveFromHierarchy();
			return null;
		}

		foreach (int i, float weight : m_aGridFillWeights)
		{
			contentHolder.SetColumnFillWeight(i, weight);
		}

		return contentHolder;
	}

	//------------------------------------------------------------------------------------------------
	//! Meant to add a single cell
	protected Widget AddCell(notnull Widget parentWidget, string text, int columnID = 0)
	{
		Widget layout = GetGame().GetWorkspace().CreateWidgets(m_sCellPrefab, parentWidget);
		if (!layout)
			return null;

		GridSlot.SetColumn(layout, columnID);
		TextWidget textWidget = TextWidget.Cast(layout.FindAnyWidget(CONTENT_LABEL_NAME));
		if (!textWidget)
		{
			layout.RemoveFromHierarchy();
			return null;
		}

		textWidget.SetText(text);
		return textWidget;
	}
}
