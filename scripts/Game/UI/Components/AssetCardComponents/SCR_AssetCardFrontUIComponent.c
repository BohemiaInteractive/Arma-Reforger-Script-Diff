//! @ingroup Editor_UI Editor_UI_Components

//! Content Browser Card
class SCR_AssetCardFrontUIComponent : SCR_ScriptedWidgetComponent
{
	//ToDo: Replace many of these values with automated solutions
    [Attribute()]
	private string m_sEntityNameWidgetName;
	
	//[Attribute()]
	//private string m_sTitleBackgroundWidgetName;
	
	[Attribute()]
	private string m_sEnityImageWidgetName;
	
	[Attribute()]
	protected string m_sIconSlotWidgetName;
	
	[Attribute("ModOverlay")]
	protected string m_sModIndicatorWidgetName;
	
	[Attribute("ModIndicatorIcon")]
	protected string m_sModIndicatorIconName;
	
	[Attribute("NoBudget")]
	protected string m_sExceedBudgetLayoutName;
	
	[Attribute("BudgetIcon")]
	protected string m_sExceedBudgetIconName;
	
	[Attribute("CompositionCost")]
	protected string m_sBudgetCostLayoutName;
	
	[Attribute("CostValue")]
	protected string m_sBudgetCostTextName;
	
	[Attribute("ImageOverlay")]
	protected string m_sImageOverlayName;
	
	[Attribute("AssetCardBackground")]
	protected string m_sImageHoverName;
	
	[Attribute("Image")]
	protected string m_sImageName;
	
	[Attribute("FullBackground")]
	protected string m_sImageBackgroundName;
	
	[Attribute("")]
	protected string m_sAreaName;
	
	[Attribute()]
	protected bool m_bEnableHover;
	
