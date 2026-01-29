class SCR_ServicePointMapDescriptorComponentClass : SCR_MapDescriptorComponentClass
{
}

class SCR_ServicePointMapDescriptorComponent : SCR_MapDescriptorComponent
{
	[Attribute("", desc: "Name of marker (service is active).")]
	protected string m_sMarkerActive;
	
	[Attribute("", desc: "Name of marker (service is inctive).")]
	protected string m_sMarkerInactive;
	
	[Attribute("Default Name", desc: "Name of service to be shown on hover.")]
	protected string m_sMarkerName;
	
	[Attribute("Default Description", desc: "Description of service to be shown on hover.")]
	protected string m_sMarkerDesc;
	
	[Attribute("{94F1E2223D7E0588}UI/layouts/Campaign/ServiceHint.layout")]
	protected ResourceName m_sDescriptorServiceHint;
	
	protected string m_sMarkerSmall = "Slot_Small";
	
	protected Widget m_wDescriptorServiceHint;
	
	protected SCR_MilitaryBaseComponent m_Base;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	//! \param[in] duringInit
	void SetParentBase(SCR_MilitaryBaseComponent base, bool duringInit = false)
	{
		m_Base = base;
		
		if (!m_Base || m_Base.GetFaction() != SCR_FactionManager.SGetLocalPlayerFaction() || duringInit)
			SetServiceMarker(m_Base.GetFaction(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \param[in] visible
	void SetServiceMarker(Faction faction = null, bool visible = true)
	{
		MapItem item = Item();

		if (!item)
			return; 
		
		if (visible)
		{
			item.SetVisible(true);
		}
		else
		{
			item.SetVisible(false);
			return;
		}
		
		if (!faction)
			return;

		MapDescriptorProps props = item.GetProps();

		props.SetFrontColor(faction.GetFactionColor());
		props.SetTextVisible(false);
		props.Activate(true);
		
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		
		if (mapEntity)
			SetVisible(mapEntity.GetLayerIndex());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] active
	void SetServiceMarkerActive(bool active)
	{
		MapItem item = Item();
		
		if (!item)
			return; 
		
		string marker;
		
		if (active)
			marker = m_sMarkerActive;
		else
			marker = m_sMarkerInactive;
		
		item.SetImageDef(marker);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] layer
	void SetVisible(int layer)
	{
		if (!m_Base)
			return;

		MapItem item = Item();
		MapDescriptorProps props = item.GetProps();

		if (layer == 0)
		{
			item.SetImageDef(m_sMarkerActive);
			props.SetIconSize(32, 0.3, 0.3);
		}
		else
		{
			item.SetImageDef(m_sMarkerSmall);
			props.SetIconSize(32, 0.1, 0.1);
		}
		
		// TODO: Get rid of Campaign dependency once radio signal range is a sandbox feature
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(m_Base);
		
		if (!campaignBase)
			return;
		
		SCR_CampaignFaction f = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		
		if (!f)
			return;
		
		if (campaignBase.IsHQRadioTrafficPossible(f))
			Item().GetProps().SetVisible(layer < 3);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] show
	void ShowServiceHint(MapItem show)
	{	
		MapItem item = Item();
		if (show != item)
			return;
		
		int mx;
		int my;
		WidgetManager.GetMousePos(mx, my);

		mx = GetGame().GetWorkspace().DPIUnscale(mx);
		my = GetGame().GetWorkspace().DPIUnscale(my);

		if (!m_wDescriptorServiceHint)
		{
			SCR_MapEntity entity = SCR_MapEntity.GetMapInstance();
			m_wDescriptorServiceHint = entity.GetMapMenuRoot().FindAnyWidget("ServiceHintRoot");
		}
		
		if (!m_wDescriptorServiceHint)
			return;

		if (show)
		{
			FrameSlot.SetPos(m_wDescriptorServiceHint, mx, my);
			TextWidget.Cast(m_wDescriptorServiceHint.FindAnyWidget("ServiceName")).SetTextFormat(m_sMarkerName);
			Widget resupplyText = m_wDescriptorServiceHint.FindAnyWidget("ResupplyText");
			if (resupplyText)
				resupplyText.SetVisible(false);
			
			TextWidget serviceText = TextWidget.Cast(m_wDescriptorServiceHint.FindAnyWidget("ServiceText"));
			if (serviceText)
			{
				if (m_sMarkerDesc != "Default Description")
				{
					serviceText.SetTextFormat(m_sMarkerDesc);
					serviceText.SetVisible(true);
				}
				else
				{
					serviceText.SetVisible(false);
				}
			}
		}

		m_wDescriptorServiceHint.SetVisible(true)
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] show
	void HideServiceHint(MapItem show)
	{
		MapItem item = Item();
		if (show != item)
			return;
		
		if (m_wDescriptorServiceHint)
			m_wDescriptorServiceHint.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ServicePointMapDescriptorComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SCR_MapEntity.GetOnLayerChanged().Insert(SetVisible);
		SCR_MapEntity.GetOnHoverItem().Insert(ShowServiceHint);
		SCR_MapEntity.GetOnHoverEnd().Insert(HideServiceHint);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_ServicePointMapDescriptorComponent()
	{
		SCR_MapEntity.GetOnLayerChanged().Remove(SetVisible);
		SCR_MapEntity.GetOnHoverItem().Remove(ShowServiceHint);
		SCR_MapEntity.GetOnHoverEnd().Remove(HideServiceHint);
	}
}
