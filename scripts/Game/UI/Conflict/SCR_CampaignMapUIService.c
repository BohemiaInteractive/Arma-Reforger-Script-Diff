//------------------------------------------------------------------------------
class SCR_CampaignMapUIService : SCR_CampaignMapUIElement
{
	protected SCR_CampaignMapUIBase m_ParentBase;

	protected SCR_EServicePointType m_eServiceType;

	protected bool m_bEnabled;
	protected string m_sServiceName;
	protected string m_sServiceIcon;
	protected string m_sServiceText;

	//------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		PlayHoverSound(m_sSoundService);

		ShowHint(true);
		if (m_ParentBase)
			m_ParentBase.m_bIsAnyElementHovered = true;

		return false;
	}

	//------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		ShowHint(false);
		if (m_ParentBase)
			m_ParentBase.m_bIsAnyElementHovered = false;

		return false;
	}

	//------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		SCR_MapEntity mapa = SCR_MapEntity.GetMapInstance();
		if (m_MapItem && m_MapItem.IsVisible()  && mapa)
		{
			vector pos = m_MapItem.GetPos();
			mapa.ZoomPanSmooth(10, pos[0], pos[2]);
		}

		return false;
	}

	//------------------------------------------------------------------------------
	override void AnimExpand()
	{
	}

	//------------------------------------------------------------------------------
	override void AnimCollapse()
	{
	}

	//------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_eIconType = SCR_EIconType.SERVICE;
	}

	//------------------------------------------------------------------------------
	void ShowHint(bool show)
	{
		if (m_ParentBase)
		{
			SCR_CampaignMilitaryBaseComponent base = m_ParentBase.GetBase();
			
			if (m_eServiceType == EEditableEntityLabel.SERVICE_SUPPLY_STORAGE && base)
			{
				Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
				if (base.GetFaction() == playerFaction)
				{
					int supplies = base.GetSupplies();
					int suppliesMax = base.GetSuppliesMax();

					m_sServiceText = "#AR-Campaign_BaseSuppliesHintAmount";
					m_ParentBase.ShowServiceHint(m_sServiceName, m_sServiceText, show, supplies, suppliesMax);
				}
				else
				{
					m_sServiceText = "#AR-Campaign_BaseSuppliesHintUnknown";
					m_ParentBase.ShowServiceHint(m_sServiceName, m_sServiceText, show);
				}
			}
			else
			{
				m_ParentBase.ShowServiceHint(m_sServiceName, m_sServiceText, show);
			}
		}
	}

	//------------------------------------------------------------------------------
	void SetServiceImage()
	{
		switch (m_eServiceType)
		{
			case SCR_EServicePointType.BARRACKS:
			{
				SetImage(m_sBarracks);
				m_sServiceName = "#AR-Campaign_Building_Barracks";
			} break;

			case SCR_EServicePointType.FIELD_HOSPITAL:
			{
				SetImage(m_sFieldHospital);
				m_sServiceName = "#AR-Campaign_Building_FieldHospital";
			} break;

			case SCR_EServicePointType.LIGHT_VEHICLE_DEPOT:
			{
				SetImage(m_sLightVehicleDepot);
				m_sServiceName = "#AR-Comm_Variable_Miscellaneous_LightVehicleDepot";
			} break;
			
			case SCR_EServicePointType.HEAVY_VEHICLE_DEPOT:
			{
				SetImage(m_sHeavyVehicleDepot);
				m_sServiceName = "#AR-Comm_Variable_Miscellaneous_HeavyVehicleDepot";
			} break;
			
			case SCR_EServicePointType.RADIO_ANTENNA:
			{
				SetImage(m_sRadioAntenna);
				m_sServiceName = "#AR-Comm_Variable_Miscellaneous_RadioAntenna";
			} break;

			case SCR_EServicePointType.SUPPLY_DEPOT:
			{
				SetImage(m_sSupplyDepot);
				m_sServiceName = "#AR-Comm_Variable_Miscellaneous_Supplydepot_US";
			} break;

			case SCR_EServicePointType.ARMORY:
			{
				SetImage(m_sArmory);
				m_sServiceName = "#AR-Campaign_Building_Armory";
			} break;

			case SCR_EServicePointType.FUEL_DEPOT:
			{
				SetImage(m_sFuelDepot);
				m_sServiceName = "#AR-Comm_Variable_Miscellaneous_Fueldepot_US";
			} break;

			case SCR_EServicePointType.HELIPAD:
			{
				SetImage(m_sHelipad);
				m_sServiceName = "#AR-EditableEntity_Helipad_L_US_01_Name";
			} break;

			/*case SCR_EServicePointType.VEHICLE_DEPOT:
			{
				SetImage(m_sVehicleDepot);
				m_sServiceName = "#AR-Campaign_Building_MotorPool";
			} break;*/
		}
	}

	//------------------------------------------------------------------------------
	void SetParent(SCR_CampaignMapUIBase parent)
	{
		m_ParentBase = parent;
	}

	//------------------------------------------------------------------------------
	void SetService(EEditableEntityLabel type, SCR_ServicePointDelegateComponent service)
	{
		m_bEnabled = service != null;
		m_eServiceType = type;
		SetServiceImage();
		
		if (!service)
			return;
		
		IEntity owner = service.GetOwner();
		
		if (!owner)
			return;
		
		SCR_ServicePointMapDescriptorComponent descr = SCR_ServicePointMapDescriptorComponent.Cast(owner.FindComponent(SCR_ServicePointMapDescriptorComponent));

		if (!descr)
		{
			IEntity compositionParent = SCR_EntityHelper.GetMainParent(owner, true);
			descr = SCR_ServicePointMapDescriptorComponent.Cast(compositionParent.FindComponent(SCR_ServicePointMapDescriptorComponent));
		}
		
		if (descr)
			m_MapItem = descr.Item();
	}

	//------------------------------------------------------------------------------
	override void SetImage(string image)
	{
		m_sServiceIcon = image;
		string suffix = string.Empty;
		
		SCR_ButtonImageComponent img = SCR_ButtonImageComponent.Cast(m_wRoot.FindHandler(SCR_ButtonImageComponent));
		if (img)
		{
			img.SetImage(m_sImageSet, image + suffix);
			img.SetEnabled(m_bEnabled);
		}
	}
};