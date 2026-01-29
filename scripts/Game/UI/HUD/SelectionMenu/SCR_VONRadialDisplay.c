//------------------------------------------------------------------------------------------------
enum EVONMenuTable
{
	PREVIOUS = 0,
	CURRENT,
	NEXT
}

//------------------------------------------------------------------------------------------------
class SCR_CompareGroupRadioFreq : SCR_SortCompare<SCR_AIGroup>
{
	override static int Compare(SCR_AIGroup left, SCR_AIGroup right)
	{
		return left.GetRadioFrequency() < right.GetRadioFrequency();
	}
};

//------------------------------------------------------------------------------------------------
//! Info display within hud manager of player controller
[BaseContainerProps()]
class SCR_VONRadialDisplay : SCR_RadialMenuDisplay
{
	static const ref Color ORANGE = new Color(1.0, 0.5, 0, 0.05); // TODO placeholder before visual concept is done
	
	protected Widget m_wFrequencyDisplay;		// group frequency list within the center of the menu
	
	protected TextWidget m_aChannelDisplay[3];
	protected ImageWidget m_aIconDisplay[3];
	protected TextWidget m_aFrequencyDisplay[3];
	
	protected Widget m_wSizeLayout;
	protected ItemPreviewWidget m_wPreviewItem;
	protected IEntity m_Item;
	protected PreviewRenderAttributes m_PreviewAttributes;
	
