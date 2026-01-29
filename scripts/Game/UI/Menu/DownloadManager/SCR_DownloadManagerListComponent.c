/*!
Download manager list that can group download entries into various sections
*/

class SCR_DownloadManagerListComponent : SCR_SubMenuBase
{
	[Attribute("m_AddonsList")]
	protected string m_sList;
	
	[Attribute("m_FallbackTextWrap")]
	protected string m_sFallbackTextWrapWidget;
	
	[Attribute()]
	protected ref array<ref SCR_DownloadManagerListCategory> m_aCategories;
	
	protected ScrollLayoutWidget m_ScrollLayout;
	protected Widget m_wList;
	protected Widget m_wFallbackTextWrap;
	
	protected SCR_InputButtonComponent m_NavPauseResume;
	protected SCR_InputButtonComponent m_NavPauseResumeAll;
	protected SCR_InputButtonComponent m_NavCancel;
	protected SCR_InputButtonComponent m_NavRetry;
	
	protected ref array<ref SCR_DownloadManagerEntry> m_aAddonLines = {};
	
	protected bool m_bAllPaused = false;
	
	protected SCR_DownloadManagerEntry m_FocusedEntry;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_ScrollLayout = ScrollLayoutWidget.Cast(w.FindAnyWidget("ScrollLayout"));
		m_wList = w.FindAnyWidget(m_sList);
		m_wFallbackTextWrap = w.FindAnyWidget(m_sFallbackTextWrapWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
		
		UpdateNavButtons(m_FocusedEntry);
		
		// Show fallback if list is empty 
		ShowFallbackText(m_wList.GetChildren() == null);
		
		GetGame().GetCallqueue().Call(FocusFirstLine);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_NavPauseResume = CreateNavigationButton("WorkshopPauseDownload", "#AR-Workshop_ButtonResume", true, false);
		m_NavPauseResume.m_OnActivated.Insert(OnClickPauseResume);
		
		m_NavCancel = CreateNavigationButton("WorkshopCancelDownload", "#AR-Keybind_Cancel", true, false);
		m_NavCancel.m_OnActivated.Insert(OnClickCancel);
		
		m_NavRetry = CreateNavigationButton("WorkshopPauseDownload", "#AR-Workshop_ButtonTryAgain", true, false);
		m_NavRetry.m_OnActivated.Insert(OnClickRetry);
		
		m_NavPauseResumeAll = CreateNavigationButton("WorkshopPauseAllDownloads", "#AR-DownloadManager_ButtonPauseAll", false);
		m_NavPauseResumeAll.m_OnActivated.Insert(OnClickPauseResumeAll);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		
		GetGame().GetCallqueue().Call(FocusFirstLine);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add new widget in category based based on category type
	Widget AddEntry(ResourceName entryPath, SCR_WorkshopItemActionDownload action)
	{
		if (!m_wList || !entryPath)
		{
			Print("List widget or widgte resource wasn't set properly!", LogLevel.WARNING);
			return null;
		}
		
		// Create new entry 
		Widget entry = GetGame().GetWorkspace().CreateWidgets(entryPath, m_wList);
		LayoutSlot.SetHorizontalAlign(entry, LayoutHorizontalAlign.Stretch);
		
		// Add addon line comp
		SCR_ButtonBaseComponent btn = SCR_ButtonBaseComponent.Cast(entry.FindHandler(SCR_ButtonBaseComponent));
		btn.m_OnFocus.Insert(OnEntryFocus);
		btn.m_OnFocusLost.Insert(OnEntryFocusLost);
		
		SCR_DownloadManagerEntry entryComp = SCR_DownloadManagerEntry.Cast(entry.FindHandler(SCR_DownloadManagerEntry));
		entryComp.InitForDownloadAction(action.GetWorkshopItem(), action);
		m_aAddonLines.Insert(entryComp);
		
		ShowFallbackText(false);
		
		// Set category z order 
		EDownloadManagerActionState type = SCR_DownloadManagerEntry.DownloadActionState(action);
		
		for (int i = 0, count = m_aCategories.Count(); i < count; i++)
		{
			if (m_aCategories[i].m_iType == type)
			{
				entry.SetZOrder(m_aCategories[i].m_iZOrder);
				return entry;
			}
		}
		
		// No z order warning!
		Print(string.Format("No Z order found for type: %1!", typename.EnumToString(EDownloadManagerActionState, type)), LogLevel.WARNING);
		
		return entry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change widget in category based based on category type
	void ChangeEntryCategory(notnull Widget entry, EDownloadManagerActionState type)
	{
		if (entry.GetParent() != m_wList)
		{
			Print("Selected entry is not part of list!", LogLevel.WARNING);
			return;
		}
		
		int categoryZOrder = -1;
		
		// Find category 
		for (int i = 0, count = m_aCategories.Count(); i < count; i++)
		{
			if (m_aCategories[i].m_iType == type)
			{
				categoryZOrder = m_aCategories[i].m_iZOrder;
				break;
			}
		}
		
		// Move to category widget
		if (categoryZOrder != -1)
			entry.SetZOrder(categoryZOrder);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveEntry(notnull SCR_DownloadManagerEntry entry)
	{
		m_wList.RemoveChild(entry.GetRootWidget());
		m_aAddonLines.RemoveItem(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find line containing selected download action 
	SCR_DownloadManagerEntry DownloadActionLine(notnull SCR_WorkshopItemActionDownload action)
	{
		foreach (SCR_DownloadManagerEntry entry : m_aAddonLines)
		{
			if (entry.GetDownloadAction() == action)
				return entry;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find line containing selected item 
	SCR_DownloadManagerEntry EntryWithItem(notnull SCR_WorkshopItem item)
	{
		for (int i = 0, count = m_aAddonLines.Count(); i < count; i++)
		{
			if (m_aAddonLines[i].GetItem() == item)
				return m_aAddonLines[i];
		}
		
		return null;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Return addon preset line component of currently focused widget if has any 
	SCR_DownloadManagerEntry FocusedEntry()
	{
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focused)
			return null;
		
		return SCR_DownloadManagerEntry.Cast(focused.FindHandler(SCR_DownloadManagerEntry));
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowPauseResumeAllButton(bool show)
	{
		if (m_NavPauseResumeAll)
			SetNavigationButtonVisibile(m_NavPauseResumeAll, show);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowFallbackText(bool show)
	{
		if (m_wFallbackTextWrap)
			m_wFallbackTextWrap.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return true if there are entries in list
	bool HasContent()
	{
		return m_wList.GetChildren() != null;
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks 
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnEntryFocus()
	{
		SCR_DownloadManagerEntry entry = FocusedEntry();
			
		// Setup change callback
		m_FocusedEntry = entry;
		m_FocusedEntry.GetOnUpdate().Insert(UpdateNavButtons);
		
		UpdateNavButtons(m_FocusedEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntryFocusLost()
	{
		if (m_FocusedEntry)
			m_FocusedEntry.GetOnUpdate().Remove(UpdateNavButtons);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateNavButtons(SCR_DownloadManagerEntry entry)
	{
		// Setup button visibility
		SetNavigationButtonVisibile(m_NavPauseResume, entry != null);
		SetNavigationButtonVisibile(m_NavCancel, entry != null);
		SetNavigationButtonVisibile(m_NavRetry, entry != null);
		
		// Check current entry
		if (!entry)
			return;
		
		bool pause, resume, cancel, retry;
		entry.CanDoActions(pause, resume, cancel, retry);
		
		// Pause and resume 
		SetNavigationButtonVisibile(m_NavPauseResume, pause || resume);
		m_NavPauseResume.SetEnabled(entry.GetPauseEnabled());
		
		if (resume)
			m_NavPauseResume.SetLabel("#AR-DownloadManager_ButtonResume");
		else
			m_NavPauseResume.SetLabel("#AR-DownloadManager_ButtonPause");
		
		// Cancel
		SetNavigationButtonVisibile(m_NavCancel, cancel);
		
		// Retry
		SetNavigationButtonVisibile(m_NavRetry, retry);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnClickPauseResume()
	{
		SCR_DownloadManagerEntry entry = FocusedEntry();
		if (!entry)
			return;
		
		if (entry.GetDownloadAction().IsPaused())
			entry.OnClickResume();
		else
			entry.OnClickPause();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnClickCancel()
	{
		SCR_DownloadManagerEntry entry = FocusedEntry();
		if (entry)
			entry.OnClickCancel();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnClickRetry()
	{
		SCR_DownloadManagerEntry entry = FocusedEntry();
		if (!entry)
			return;
		
		entry.OnClickRetry();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnClickPauseResumeAll()
	{
		for (int i = 0, count = m_aAddonLines.Count(); i < count; i++)
		{
			// Pause all 
			if (!m_bAllPaused)
			{
				if (!m_aAddonLines[i].GetDownloadAction().IsPaused())
					m_aAddonLines[i].OnClickPause();
				
				continue;
			}
			
			// Resume all 
			if (m_aAddonLines[i].GetDownloadAction().IsPaused())
				m_aAddonLines[i].OnClickResume();
		}
		
		m_bAllPaused = !m_bAllPaused;
		
		if (m_bAllPaused)
			m_NavPauseResumeAll.SetLabel("#AR-DownloadManager_ButtonResumeAll");
		else
			m_NavPauseResumeAll.SetLabel("#AR-DownloadManager_ButtonPauseAll");
		
		// Disable and enable later to prevent pause/resume request spamming
		m_NavPauseResumeAll.SetEnabled(false);
		GetGame().GetCallqueue().CallLater(EnablePauseResumeAll, SCR_DownloadManagerEntry.PAUSE_ENABLE_DELAY_MS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EnablePauseResumeAll()
	{
		m_NavPauseResumeAll.SetEnabled(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FocusFirstLine()
	{
		if (m_aAddonLines.IsEmpty())
			return;
		
		GetGame().GetWorkspace().SetFocusedWidget(m_aAddonLines[0].GetRootWidget());
		
		if (m_ScrollLayout)
			m_ScrollLayout.SetSliderPos(0, 0);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_DownloadManagerListCategory
{
	// Using z order until it's possible to change widget parent
	[Attribute()]
	int m_iZOrder;
	
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EDownloadManagerActionState))]
	EDownloadManagerActionState m_iType;
}