	[Attribute()]
	protected ref array<string> m_aFactionColorNames;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabelGroup))]
	protected ref array<EEditableEntityLabelGroup> m_sTraitLabelGroups;
	
	[Attribute()]
	protected ref array<string> m_sTraitNames;
	
	[Attribute()]
	private ref Color m_DisabledColor;
	
	[Attribute(params: "edds")]
	protected ref array<ResourceName> m_ImageOverlayTextures;
	
	[Attribute("", UIWidgets.EditBox, "Name of frame widget which holds Content meant for Horizontal scrolling")]
	string m_sAssetCardNameFrameWidget;
	
	protected ref ImageWidget m_EntityImageWidget;
	protected ref ImageWidget m_wHoverImageWidget;
	
	protected Widget m_BudgetCostLayout;
	protected Widget m_ExceedBudgetLayout;
	protected ref ScriptInvoker Event_OnCardInit = new ScriptInvoker();
	
	//Widgets
	private Widget m_wWidget;
	private Widget m_wAssetCardName //~ saved for hotfix below (todo ewe)
	protected ImageWidget m_wBackground;
	
	//Grid
	protected int m_iPrefabIndex;
	
	protected ref ScriptInvoker m_OnHover = new ScriptInvoker();
	protected ref ScriptInvoker m_OnFocus = new ScriptInvoker();
	
	//Info Ref
	private ref SCR_UIInfo m_Info;
	
	//Component Ref
	private ref SCR_HorizontalScrollAnimationComponent m_HorizontalScrollComponent;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnCardInit()
	{		
		return Event_OnCardInit;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entityBudgetCost
	void UpdateBudgetCost(SCR_EntityBudgetValue entityBudgetCost = null)
	{
		m_BudgetCostLayout = m_wWidget.FindAnyWidget(m_sBudgetCostLayoutName);
		if (!m_BudgetCostLayout)
			return;
		
		if (!entityBudgetCost)
			m_BudgetCostLayout.SetVisible(false);
		else
		{
			m_BudgetCostLayout.SetVisible(true);
			TextWidget budgetCostTextWidget = TextWidget.Cast(m_BudgetCostLayout.FindAnyWidget(m_sBudgetCostTextName));
			if (budgetCostTextWidget)
				budgetCostTextWidget.SetText(entityBudgetCost.GetBudgetValue().ToString());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] blockingBudgetInfo
	void UpdateBlockingBudget(SCR_UIInfo blockingBudgetInfo = null)
	{
		if (!m_ExceedBudgetLayout)
			return;
		
		bool canPlace = blockingBudgetInfo == null;
		
		m_EntityImageWidget.SetSaturation(canPlace);
		m_ExceedBudgetLayout.SetVisible(!canPlace);
		
		ImageWidget exceedBudgetIcon = ImageWidget.Cast(m_ExceedBudgetLayout.FindWidget(m_sExceedBudgetIconName));
		if (blockingBudgetInfo)
			blockingBudgetInfo.SetIconTo(exceedBudgetIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init card
	//! \param[in] prefabID
	//! \param[in] info
	//! \param[in] prefab
	//! \param[in] blockingBudgetInfo
	void InitCard(int prefabID, SCR_UIInfo info, ResourceName prefab, SCR_UIInfo blockingBudgetInfo = null)
	{
		if (!m_wWidget)
			return;
		
		m_iPrefabIndex = prefabID;
		
		TextWidget entityNameWidget = TextWidget.Cast(m_wWidget.FindAnyWidget(m_sEntityNameWidgetName));
		m_wBackground = ImageWidget.Cast(m_wWidget.FindAnyWidget(m_sImageBackgroundName));
		
		if (info)
		{
			m_Info = info;			
			//Set the entity name
			info.SetNameTo(entityNameWidget);

			SCR_EditableEntityUIInfo infoCard = SCR_EditableEntityUIInfo.Cast(info);
			if (infoCard)
			{
				SCR_EditableEntityUIInfoColored colorOverwriteInfo = SCR_EditableEntityUIInfoColored.Cast(infoCard);
				Faction faction = infoCard.GetFaction();
				
				if (!colorOverwriteInfo)
				{
					//--- Set faction color (all widgets marked by 'Inherit Color' will use it as well)
					if (faction)
					{
						Widget factionColor;
						foreach (string factionColorName: m_aFactionColorNames)
						{
							if (infoCard.IsFullBackgroundAssetCard() && factionColorName == "ColorOverlay")
								continue;
							
							factionColor = m_wWidget.FindAnyWidget(factionColorName);
							if (factionColor)
								factionColor.SetColor(Color.FromInt(faction.GetFactionColor().PackToInt()));
						}
					}
				}
				else 
				{
					Widget colorOverlay = m_wWidget.FindAnyWidget("ColorOverlay");
					if (colorOverlay)
						colorOverlay.SetColor(colorOverwriteInfo.GetOverwriteColor());
				}
				
				
				//--- Set icon
				Widget iconSlotWidget = m_wWidget.FindAnyWidget(m_sIconSlotWidgetName);
				if (iconSlotWidget)
				{
					SCR_EditableEntityBaseSlotUIComponent iconSlot = SCR_EditableEntityBaseSlotUIComponent.Cast(iconSlotWidget.FindHandler(SCR_EditableEntityBaseSlotUIComponent));
					if (iconSlot)
					{
						SCR_EditableEntityUIConfig entityUIConfig = SCR_EditableEntityUIConfig.GetConfig();
						if (entityUIConfig)
						{
							array<ref SCR_EntitiesEditorUIRule> entityRules = entityUIConfig.GetRules();
							foreach (SCR_EntitiesEditorUIRule rule: entityRules)
							{
								if (rule.GetStates() & EEditableEntityState.RENDERED)
								{
									iconSlot.CreateWidget(infoCard, rule);
								}
							}
						}
					}
				}
				
				//--- Set entity image
				m_EntityImageWidget = ImageWidget.Cast(m_wWidget.FindAnyWidget(m_sEnityImageWidgetName));
				if (m_EntityImageWidget)
					infoCard.SetAssetImageTo(m_EntityImageWidget); // Also, depending on the entity flag, can set the background stretched, check function definition
				
				ImageWidget imageOverlayWidget = ImageWidget.Cast(m_wWidget.FindAnyWidget(m_sImageOverlayName));
				if (imageOverlayWidget && !m_ImageOverlayTextures.IsEmpty()) 
				{
					int textureIndex = m_iPrefabIndex % m_ImageOverlayTextures.Count();
					imageOverlayWidget.LoadImageTexture(0, m_ImageOverlayTextures[textureIndex]);
					m_EntityImageWidget = imageOverlayWidget;
				}
				
				m_ExceedBudgetLayout = m_wWidget.FindAnyWidget(m_sExceedBudgetLayoutName);
				
				UpdateBudgetCost();	
				UpdateBlockingBudget(blockingBudgetInfo);
				
				//--- Set faction flag
				int traitIndex;
				Widget traitWidget;
				ImageWidget traitIcon;
				if (faction)
				{
					SCR_ContentBrowserEditorComponent contentBrowser = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, false, true));
					if (!contentBrowser || contentBrowser.AreFactionsShownOnContentCards())
					{
						if (FindTraitIcon(traitIndex, traitWidget, traitIcon))
						{
							traitWidget.SetVisible(true);
							traitIcon.LoadImageTexture(0, faction.GetUIInfo().GetIconPath());
						}
					}
				}
				
				EEditorMode editorMode = 0;
				SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
				if (editorManager) 
				{
					SCR_EditorModeEntity modeEntity = editorManager.GetCurrentModeEntity();
					if (modeEntity)
						editorMode = modeEntity.GetModeType();
				}		
						
				//--- Find traits in labels
				SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
				if (core)
				{
					EEditableEntityLabelGroup labelGroup;
					array<EEditableEntityLabel> labels = {};
					SCR_UIInfo labelInfo;
					for (int i, count = infoCard.GetEntityLabels(labels); i < count; i++)
					{
						if (!core.GetLabelUIInfoIfValid(labels[i], editorMode, labelInfo))
							continue;
						
						if (core.GetLabelGroupType(labels[i], labelGroup) && m_sTraitLabelGroups.Contains(labelGroup) && labelInfo.HasIcon())
						{
							if (FindTraitIcon(traitIndex, traitWidget, traitIcon))
							{
								traitWidget.SetVisible(true);
								labelInfo.SetIconTo(traitIcon);
								
								SCR_LinkTooltipTargetEditorUIComponent tooltip = SCR_LinkTooltipTargetEditorUIComponent.Cast(traitWidget.FindHandler(SCR_LinkTooltipTargetEditorUIComponent));
								if (tooltip)
									tooltip.SetInfo(labelInfo);
							}
							else
							{
								//--- Out of trait icons
								break;
							}
						}
					}
				}
				
				//--- Show area image (only for systems)
				if (m_sAreaName)
				{
					Widget areaWidget = m_wWidget.FindAnyWidget(m_sAreaName);
					if (areaWidget)
						areaWidget.SetVisible(infoCard.HasEntityFlag(EEditableEntityFlag.HAS_AREA));
				}
			}
		}
		else
		{
			if (entityNameWidget && prefab)
			{
				string path = prefab.GetPath();
				array<string> pathNames = {};
				path.Split("/", pathNames, true);
				entityNameWidget.SetText(pathNames[pathNames.Count() - 1]);
			}
		}
		
		//--- Hook up events for Asset name horizontal scrolling component
		m_wAssetCardName = m_wWidget.FindAnyWidget(m_sAssetCardNameFrameWidget);
		
		if (m_wAssetCardName)
		{
			m_HorizontalScrollComponent = SCR_HorizontalScrollAnimationComponent.Cast(m_wAssetCardName.FindHandler(SCR_HorizontalScrollAnimationComponent));
			if (m_HorizontalScrollComponent)
			{
				// Focus is for controller "hover"
				GetOnFocus().Insert(OnHoverScrollAnimation);
				// Hover is for mouse hover
				GetOnHover().Insert(OnHoverScrollAnimation);
				
				// Necessary otherwise animation starts playing on Init
				m_HorizontalScrollComponent.AnimationStop();
				m_HorizontalScrollComponent.ResetPosition();
			}
		}
		
		//Check if asset is modded
		Widget modIndicator = m_wWidget.FindAnyWidget(m_sModIndicatorWidgetName);
		if (modIndicator)
		{
			array<string> modList = SCR_AddonTool.GetResourceAddons(prefab, true);
			if (modList)
				modIndicator.SetVisible(modList.Count() > 0);
			else
				modIndicator.SetVisible(false);
			
			//~todo: Get Addon Icon. Use m_sModIndicatorIconName to get the ImageWidget
		}
		
		Event_OnCardInit.Invoke(m_iPrefabIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	set<SCR_EditableEntityComponent> FactionRecipients()
	{
		Faction faction;
		
		SCR_EditableEntityUIInfo infoCard = SCR_EditableEntityUIInfo.Cast(m_Info);
		if (infoCard)
			faction = infoCard.GetFaction();
		
		if (!faction)
			return null;
		
		SCR_EditableFactionComponent factionDelegate;
		
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (delegateFactionManager)
			factionDelegate = delegateFactionManager.GetFactionDelegate(faction);
		
		set<SCR_EditableEntityComponent> factionRecipients = new set<SCR_EditableEntityComponent>();
		if (factionDelegate)
			factionRecipients.Insert(factionDelegate);
		
		return factionRecipients;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FindTraitIcon(out int outIndex, out Widget traitWidget, out ImageWidget traitIcon)
	{
		if (m_sTraitNames && outIndex < m_sTraitNames.Count())
		{
			traitWidget = m_wWidget.FindAnyWidget(m_sTraitNames[outIndex]);
			if (traitWidget)
			{
				traitIcon = ImageWidget.Cast(traitWidget.FindAnyWidget("Trait"));
				outIndex++;
				return true;
			}
		}

		outIndex = int.MAX;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHoverScrollAnimation(Widget widget, int prefabIndex, bool hoverEntered)
	{
		//~todo ewe: known issue on Content browser Animation wont start cause frameWidth is always 0 on init
		// Makes sure name text widget is up to date for ContentFit check
		widget.Update();
		
		// Hotfix for animation starting when content fits on Content Browser Init (related to todo above)
		float width, height;
		
		m_wAssetCardName.Update();
		m_wAssetCardName.GetScreenSize(width, height);
		
		// Checks if content does not fit before starting animation
		if (hoverEntered && !m_HorizontalScrollComponent.GetContentFitX() && width > 0)
		{
			m_HorizontalScrollComponent.AnimationStart();
		}
		else if (!hoverEntered || m_HorizontalScrollComponent.GetContentFitX() || width <= 0)
		{
			m_HorizontalScrollComponent.AnimationStop();
			m_HorizontalScrollComponent.ResetPosition();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get Card info
	//! \return SCR_UIInfo
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set Background Image
	//! \param[in] imageName
	void SetBackgroundImage(ResourceName imageName)
	{		
		if (m_wBackground)
		{
			if (imageName.IsEmpty())
			{
				m_wBackground.SetVisible(false);
			}
			else
			{
				m_wBackground.LoadImageTexture(0, imageName);
				m_wBackground.SetVisible(true);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set Image
	//! \param[in] imageName
	void SetHoverImage(ResourceName imageName)
	{		
		if (m_wHoverImageWidget)
		{
			if (imageName.IsEmpty())
			{
				m_wHoverImageWidget.SetVisible(false);
			}
			else
			{
				m_wHoverImageWidget.LoadImageTexture(0, imageName);
				m_wHoverImageWidget.SetVisible(true);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set Image
	//! \param[in] imageName
	void SetImage(ResourceName imageName)
	{		
		if (m_EntityImageWidget)
		{
			if (imageName.IsEmpty())
			{
				m_EntityImageWidget.SetVisible(false);
			}
			else
			{
				m_EntityImageWidget.LoadImageTexture(0, imageName);
				m_EntityImageWidget.SetVisible(true);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set Image
	//! \param[in] imageName
	void SetText(string newText)
	{
		TextWidget entityNameWidget = TextWidget.Cast(m_wWidget.FindAnyWidget(m_sEntityNameWidgetName));
		if (entityNameWidget)
		{
			if (newText.IsEmpty())
			{
				entityNameWidget.SetVisible(false);
			}
			else
			{
				entityNameWidget.SetVisible(true);
				entityNameWidget.SetText(newText);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set faction color of specific widgets
	//! \param[in] newColor
	protected void SetHover(bool state)
	{
		if (!m_bEnableHover)
			return;
		
		if (m_wHoverImageWidget)
			AnimateWidget.Opacity(m_wHoverImageWidget, !state, UIConstants.FADE_RATE_DEFAULT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set faction color of specific widgets
	//! \param[in] newColor
	protected void SetBackgroundColor(Color newColor, float transition =  UIConstants.FADE_RATE_FAST)
	{
		if (m_wBackground)
			AnimateWidget.Color(m_wBackground, newColor, transition);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set faction color of specific widgets
	//! \param[in] newColor
	protected void SetImageColor(Color newColor, float transition =  UIConstants.FADE_RATE_FAST)
	{
		if (m_EntityImageWidget)
			AnimateWidget.Color(m_EntityImageWidget, newColor, transition);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Button Widget
	//! \return Widget button
	Widget GetButtonWidget()
	{
		return m_wWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_wWidget)
			m_OnHover.Invoke(m_wWidget, m_iPrefabIndex, true);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_wWidget)
			m_OnHover.Invoke(m_wWidget, m_iPrefabIndex, false);

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get ScriptInvoker onHover
	//! \return ScriptInvoker
	ScriptInvoker GetOnHover()
	{
		return m_OnHover;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		//SetImageColor(UIColors.CONTRAST_CLICKED);
		SetHover(true);
		
		if (m_wWidget)
			m_OnFocus.Invoke(m_wWidget, m_iPrefabIndex, true);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		//SetImageColor(Color.White);
		SetHover(false);
		
		if (m_wWidget)
			m_OnFocus.Invoke(m_wWidget, m_iPrefabIndex, false);

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get ScriptInvoker onFocus
	//! \return ScriptInvoker
	ScriptInvoker GetOnFocus()
	{
		return m_OnFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Prefab index from the list of placeable prefabs.
	int GetPrefabIndex()
	{
		return m_iPrefabIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
    {
		m_wWidget = w;
		
		if (!m_EntityImageWidget)
			m_EntityImageWidget = ImageWidget.Cast(m_wWidget.FindAnyWidget(m_sImageName));
		
		if (!m_wHoverImageWidget)
			m_wHoverImageWidget = ImageWidget.Cast(m_wWidget.FindAnyWidget(m_sImageHoverName));
		
		if (!m_wBackground)
			m_wBackground = ImageWidget.Cast(m_wWidget.FindAnyWidget(m_sImageBackgroundName));
		
		if (m_bEnableHover)
			SetHover(false);
	}
	
	override void HandlerDeattached(Widget w)
	{
		if (m_HorizontalScrollComponent)
		{
			GetOnFocus().Remove(OnHoverScrollAnimation);
			GetOnHover().Remove(OnHoverScrollAnimation);
		}
	}
}