	//------------------------------------------------------------------------------------------------
	//! Update frequency list data
	void UpdateFrequencyList(notnull SCR_VONEntryRadio entry)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!groupManager || !factionManager)
		{
			SetFrequenciesVisible(false);
			return;
		}
		
		Faction playerFaction = factionManager.GetLocalPlayerFaction();
		if (!playerFaction)
		{
			SetFrequenciesVisible(false);
			return;
		}
		
		array<SCR_AIGroup> playableGroups = {};
		array<SCR_AIGroup> playableGroupsOrigin = groupManager.GetPlayableGroupsByFaction(playerFaction);
		if (!playableGroupsOrigin)
		{
			SetFrequenciesVisible(false);
			return;
		}
		
		int groupCount = playableGroups.Copy(playableGroupsOrigin);
		SCR_Sorting<SCR_AIGroup, SCR_CompareGroupRadioFreq>.HeapSort(playableGroups);

		int matchID = -2; // -1 for platoon, 0+ for groups
		SCR_Faction milFaction = SCR_Faction.Cast(playerFaction);
		if (!milFaction || milFaction.GetFactionRadioFrequency() == 0 || groupCount == 0)
		{
			SetFrequenciesVisible(false);
			return;
		}
				
		int frequency =  entry.GetEntryFrequency();
		
		if (milFaction.GetFactionRadioFrequency() == frequency)
		{
			m_aChannelDisplay[EVONMenuTable.CURRENT].SetColor(GUIColors.ORANGE);
			m_aIconDisplay[EVONMenuTable.CURRENT].SetColor(GUIColors.ORANGE);
			m_aFrequencyDisplay[EVONMenuTable.CURRENT].SetColor(GUIColors.ORANGE);
			SetFrequencyDisplay(EVONMenuTable.CURRENT, null, milFaction);
			matchID = -1;
		}
		else 
		{
			foreach (int i, SCR_AIGroup group : playableGroups)
			{				
				if (group.GetRadioFrequency() == frequency)
				{
					m_aChannelDisplay[EVONMenuTable.CURRENT].SetColor(GUIColors.ORANGE);
					m_aIconDisplay[EVONMenuTable.CURRENT].SetColor(GUIColors.ORANGE);
					m_aFrequencyDisplay[EVONMenuTable.CURRENT].SetColor(GUIColors.ORANGE);
					SetFrequencyDisplay(EVONMenuTable.CURRENT, group);
					matchID = i;
				}
			}
		}
		
		switch (matchID)
		{
			case -2:	// no match, current frequency to default
				SetFrequencyDisplay(EVONMenuTable.PREVIOUS, null, milFaction);
				SetFrequencyDisplay(EVONMenuTable.CURRENT, playableGroups[0]);
	
				if (groupCount > 1)
					SetFrequencyDisplay(EVONMenuTable.NEXT, playableGroups[1]);
				else 
					ClearDisplayLine(EVONMenuTable.NEXT);		
				
				m_aChannelDisplay[EVONMenuTable.CURRENT].SetColor(GUIColors.ENABLED);
				m_aIconDisplay[EVONMenuTable.CURRENT].SetColor(GUIColors.ENABLED);
				m_aFrequencyDisplay[EVONMenuTable.CURRENT].SetColor(GUIColors.ENABLED);
				break;
			
			case -1:	 // match found, set next/prev frequencies
				SetFrequencyDisplay(EVONMenuTable.NEXT, playableGroups[0]);
				ClearDisplayLine(EVONMenuTable.PREVIOUS);
				break;

			case 0:
				SetFrequencyDisplay(EVONMenuTable.PREVIOUS, null, milFaction);
				
				if (groupCount > 1)
					SetFrequencyDisplay(EVONMenuTable.NEXT, playableGroups[1]);
				else 
					ClearDisplayLine(EVONMenuTable.NEXT);
				
				break;
			
			default: 
				SetFrequencyDisplay(EVONMenuTable.PREVIOUS, playableGroups[matchID - 1]);
				
				if (groupCount > matchID + 1)
					SetFrequencyDisplay(EVONMenuTable.NEXT, playableGroups[matchID + 1]);
				else 
					ClearDisplayLine(EVONMenuTable.NEXT);		
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set group description and frequency text
	//! \param widget is the subject widget
	//! \param group is AIGroup used to fetch relevant data from
	//! \param milFaction if passed, will use a faction to set a platoon frequency instead
	protected void SetFrequencyDisplay(EVONMenuTable line, SCR_AIGroup group, SCR_Faction milFaction = null)
	{
		if (milFaction)
		{
			m_aChannelDisplay[line].SetText("#AR-Comm_PlatoonChannel");
			m_aFrequencyDisplay[line].SetText((milFaction.GetFactionRadioFrequency() * 0.001).ToString() + " #AR-VON_FrequencyUnits_MHz");
		}
		else 
		{
			string company, platoon, squad, character, format;
			group.GetCallsigns(company, platoon, squad, character, format);
			m_aChannelDisplay[line].SetTextFormat(format, company, platoon, squad, character);
			m_aFrequencyDisplay[line].SetText((group.GetRadioFrequency()*0.001).ToString() + " #AR-VON_FrequencyUnits_MHz ");
		}
		
		m_aChannelDisplay[line].SetVisible(true);
		m_aIconDisplay[line].SetVisible(true);
		m_aFrequencyDisplay[line].SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearDisplayLine(EVONMenuTable line)
	{
		m_aChannelDisplay[line].SetVisible(false);
		m_aIconDisplay[line].SetVisible(false);
		m_aFrequencyDisplay[line].SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Color background of sibling transceivers when hovering over one, feed invali id to only restore color
	//! \param id is entry id which is the same as segment one
	void MarkSegmentBackground(int id)
	{
		foreach ( SCR_RadialMenuSegmentComponent comp : m_aSegments )
		{
			comp.GetBackgroundWidget().SetColor(Color.FromInt(Color.BLACK));
		}
		
		if (m_aSegments.IsIndexValid(id))
			m_aSegments[id].GetBackgroundWidget().SetColor(ORANGE);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepare preview item and render given entity 
	void SetPreviewItem(IEntity item)
	{
		// Visibility 
		m_wPreviewItem.SetVisible(item != null);
		
		m_Item = item;
		if (!item)
			return;

		// Get manager and render preview 
		ChimeraWorld world = ChimeraWorld.CastFrom(item.GetWorld());
		if (!world)
			return;
		
		ItemPreviewManagerEntity manager = world.GetItemPreviewManager();
		if (!manager)
			return;
		
		// Set rendering and preview properties 
		SetupPreviewAttributes(m_PreviewAttributes);
		manager.SetPreviewItem(m_wPreviewItem, item, m_PreviewAttributes);
		m_wPreviewItem.SetResolutionScale(1, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find and setup preview attributes that should be used in previwe
	protected void SetupPreviewAttributes(out PreviewRenderAttributes preview)
	{
		if (!m_Item)
			return;
		
		InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(m_Item.FindComponent(InventoryItemComponent));
		if (!inventoryItem)
			return;
		
		preview = SCR_VONPreviewAttributes.Cast(inventoryItem.FindAttribute(SCR_VONPreviewAttributes));
	}
	
	//------------------------------------------------------------------------------------------------
	void FadeItemPreview(bool state = true)
	{
		if (state)
		{
			m_wPreviewItem.SetOpacity(0);
			AnimateWidget.Opacity(m_wPreviewItem, 1, 1/0.3);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Visibility of frequency list
	void SetFrequenciesVisible(bool state)
	{
		if (state)
			AnimateWidget.Opacity(m_wFrequencyDisplay, 1, 1/0.3);
		else 
			AnimateWidget.Opacity(m_wFrequencyDisplay, 0, 1/0.3);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuEntryPerform(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry)
	{
		// TODO this is mostly overridden to avoid secondary resize animation in the parent class
		
		if (m_wSelectedIndicatorFeedback)
		{
			m_wSelectedIndicatorFeedback.SetOpacity(0);
			
			if (!AnimateWidget.IsAnimating(m_wSelectedIndicatorFeedback))
			{
				WidgetAnimationOpacity anim = AnimateWidget.Opacity(m_wSelectedIndicatorFeedback, CHANGE_CONTENT_OPACITY, UIConstants.FADE_RATE_FAST);
				anim.SetRepeat(true);
				anim.GetOnCycleCompleted().Insert(OnEntrySizeAnimCycleCompleted);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);

		m_wFrequencyDisplay = GetRootWidget().FindAnyWidget("FrequencyDisplay");
		
		m_aChannelDisplay[EVONMenuTable.PREVIOUS] = TextWidget.Cast(m_wFrequencyDisplay.FindAnyWidget("ChannelUp"));		
		m_aChannelDisplay[EVONMenuTable.CURRENT] = TextWidget.Cast(m_wFrequencyDisplay.FindAnyWidget("ChannelCurrent"));
		m_aChannelDisplay[EVONMenuTable.NEXT] = TextWidget.Cast(m_wFrequencyDisplay.FindAnyWidget("ChannelDown"));
		
		m_aIconDisplay[EVONMenuTable.PREVIOUS] = ImageWidget.Cast(m_wFrequencyDisplay.FindAnyWidget("IconUp"));
		m_aIconDisplay[EVONMenuTable.CURRENT] = ImageWidget.Cast(m_wFrequencyDisplay.FindAnyWidget("IconCurrent"));
		m_aIconDisplay[EVONMenuTable.NEXT] = ImageWidget.Cast(m_wFrequencyDisplay.FindAnyWidget("IconDown"));
		
		m_aFrequencyDisplay[EVONMenuTable.PREVIOUS] = TextWidget.Cast(m_wFrequencyDisplay.FindAnyWidget("FrequencyUp"));		
		m_aFrequencyDisplay[EVONMenuTable.CURRENT] = TextWidget.Cast(m_wFrequencyDisplay.FindAnyWidget("FrequencyCurrent"));
		m_aFrequencyDisplay[EVONMenuTable.NEXT] = TextWidget.Cast(m_wFrequencyDisplay.FindAnyWidget("FrequencyDown"));
		
		m_wPreviewItem = ItemPreviewWidget.Cast(GetRootWidget().FindAnyWidget("ItemPreview"));
		m_wSizeLayout = GetRootWidget().FindAnyWidget("SizeRTW");
		
		SetFrequenciesVisible(false);
	}
};