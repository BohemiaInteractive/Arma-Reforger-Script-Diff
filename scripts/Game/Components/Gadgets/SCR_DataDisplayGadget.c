class SCR_DataDisplayGadgetComponentClass : SCR_GadgetComponentClass
{
	[Attribute("{6A90C4A4FEAE57A5}UI/layouts/Gadgets/BallisticTable/BallisticTableRenderTarget.layout", UIWidgets.ResourceNamePicker, "Layout name", "layout", category: "Layouts")]
	protected ResourceName m_sLayoutName;

	[Attribute(desc: "List of pages available for this ballistic data", category: "Ballistics")]
	protected ref array<ref SCR_BaseDataPageConfig> m_aPages;

	[Attribute("SOUND_PICK_UP", desc: "Name of a sound event that will be played when page is changed")]
	protected string m_sPageTurnSoundEventName;

	//------------------------------------------------------------------------------------------------
	ResourceName GetLayoutName()
	{
		return m_sLayoutName;
	}

	//------------------------------------------------------------------------------------------------
	string GetPageChangeSoundName()
	{
		return m_sPageTurnSoundEventName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetNumberOfPages()
	{
		return m_aPages.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pageID position in the array of configs
	//! \return
	SCR_BaseDataPageConfig GetPage(int pageID)
	{
		if (!m_aPages.IsIndexValid(pageID))
			return null;

		return m_aPages[pageID];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] outPages
	//! \return number of ballistic pages that are returned
	int GetPages(notnull out array<SCR_BaseDataPageConfig> outPages)
	{
		outPages.Clear();
		foreach (SCR_BaseDataPageConfig page : m_aPages)
		{
			outPages.Insert(page);
		}

		return outPages.Count();
	}
}

class SCR_DataDisplayGadgetComponent : SCR_GadgetComponent
{
	int m_iOpacityMapId;

	protected RTTextureWidget m_wRenderTargetTexture
	protected Widget m_wRootWidget;
	protected int m_iNumberOfPages;
	protected int m_iCurrentPageID;

	const string PAGE_NUMBER_NAME = "PageNumber";
	const string PAGE_NUMBER_FORMAT = "%1 / %2";

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
	//! Method used to update currently displayed ballistic data
	protected void UpdateData(bool playSound = true)
	{
		if (!m_wRootWidget)
			return;

		const IEntity owner = GetOwner();
		SCR_DataDisplayGadgetComponentClass data = SCR_DataDisplayGadgetComponentClass.Cast(GetComponentData(owner));
		SCR_BaseDataPageConfig page = data.GetPage(m_iCurrentPageID);
		if (!page)
			return;

		Widget contentHolder = m_wRootWidget.GetChildren();
		if (!contentHolder)
			return;

		m_wRenderTargetTexture.SetEnabled(true);
		page.DrawPageData(contentHolder, owner, this, m_iCurrentPageID);
		m_wRenderTargetTexture.SetEnabled(false);

		if (playSound)
			SCR_SoundManagerModule.CreateAndPlayAudioSource(owner, data.GetPageChangeSoundName());

		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl || !rpl.IsOwner())
			return;

		Rpc(RpcDo_SyncPageChange, m_iCurrentPageID);
	}

	//------------------------------------------------------------------------------------------------
	protected override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);

		if (mode != EGadgetMode.IN_HAND)
			return;

		if (!InitializeRenderTarget())
			return;

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
	protected bool InitializeRenderTarget()
	{
		IEntity owner = GetOwner();
		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return false;

		if (data.GetLayoutName() == ResourceName.Empty)
			return false;

		m_wRootWidget = GetGame().GetWorkspace().CreateWidgets(data.GetLayoutName());
		if (!m_wRootWidget)
			return false;

		m_wRenderTargetTexture = RTTextureWidget.Cast(m_wRootWidget.FindAnyWidget("RTTexture0"));
		if (!m_wRenderTargetTexture)
			return false;

		m_wRenderTargetTexture.SetRenderTarget(owner);
		m_wRenderTargetTexture.SetEnabled(false);
		return true;
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
	override EGadgetType GetType()
	{
		return EGadgetType.SPECIALIST_ITEM;
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		SCR_BallisticTableComponentClass data = SCR_BallisticTableComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		m_iNumberOfPages = data.GetNumberOfPages();
		SCR_VisualisedBallisticConfig ballisticPage;
		for (int i; i < m_iNumberOfPages; i++)
		{
			ballisticPage = SCR_VisualisedBallisticConfig.Cast(data.GetPage(i));
			if (!ballisticPage)
				continue;

			if (ballisticPage.IsGenerated())
				continue;

			ballisticPage.GenerateBallisticData();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		ClearRenderTarget();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority method for replicating change of currently displayed page
	//! \param[in] selectedPage id of newly displayed page
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SyncPageChange(int selectedPage)
	{
		if (m_iCurrentPageID == selectedPage)
			return;

		m_iCurrentPageID = selectedPage;
		ChangePage();
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iCurrentPageID);

		return super.RplSave(writer);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadInt(m_iCurrentPageID);
		UpdateData(false);

		return super.RplLoad(reader);
	}
}