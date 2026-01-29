class SCR_TutorialMapUIBase : SCR_CampaignMapUIElement
{
	protected SizeLayoutWidget m_wImageOverlay;
	protected Widget m_wBaseFrame;
	protected Widget m_wBaseIcon;
	protected Widget m_wBaseOverlay;
	protected Widget m_wInfoOverlay;
	protected Widget m_wServices;
	protected Widget m_wSymbolWidget
	protected TextWidget m_w_NameDialog;
	protected static Widget m_wServiceHint;
	protected Widget m_w_ServicesOverlay;
	protected ImageWidget m_wLocalTask;
	protected TextWidget m_wBaseName;
	protected TextWidget m_wCallsignName;
	protected TextWidget m_wBaseNameDialog;
	protected TextWidget m_wCallsignNameDialog;
	protected TextWidget m_wSuppliesText
	protected Widget m_wInfoText;
	protected Widget m_wAntennaImg;
	
	protected bool m_bCanRespawn;
	protected bool m_bCanPlaySounds = true;
	
	SCR_MilitarySymbolUIComponent m_MilitarySymbol;
	
	SCR_TutorialFakeBaseComponent m_FakeBaseComp;
	
	static ref ScriptInvoker Event_OnIconUpdated = new ScriptInvoker();

	protected ref ScriptInvoker m_OnBaseSelected = new ScriptInvoker();
	protected ref ScriptInvoker m_OnMapIconEnter;
	protected ref ScriptInvoker m_OnMapIconClick;
	
	//------------------------------------------------------------------------------------------------
	void UpdateMilitarySymbol()
	{
		if (!m_MilitarySymbol || !m_FakeBaseComp)
			return;
		
		SCR_MilitarySymbol symbol = new SCR_MilitarySymbol;
		symbol.SetIdentity(m_FakeBaseComp.m_eMilitarySymbolIdentity);
		symbol.SetDimension(m_FakeBaseComp.m_eMilitarySymbolDimension);
		symbol.SetIcons(m_FakeBaseComp.m_eMilitarySymbolIcon);
		symbol.SetAmplifier(m_FakeBaseComp.m_eMilitarySymbolAplifier);
		
		m_MilitarySymbol.Update(symbol);
		
		m_wBaseName.SetText(m_FakeBaseComp.m_sBaseName);
		m_wCallsignName.SetText(m_FakeBaseComp.m_sBaseCallSign);
		m_wBaseNameDialog.SetText(m_FakeBaseComp.m_sBaseNameLonger);
		m_wBaseNameDialog.SetColor(m_FakeBaseComp.m_BaseColor);
		m_wSymbolWidget.SetColor(m_FakeBaseComp.m_BaseColor);
		
		m_wImageOverlay.SetWidthOverride(m_FakeBaseComp.m_iWidthOverride);
		m_wImageOverlay.SetHeightOverride(m_FakeBaseComp.m_iHeightOverride);
		
		m_wAntennaImg.SetVisible(m_FakeBaseComp.m_bAntennaWidgetVisible);
		
		m_wServices.SetVisible(m_FakeBaseComp.m_bAllowServices);
		m_w_ServicesOverlay.SetVisible(m_FakeBaseComp.m_bAllowServicesSizeOverlay);
		if (m_FakeBaseComp.m_bAllowServicesSizeOverlay)
			InitServices();
		
		m_wHighlightImg.LoadImageFromSet(0, m_sImageSetARO, m_FakeBaseComp.m_sHighlight);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void InitServices()
	{
		Widget w = m_wServices.FindAnyWidget("ServicesSizeOverlay");
		if (!w)
			return;

		m_wServices.SetVisible(false);
		m_wServices.SetOpacity(0);

		for (int serviceId = 0; serviceId < 8; ++serviceId)
		{
			string btnName = "Service" + serviceId.ToString();
			Widget serviceBtn = w.FindAnyWidget(btnName);
			serviceBtn.SetOpacity(0.5);
			
			SCR_CampaignMapUIService handler = SCR_CampaignMapUIService.Cast(serviceBtn.FindHandler(SCR_CampaignMapUIService));
			if (handler)
			{
				switch (serviceId)
				{
					case 0:
					{
						handler.SetImage(m_sArmory);
						if (SCR_Enum.HasFlag(m_FakeBaseComp.m_eServices, SCR_EFakeBaseServices.ARMORY))
							serviceBtn.SetOpacity(1);
						
						break;
					}
					
					case 1:
					{
						handler.SetImage(m_sHelipad);
						if (SCR_Enum.HasFlag(m_FakeBaseComp.m_eServices, SCR_EFakeBaseServices.HELIPAD))
							serviceBtn.SetOpacity(1);
						
						break;
					}
					
					case 2:
					{
						handler.SetImage(m_sBarracks);
						if (SCR_Enum.HasFlag(m_FakeBaseComp.m_eServices, SCR_EFakeBaseServices.BARRACKS))
							serviceBtn.SetOpacity(1);
						
						break;
					}
					
					case 3:
					{
						handler.SetImage(m_sFuelDepot);
						if (SCR_Enum.HasFlag(m_FakeBaseComp.m_eServices, SCR_EFakeBaseServices.FUEL_DEPOT))
							serviceBtn.SetOpacity(1);
						
						break;
					}
					
					case 4:
					{
						handler.SetImage(m_sRadioAntenna);
						if (SCR_Enum.HasFlag(m_FakeBaseComp.m_eServices, SCR_EFakeBaseServices.RADIO_ANTENNA))
							serviceBtn.SetOpacity(1);
						
						break;
					}
					
					case 5:
					{
						handler.SetImage(m_sLightVehicleDepot);
						if (SCR_Enum.HasFlag(m_FakeBaseComp.m_eServices, SCR_EFakeBaseServices.LIGHT_VEHICLE_DEPOT))
							serviceBtn.SetOpacity(1);
						
						break;
					}
					
					case 6:
					{
						handler.SetImage(m_sHeavyVehicleDepot);
						if (SCR_Enum.HasFlag(m_FakeBaseComp.m_eServices, SCR_EFakeBaseServices.HEAVY_VEHICLE_DEPOT))
							serviceBtn.SetOpacity(1);
						
						break;
					}
					
					case 7:
					{
						handler.SetImage(m_sFieldHospital);
						if (SCR_Enum.HasFlag(m_FakeBaseComp.m_eServices, SCR_EFakeBaseServices.FIELD_HOSPITAL))
							serviceBtn.SetOpacity(1);
						
						break;
					}
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFakeBase(SCR_TutorialFakeBaseComponent fakeBase)
	{
		m_FakeBaseComp = fakeBase;
	}
	
	//------------------------------------------------------------------------------------------------
	override vector GetPos()
	{
		if (m_FakeBaseComp)
			return m_FakeBaseComp.GetOwner().GetOrigin();
		
		return vector.Zero;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetImage(string image)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SCR_TaskManagerUIComponent tm = SCR_TaskManagerUIComponent.GetInstance();
		if (tm && !tm.IsTaskListOpen())
		{
			GetGame().GetWorkspace().SetFocusedWidget(w);
		}

		if (m_OnMapIconEnter)
			m_OnMapIconEnter.Invoke();

		super.OnMouseEnter(w, x, y);

		if (w.Type() == ButtonWidget)
			AnimExpand();
		
		/*if (m_Base)
		{
			m_Base.GetMapDescriptor().OnIconHovered(this, true);
		*/
		if (m_wServices)
		{
			m_wServices.SetVisible(true);
			m_wServices.SetEnabled(true);
		}
		//}
		
		if (m_wInfoText)
		{
			m_wInfoText.SetVisible(true);
			m_wRoot.SetZOrder(1);
		}
		
		/*if (m_MobileAssembly){
			m_MobileAssembly.OnIconHovered(this, true);
			m_wServices.SetVisible(true);
			m_w_ServicesOverlay.SetVisible(false);
			m_wRoot.SetZOrder(1);
		}*/

		m_FakeBaseComp.ColorLinks(true);
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);

		AnimCollapse();
		
		if(m_wInfoText)
		{
		m_wInfoText.SetVisible(false);
		m_wRoot.SetZOrder(0);
		}
		
		//if (m_Base)
		//	m_Base.GetMapDescriptor().OnIconHovered(this, false);

		/*if (m_MobileAssembly)
			m_MobileAssembly.OnIconHovered(this, false);
			m_wInfoText.SetVisible(false);

		if (!m_bCanRespawn && m_bIsRespawnMenu)
			return false;
		*/
		
		if (m_wServices)
			m_wServices.SetEnabled(false);

		//if (m_wLocalTask.IsEnabled())
		//	m_wLocalTask.SetVisible(true);

		if (enterW)
			m_bCanPlaySounds = w.FindHandler(SCR_CampaignMapUIService) == null;
		else
			m_bCanPlaySounds = true;

		if (RenderTargetWidget.Cast(enterW) && m_wBaseOverlay.IsEnabled())
		{
			m_wBaseOverlay.SetEnabled(false); // disable the base widget when not hovered, deactivating the button
			m_bCanPlaySounds = true;
		}

		m_FakeBaseComp.ColorLinks(false);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void AnimCollapse()
	{
		m_wHighlightImg.SetVisible(false);
		if (m_wGradient && !m_bIsSelected)
			m_wGradient.SetVisible(false);

		if (m_wServices)
		{
			AnimateWidget.Opacity(m_wServices, 0, ANIM_SPEED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void AnimExpand()
	{
		/*if (!m_bIsAnyElementHovered && m_bCanPlaySounds)
		{
			switch (m_eIconType)
			{
				case SCR_EIconType.NONE:
				{
				} break;

				case SCR_EIconType.BASE:
				{
					if (!m_bIsAnyElementClicked)
						PlayHoverSound(m_sSoundBase);
				} break;

				case SCR_EIconType.ENEMY_BASE:
				{
					PlayHoverSound(m_sSoundEnemyBase);
				} break;

				case SCR_EIconType.RELAY:
				{
					PlayHoverSound(m_sSoundRelay);
				} break;
			}
		}*/

		//int paddingLeft = 0;
		//int paddingRight = 0;
		//int paddingBottom = 0;
		int expand = -5;
		/*if (m_mTasks.IsEmpty())
			paddingRight = 0;
		if (m_mServices.IsEmpty())
			paddingLeft = 0;
		*/
		
		if (m_wServices)
		{
			AnimateWidget.Opacity(m_wServices, 1, ANIM_SPEED);
		}

		m_wHighlightImg.SetVisible(true);
		if (m_wGradient)
			m_wGradient.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wSymbolWidget = w.FindAnyWidget("SideSymbol");
		if (m_wSymbolWidget)
			m_MilitarySymbol = SCR_MilitarySymbolUIComponent.Cast(m_wSymbolWidget.FindHandler(SCR_MilitarySymbolUIComponent));
		
		m_wImageOverlay = SizeLayoutWidget.Cast(w.FindAnyWidget("SizeBaseIcon"));
		m_wBaseOverlay = w.FindAnyWidget("ImageOverlay");
		m_wInfoOverlay = w.FindAnyWidget("InfoOverlay");
		m_wServices = w.FindAnyWidget("ServicesFrame");
		m_w_ServicesOverlay = w.FindAnyWidget("ServicesSizeOverlay");
		m_wBaseFrame = w.FindAnyWidget("BaseFrame");
		m_wBaseIcon = w.FindAnyWidget("SideSymbol");
		m_wBaseName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wCallsignName = TextWidget.Cast(w.FindAnyWidget("Callsign"));
		m_wBaseNameDialog = TextWidget.Cast(w.FindAnyWidget("m_w_NameDialog"));
		m_wCallsignNameDialog = TextWidget.Cast(w.FindAnyWidget("Callsign-Dialog"));
		m_wInfoText = w.FindAnyWidget("Info");
		m_wAntennaImg = w.FindAnyWidget("AntenaOff");
		m_wLocalTask = ImageWidget.Cast(w.FindAnyWidget("LocalTask"));
		
		if (m_wInfoText)
			m_wSuppliesText = TextWidget.Cast(m_wInfoText.FindAnyWidget("Supplies"));
		
		m_wServices.SetVisible(false);
		m_wServices.SetOpacity(0);
	}
}