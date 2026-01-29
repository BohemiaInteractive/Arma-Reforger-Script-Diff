[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_BallisticTableComponentClass : SCR_GadgetComponentClass
{
	[Attribute("{6E4CC0DD94FAB365}UI/layouts/Gadgets/BallisticTable/BallisticTable.layout", UIWidgets.ResourceNamePicker, "Layout name", "layout", category: "Layouts")]
	protected ResourceName m_sLayoutName;

	[Attribute("{F75FE2331AF70BF8}UI/layouts/Gadgets/BallisticTable/ContentRowLayout.layout", UIWidgets.ResourceNamePicker, "Layout name", "layout", category: "Layouts")]
	protected ResourceName m_sRowPrefab;

	[Attribute("{801F5CED215A1CFF}UI/layouts/Gadgets/BallisticTable/Content.layout", UIWidgets.ResourceNamePicker, "Layout name", "layout", category: "Layouts")]
	protected ResourceName m_sCellPrefab;

	[Attribute("0", desc: "Should value of 0 be replaced with '-' sign", category: "Layouts")]
	protected bool m_bReplaceZerosWithDash;

	[Attribute("0", desc: "Weights that will be applied to the grid of contents and thus impact how much space each cell will have in relation to other cells", category: "Layouts")]
	protected ref array<float> m_aGridFillWeights;

	[Attribute(desc: "List of pages available for this ballistic data", category: "Ballistics")]
	protected ref array<ref SCR_VisualisedBallisticConfig> m_aBallisticPages;

	[Attribute("SOUND_PICK_UP", desc: "Name of a sound event that will be played when page is changed")]
	protected string m_sPageTurnSoundEventName;

	[Attribute("%1 m", desc: "Format for displaying average dispersion.")]
	protected LocalizedString m_sAverageDispersionFormat;

	//------------------------------------------------------------------------------------------------
	ResourceName GetLayoutName()
	{
		return m_sLayoutName;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetCellPrefab()
	{
		return m_sCellPrefab;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetRowPrefab()
	{
		return m_sRowPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! If for this config values that area equal to zero should be replaced with a dash
	bool UseDashWhenZero()
	{
		return m_bReplaceZerosWithDash;
	}

	//------------------------------------------------------------------------------------------------
	int GetNumberOfColumns()
	{
		return m_aGridFillWeights.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] outWeights
	//! \return number of elements in returned array
	int GetWeights(notnull out array<float> outWeights)
	{
		return outWeights.Copy(m_aGridFillWeights);
	}

	//------------------------------------------------------------------------------------------------
	int GetNumberOfPages()
	{
		return m_aBallisticPages.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pageID position in the array of configs
	//! \return
	SCR_VisualisedBallisticConfig GetBallisticPage(int pageID)
	{
		if (!m_aBallisticPages.IsIndexValid(pageID))
			return null;

		return m_aBallisticPages[pageID];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] outPages
	//! \return number of ballistic pages that are returned
	int GetBallisticPages(notnull out array<SCR_VisualisedBallisticConfig> outPages)
	{
		outPages.Clear();
		foreach (SCR_VisualisedBallisticConfig page : m_aBallisticPages)
		{
			outPages.Insert(page);
		}

		return outPages.Count();
	}

	//------------------------------------------------------------------------------------------------
	string GetPageChangeSoundName()
	{
		return m_sPageTurnSoundEventName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetDispersionTextFormat()
	{
		return m_sAverageDispersionFormat;
	}
}

class SCR_BallisticTableComponent : SCR_GadgetComponent
{
	int m_iOpacityMapId;

	protected RTTextureWidget m_wRenderTargetTexture
	protected Widget m_wRootWidget;
	protected int m_iNumberOfPages;
	protected int m_iCurrentPageID;

	protected const string EMPTY_CELL_TEXT = "-";
	protected const string ROW_HOLDER_NAME = "ContentLayout";
	protected const string CONTENT_HOLDER_NAME = "ContentHolder";
	protected const string CONTENT_LABEL_NAME = "Label";
	protected const string TITLE_LABEL_NAME = "Title";
	protected const string AMMO_TYPE_NAME = "AmmoTypeIcon";
	protected const string PAGE_NUMBER_NAME = "PageNumber";
	protected const string DISPERSION_VALUE_CONTAINER_NAME = "DispersionValue";
	protected const string PAGE_NUMBER_FORMAT = "%1 / %2";
	protected const string STATIC_WIDGET_INDICATOR = "STATIC";

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfPages()
	{
		return m_iNumberOfPages;
	}

	//------------------------------------------------------------------------------------------------
	//! Interface for changing pages that are being displayed
	//! \param[in] direction -1 == previous | +1 == next
	void ChangePage(int direction = 0)
	{
		if (!m_wRenderTargetTexture)
			return;

		m_iCurrentPageID += direction;

		if (m_iCurrentPageID >= m_iNumberOfPages)
			m_iCurrentPageID = 0;

		if (m_iCurrentPageID < 0)
			m_iCurrentPageID = m_iNumberOfPages - 1;

		UpdateData();
	}

	//------------------------------------------------------------------------------------------------
	//! Interface for changing page to a different shell type
	//! \param[in] direction -1 == previous | +1 == next
	void ChangeShellType(int direction = 0)
	{
		if (direction == 0 || m_iNumberOfPages < 2)
			return;

		if (!m_wRenderTargetTexture)
			return;

		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		SCR_VisualisedBallisticConfig currentData = data.GetBallisticPage(m_iCurrentPageID);
		if (!currentData)
			return;

		int newDataId = m_iCurrentPageID + direction;
		SCR_VisualisedBallisticConfig replacementData;
		while (newDataId != m_iCurrentPageID)
		{
			if (newDataId >= m_iNumberOfPages)
				newDataId = 0;
			else if (newDataId < 0)
				newDataId = m_iNumberOfPages - 1;

			replacementData = data.GetBallisticPage(newDataId);
			if (replacementData && replacementData.GetProjectilePrefab() != currentData.GetProjectilePrefab())
				break;

			newDataId += direction;
		}

		if (newDataId == m_iCurrentPageID)
			return;

		m_iCurrentPageID = newDataId;
		UpdateData();
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to update currently displayed ballistic data
	protected void UpdateData()
	{
		m_wRenderTargetTexture.SetEnabled(true);
		ClearBallisticData();
		LoadBallisticData();
		m_wRenderTargetTexture.SetEnabled(false);

		const IEntity owner = GetOwner();
		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(owner));
		if (data)
			SCR_SoundManagerModule.CreateAndPlayAudioSource(owner, data.GetPageChangeSoundName());

		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl || !rpl.IsOwner())
			return;

		Rpc(RpcDo_SyncPageChange, m_iCurrentPageID);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to add widgets with ballistic data for currently selected page
	protected void LoadBallisticData()
	{
		if (!m_wRootWidget)
			return;

		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		Widget rowHolder = m_wRootWidget.FindAnyWidget(ROW_HOLDER_NAME);
		if (!rowHolder)
			return;

		SCR_VisualisedBallisticConfig pageData = data.GetBallisticPage(m_iCurrentPageID);
		if (!pageData)
			return;

		if (!pageData.IsGenerated())
			if (!pageData.GenerateBallisticData())
				return;

		SCR_BallisticData ballisticData = SCR_BallisticData.s_aBallistics[pageData.GetBallisticDataId()];
		if (!ballisticData)
			return;

		TextWidget title = TextWidget.Cast(m_wRootWidget.FindAnyWidget(TITLE_LABEL_NAME));
		if (title)
			title.SetText(pageData.GetDisplayedText());

		ImageWidget ammoTypeIcon = ImageWidget.Cast(m_wRootWidget.FindAnyWidget(AMMO_TYPE_NAME));
		if (ammoTypeIcon)
		{
			if (pageData.GetAmmoTypeQuadName().IsEmpty())
			{
				ammoTypeIcon.SetVisible(false);
			}
			else
			{
				ammoTypeIcon.SetVisible(true);
				ammoTypeIcon.LoadImageFromSet(0, pageData.GetAmmoTypeImageSet(), pageData.GetAmmoTypeQuadName());
				ammoTypeIcon.SetSize(pageData.GetAmmoTypeImageSize(), pageData.GetAmmoTypeImageSize());
			}
		}

		HorizontalLayoutWidget dispersionContainer = HorizontalLayoutWidget.Cast(m_wRootWidget.FindAnyWidget(DISPERSION_VALUE_CONTAINER_NAME));
		if (dispersionContainer)
		{
			TextWidget dispersionValue = TextWidget.Cast(dispersionContainer.FindAnyWidget(CONTENT_LABEL_NAME));
			if (dispersionValue)
				dispersionValue.SetTextFormat(data.GetDispersionTextFormat(), pageData.GetStandardDispersion().ToString(0, 0));
		}

		Widget contentHolder;
		array<float> values;
		string cellText;
		int id, lastEntry, numberOfValues, maxNumberOfColumns = data.GetNumberOfColumns();
		ballisticData.GetBallisticValuesForClosestRange(pageData.GetMaxRange(), lastEntry);
		ballisticData.GetBallisticValuesForClosestRange(pageData.GetMinRange(), id);

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
				if (valueId >= numberOfValues || float.AlmostEqual(values[valueId], 0) && data.UseDashWhenZero())
					cellText = EMPTY_CELL_TEXT;
				else if (valueId == 2)	//time of flight
					cellText = values[valueId].ToString(0, 1);
				else
					cellText = values[valueId].ToString();

				AddCell(contentHolder, cellText, valueId);
			}
		}

		TextWidget pageNumber = TextWidget.Cast(m_wRootWidget.FindAnyWidget(PAGE_NUMBER_NAME));
		if (pageNumber)
			pageNumber.SetText(string.Format(PAGE_NUMBER_FORMAT, (m_iCurrentPageID + 1), data.GetNumberOfPages()));
	}

	//------------------------------------------------------------------------------------------------
	//! Clears the content of the ROW_HOLDER_NAME from any non STATIC elements
	//! F.e. to be used when we will want to load different data when we change page
	void ClearBallisticData()
	{
		if (!m_wRootWidget)
			return;

		Widget rowHolder = m_wRootWidget.FindAnyWidget(ROW_HOLDER_NAME);
		if (!rowHolder)
			return;

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
		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return null;

		Widget layout = GetGame().GetWorkspace().CreateWidgets(data.GetRowPrefab(), parentWidget);
		if (!layout)
			return null;

		layout.SetZOrder(rowID);
		GridLayoutWidget contentHolder = GridLayoutWidget.Cast(layout.FindAnyWidget(CONTENT_HOLDER_NAME));
		if (!contentHolder)
		{
			layout.RemoveFromHierarchy();
			return null;
		}

		array<float> weights = {};
		for (int i, count = data.GetWeights(weights); i < count; i++)
		{
			contentHolder.SetColumnFillWeight(i, weights[i]);
		}

		return contentHolder;
	}

	//------------------------------------------------------------------------------------------------
	//! Meant to add a single cell
	protected Widget AddCell(notnull Widget parentWidget, string text, int columnID = 0)
	{
		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return null;

		Widget layout = GetGame().GetWorkspace().CreateWidgets(data.GetCellPrefab(), parentWidget);
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

	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.SPECIALIST_ITEM;
	}

	//------------------------------------------------------------------------------------------------
	protected override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);

		if (mode != EGadgetMode.IN_HAND)
			return;

		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		if (data.GetLayoutName() == ResourceName.Empty)
			return;

		m_wRootWidget = GetGame().GetWorkspace().CreateWidgets(data.GetLayoutName());
		if (!m_wRootWidget)
			return;

		m_wRenderTargetTexture = RTTextureWidget.Cast(m_wRootWidget.FindAnyWidget("RTTexture0"));
		if (!m_wRenderTargetTexture)
			return;

		m_wRenderTargetTexture.SetRenderTarget(GetOwner());
		m_wRenderTargetTexture.SetEnabled(false);

		ChangePage();
		m_iOpacityMapId = 1;//make render target visible
	}

	//------------------------------------------------------------------------------------------------
	protected override void ModeClear(EGadgetMode mode)
	{
		super.ModeClear(mode);

		if (mode == EGadgetMode.IN_HAND)
		{
			m_iOpacityMapId = 0;//make render target invisible
			ClearRenderTarget();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		m_iNumberOfPages = data.GetNumberOfPages();
		SCR_VisualisedBallisticConfig bp;
		for (int i; i < m_iNumberOfPages; i++)
		{
			bp = data.GetBallisticPage(i);
			if (!bp)
				continue;

			if (bp.IsGenerated())
				continue;

			bp.GenerateBallisticData();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		ClearRenderTarget();
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearRenderTarget()
	{
		if (m_wRootWidget)
			m_wRootWidget.RemoveFromHierarchy();

		m_wRootWidget = null;
		if (GetOwner().IsDeleted())
		{//in such case we cannot do any operations on entity or its VObject
			m_wRenderTargetTexture = null;
			return;
		}

		if (m_wRenderTargetTexture)
			m_wRenderTargetTexture.RemoveRenderTarget(GetOwner());

		m_wRenderTargetTexture = null;
	}
	//------------------------------------------------------------------------------------------------
	//! Authority method for replicating change of currently displayed page
	//! \param[in] selectedPage id of newly displayed page
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SyncPageChange(int selectedPage)
	{
		if (m_iCurrentPageID == selectedPage)
			return;

		m_iCurrentPageID = selectedPage;
		ChangePage();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iCurrentPageID);

		return super.RplSave(writer);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadInt(m_iCurrentPageID);
		ClearBallisticData();
		LoadBallisticData();

		return super.RplLoad(reader);
	}
}
