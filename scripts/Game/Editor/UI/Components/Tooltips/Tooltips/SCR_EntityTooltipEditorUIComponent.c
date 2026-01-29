//! @ingroup Editor_UI Editor_UI_Components

class SCR_EntityTooltipEditorUIComponent : SCR_BaseTooltipEditorUIComponent
{
	[Attribute()]
	protected string m_sImageWidgetName;
	
	[Attribute()]
	protected string m_sImageOverlayWidgetName;
	
	[Attribute()]
	protected string m_sFactionWidgetName;
	
	[Attribute()]
	protected string m_sDetailsNameWidgetName;
	
	[Attribute()]
	protected string m_sHeaderTypeName;
	
	[Attribute()]
	protected string m_sAssetCardWidgetName;
	
	[Attribute("StyleBar")]
	protected string m_sColorBarName;
	
	[Attribute()]
	protected ref Color m_cPlayerColor;
	
	[Attribute("{96EFE81470EF9AA5}Configs/Editor/Tooltips/EntityTooltips.conf", desc: "Holds all entity tooltip data")]
	protected ref SCR_EntityTooltipConfig m_EntityTooltipConfig;
	
	protected SCR_EntityTooltipDetailType m_DetailType;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateTooltip(Managed instance = null)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(instance);
		if (m_DetailType)
			m_DetailType.UpdateDetailType(entity);
	}

	//------------------------------------------------------------------------------------------------
	override bool SetTooltip(SCR_UIInfo info, Managed instance = null)
	{
		//--- Get entity
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(instance);
		if (!entity)
			return false;
		
		EEditableEntityType entityType = entity.GetEntityType();
		Widget widget = GetWidget();
		int playerID = entity.GetPlayerID();
		bool showImage = true;
		
		if (!m_EntityTooltipConfig)
		{
			Print("'SCR_EntityTooltipEditorUIComponent' is missing m_EntityTooltipConfig! meaning tooltips cannot be displayed!", LogLevel.ERROR);
			return false;
		}
		
		//--- Entity details
		Widget detailsWidget = widget.FindAnyWidget(m_sDetailsNameWidgetName);
		if (detailsWidget)
		{
			array<ref SCR_EntityTooltipDetailType> detailTypes = {};
			m_EntityTooltipConfig.GetTooltipData(detailTypes);
			
			foreach (SCR_EntityTooltipDetailType detailType: detailTypes)
			{
				if (detailType.CreateDetailType(entityType, detailsWidget, entity, showImage))
				{
					m_DetailType = detailType;
					break;
				}
			}

			detailsWidget.SetVisible(m_DetailType != null);
		}
		
		//--- Get entity info (or if player without entity)
		if (info)
		{
			//--- Entity name
			TextWidget nameWidget = TextWidget.Cast(widget.FindAnyWidget(m_sNameWidgetName));
			if (nameWidget)
			{
				if (playerID > 0)
				{
					string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);
					
					if(GetGame().GetPlayerManager().GetPlatformKind(SCR_PlayerController.GetLocalPlayerId()) == PlatformKind.PSN || true){
						if (GetGame().GetPlayerManager().GetPlatformKind(playerID) == PlatformKind.PSN)
							playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME, 1.75) + playerName;
						else
							playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME, 1.75) + playerName;
					}
									
					nameWidget.SetText(playerName);
					
					Widget colorBar = widget.FindAnyWidget(m_sColorBarName);
					if (colorBar)
						colorBar.SetColor(m_cPlayerColor);
				}
				else
				{
					if (!info.SetNameTo(nameWidget))
						nameWidget.SetText(string.Format("ERROR: Unknown %1", typename.EnumToString(EEditableEntityType, entityType)));
				}
			}
			
			SCR_CallsignBaseComponent callsignComponent = SCR_CallsignBaseComponent.Cast(entity.GetOwner().FindComponent(SCR_CallsignBaseComponent));
			if (callsignComponent)
			{
				TextWidget callsignWidget = TextWidget.Cast(widget.FindAnyWidget(m_sHeaderTypeName)); 
				string company, platoon, squad, character, format;
				bool setCallsign = callsignComponent.GetCallsignNames(company, platoon, squad, character, format);
				
				if (callsignWidget && setCallsign)
				{
					callsignWidget.SetTextFormat(format, company, platoon, squad, character);
					callsignWidget.SetVisible(true);	
				}
			}
			
			//--- Get faction color
			Color factionColor = Color.FromInt(Color.WHITE);
			Faction faction = entity.GetFaction();
			if (faction) 
			{
				factionColor = faction.GetFactionColor();
			}
			else if (playerID > 0)
			{
				SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
				
				if (factionManager)
					faction = factionManager.GetPlayerFaction(playerID);
				
				if (faction)
					factionColor = faction.GetFactionColor();
			}
			
			//--- Widget colored by faction
			ImageWidget factionWidget = ImageWidget.Cast(widget.FindAnyWidget(m_sFactionWidgetName));
			if (factionWidget) 
			{
				factionWidget.SetColor(factionColor);
				factionWidget.SetOpacity(0.6);
			}
			
			//--- Entity icon

//			ImageWidget imageWidget = ImageWidget.Cast(widget.FindAnyWidget(m_sIconWidgetName));
//			if (imageWidget && !info.GetIconPath().IsEmpty())
//			{
//				imageWidget.LoadImageTexture(0, info.GetIconPath());
//				imageWidget.SetColor(factionColor);
//			}
			
			Widget slotWidget = widget.FindAnyWidget(m_sIconWidgetName);
			if (slotWidget)
			{
				SCR_EditableEntitySlotManagerUIComponent slotManager = SCR_EditableEntitySlotManagerUIComponent.Cast(slotWidget.FindHandler(SCR_EditableEntitySlotManagerUIComponent));
				SCR_EditableEntityBaseSlotUIComponent slot = SCR_EditableEntityBaseSlotUIComponent.Cast(slotWidget.FindHandler(SCR_EditableEntityBaseSlotUIComponent));
				if (slotManager && slot)
				{
					//slotManager.InitSlotManager(entity);
					slotManager.InsertSlotExternal(entity, slot);
				}
				else
				{
					Print(string.Format("SCR_EditableEntityBaseSlotUIComponent and/or SCR_EditableEntitySlotManagerUIComponent is missing on tooltip widget '%1'!", widget.GetName()), LogLevel.WARNING);
				}
			}
			
			SCR_EditableEntityUIInfo editableEntityInfo = SCR_EditableEntityUIInfo.Cast(info);
			if (editableEntityInfo && showImage)
			{
				ImageWidget imageWidget = ImageWidget.Cast(widget.FindAnyWidget(m_sImageWidgetName));
				if (imageWidget && editableEntityInfo.GetImage())
				{
					bool foundImage = imageWidget.LoadImageTexture(0, editableEntityInfo.GetImage());
					imageWidget.SetVisible(foundImage);

					Widget imageOverlay = widget.FindAnyWidget(m_sImageOverlayWidgetName);
					if (!imageOverlay)
						imageOverlay = imageWidget;
					
					imageOverlay.SetVisible(foundImage);
				}
			}
		}
		
		//--- Asset card (experimental)

//		Widget assetCardWidget = widget.FindAnyWidget(m_sAssetCardWidgetName);
//		if (assetCardWidget)
//		{
//			SCR_AssetCardFrontUIComponent assetCard = SCR_AssetCardFrontUIComponent.Cast(assetCardWidget.FindHandler(SCR_AssetCardFrontUIComponent));
//			if (assetCard)
//			{
//				assetCard.InitCard(info, true);
//			}
//		}
		
		return true;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_EntityType")]
class SCR_EntityTooltipDetailType
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	protected EEditableEntityType m_EntityType;
	
	[Attribute(params: "layout")]
	protected ResourceName m_DetailLayout;
	
	[Attribute("1")]
	protected bool m_bEnabled;
	
	[Attribute("1")]
	protected bool m_bShowImage;
	
	[Attribute()]
	protected string m_sDetailLayoutName;
	
	[Attribute()]
	protected string m_sDetailLayoutFrame;

	[Attribute()]
	protected ref array<ref SCR_EntityTooltipDetail> m_aDetails;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	void UpdateDetailType(SCR_EditableEntityComponent entity)
	{
		foreach (SCR_EntityTooltipDetail detail: m_aDetails)
		{
			if (detail.NeedUpdate())
				detail.UpdateDetail(entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] type
	//! \param[in] parent
	//! \param[in] entity
	//! \param[out] showImage
	//! \return
	bool CreateDetailType(EEditableEntityType type, Widget parent, SCR_EditableEntityComponent entity, out bool showImage = true)
	{
		if (type != m_EntityType || !m_bEnabled)
			return false;

		showImage = m_bShowImage;
		bool showDetails = false;
		WorkspaceWidget workspace = parent.GetWorkspace();
		foreach (SCR_EntityTooltipDetail detail: m_aDetails)
		{
			Widget detailWidget;
			Widget detailFrame;
			TextWidget detailName = null; 
			
			//With label
			if (detail.GetShowLabel())
			{
				detailWidget = workspace.CreateWidgets(m_DetailLayout, parent);
				detailFrame = detailWidget.FindAnyWidget(m_sDetailLayoutFrame);
				detailName = TextWidget.Cast(detailWidget.FindAnyWidget(m_sDetailLayoutName));	
			}
			//No label
			else 
			{
				detailFrame = parent;
			}
			
			if (detail.CreateDetail(entity, detailFrame, detailName, detail.GetShowLabel()))
			{
				showDetails = true;
				detail.UpdateDetail(entity);
			}
			else if (detailWidget)
			{
				detailWidget.RemoveFromHierarchy();
			}
		}

		return showDetails;
	}
}
