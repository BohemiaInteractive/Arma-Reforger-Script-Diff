//------------------------------------------------------------------------------------------------
/*!
Radial menu class for handling HUD part of menu.
Displays menu circle and distributes entries into the circle
*/
[BaseContainerProps()]
class SCR_RadialMenuDisplay : SCR_SelectionMenuDisplay
{
	protected const string HINT_BASE = "<action name='%1' hideEmpty='1'/>";
	protected const float CHANGE_CONTENT_OPACITY = 0.4;
	protected const float ENTRY_ANIMATION_SCALE = 1.25;
	protected const float ENTRY_ANIMATION_SPEED = 7.5;

	[Attribute("150", UIWidgets.Slider, "Entries distance from menu center in pixels", "0 1000 1")]
	protected int m_iEntriesRadius;

	[Attribute("0")]
	protected float m_fSelectedIndicatorOffset;

	// Widget name refs
	[Attribute(desc: "Widget name for menu base wrapper that hould all important parts of radial circle")]
	protected string m_sBaseWidget;

	[Attribute("1", desc: "Adjust base size in callback based on invoker from SCR_RadialMenu")]
	protected bool m_bForceSize;
	
	[Attribute("0", desc: "Skips the slower fade animation on entry perform")]
	protected bool m_bFastFadeOnPerform;

	[Attribute( desc: "Widget name for storing entry separator widgets")]
	protected string m_sDividersParent;

	[Attribute( ".edds", desc: "Texture used for creating dividing lines")]
	protected ResourceName m_sDividerTexture;

	[Attribute("0", UIWidgets.Slider, "Dividers distance from menu center in pixels", "0 1000 1")]
	protected int m_iDividersRadius;

	[Attribute("120")]
	protected int m_iDividerSize;

	[Attribute("1", UIWidgets.Slider, "Dividers texture opacity", "0 1 0.01")]
	protected float m_fDividersOpacity;

	[Attribute(desc: "Widget name for storing segment widgets of each entry")]
	protected string m_sSegmentsParent;

	[Attribute(".layout", desc: "Layout used for creating segment. Serves for entry split and colorize", params: "layout")]
	protected ResourceName m_sSegmentLayout;

	[Attribute(desc: "Image widget name for displaying where player is pointing to")]
	protected string m_sSelectorLine;

	[Attribute(desc: "Widget name for poiting selection arrow")]
	protected string m_sSelectorArrow;
	
	[Attribute(desc: "Widget name for selection pointing innner selector")]
	protected string m_sInnerSelector;
	
	[Attribute(desc: "Image widget name for highlighting currently selected entry")]
	protected string m_sSelectedIndicator;

	[Attribute(desc: "Image widget name for selection feedback")]
	protected string m_sSelectedIndicatorFeedback;

	[Attribute(desc: "Text widget name for displaying name of currently selected entry")]
	protected string m_sSelectedName;
	
	[Attribute(desc: "Rich text widget name for displaying key e.g. required for quick use of selected entry")]
	protected string m_sActionHint;

	[Attribute(desc: "Text widget name for displaying entry description or hint")]
	protected string m_sDescription;

	[Attribute(desc: "Widget name with breadcrumbs component to find reference for breadcrumbs")]
	protected string m_sBreadCrumbs;

	[Attribute("ImgInnerBackground", desc: "Name of image widget for inner circle background")]
	protected string m_sInnerBackround;
	
	[Attribute(desc: "Widget holding icon and shadow of crosshair")]
	protected string m_sCrosshairWrap;

	[Attribute(desc: "Front blur widget for animation effects")]
	protected string m_sBlur;
	
	[Attribute(desc: "Widget name for entry hint wrapper")]
	protected string m_sHintWrapper;
	
	[Attribute("325", UIWidgets.Slider, "Hint distance from center in pixels", "0 500 1")]
	protected float m_fHintRadius;
	
	[Attribute("1000", UIWidgets.Slider, "Time to display hint in miliseconds", "0 5000 1")]
	protected int m_iHintDisplayDelay;

	[Attribute("100")]
	protected int m_iCategoryIconRadius;
	
	[Attribute(".layout", desc: "Layout used for number hint", params: "layout")]
	protected ResourceName m_sNumberHintLayout;
	
	[Attribute(desc: "Widget name for storing number hints widgets of each entry")]
	protected string m_sNumberHintParent;
	
	[Attribute("320", UIWidgets.Slider, "Entry number hints distance from center in pixels", "0 500 1")]
	protected float m_fNumberHintRadius;

	protected Widget m_wBase;
	protected Widget m_wDividersParent;
	protected ref array<Widget> m_aDividers = {};
	protected Widget m_wSegmentsParent;
	protected ref array<SCR_RadialMenuSegmentComponent> m_aSegments = {};
	
	protected Widget m_wNumberHints;
	protected ref array<SCR_RadialMenuNumberHintComponent> m_aNumberHints = {};
	
	protected ImageWidget m_wSelectorArrow;
	protected ImageWidget m_wInnerSelector;
	protected ImageWidget m_wSelectorLine;
	protected ImageWidget m_wSelectedIndicator;
	protected ImageWidget m_wSelectedIndicatorFeedback;
	protected TextWidget m_wSelectedName;
	protected TextWidget m_wActionHint;
	protected TextWidget m_wDescription;
	protected ImageWidget m_wInnerBackround;
	protected Widget m_wCrosshairWrap;
	protected BlurWidget m_wBlur;
	
	protected Widget m_wHintWrapper;

	protected SCR_RadialMenu m_RadialMenu;
	protected SCR_BreadCrumbsComponent m_BreadCrumbs;

	protected float m_fSelectorArrowRadius;

	// Values to dynamic
	protected float m_fDynamicEntriesRadius;
	protected float m_fDynamicDividerSize;
	protected float m_fDynamicDividersRadius;
	protected float m_SizeRatio;

	protected int m_iPrevCategoryLevel = 0;

	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Setup menu
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);

		// Find widgets
		m_wBase = GetRootWidget().FindAnyWidget(m_sBaseWidget);
		m_wDividersParent = GetRootWidget().FindAnyWidget(m_sDividersParent);
		m_wSegmentsParent = GetRootWidget().FindAnyWidget(m_sSegmentsParent);

		m_wSelectorArrow = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sSelectorArrow));
		m_wInnerSelector = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sInnerSelector));
		m_wSelectorLine = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sSelectorLine));
		m_wSelectedIndicator = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sSelectedIndicator));
		m_wSelectedIndicatorFeedback = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sSelectedIndicatorFeedback));

		m_wSelectedName = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sSelectedName));
		m_wDescription = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sDescription));

		m_wActionHint = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sActionHint));

		m_wInnerBackround = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sInnerBackround));
		m_wCrosshairWrap = GetRootWidget().FindAnyWidget(m_sCrosshairWrap);
		m_wBlur = BlurWidget.Cast(GetRootWidget().FindAnyWidget(m_sBlur));

		m_wHintWrapper = GetRootWidget().FindAnyWidget(m_sHintWrapper);
		
		m_wNumberHints = GetRootWidget().FindAnyWidget(m_sNumberHintParent);
		
		// Find breadcrubms
		Widget breadCrumbs = GetRootWidget().FindAnyWidget(m_sBreadCrumbs);
		if (breadCrumbs)
			m_BreadCrumbs = SCR_BreadCrumbsComponent.Cast(breadCrumbs.FindHandler(SCR_BreadCrumbsComponent));

		// Selection
		if (m_wSelectedIndicator)
			m_wSelectedIndicator.SetVisible(false);

		if (m_wSelectedIndicatorFeedback)
		{
			m_wSelectedIndicatorFeedback.SetVisible(true);
			m_wSelectedIndicatorFeedback.SetOpacity(0);
			AnimateWidget.StopAnimation(m_wSelectedIndicatorFeedback, WidgetAnimationOpacity);
		}

		if (m_wBlur)
			m_wBlur.SetOpacity(0);

		// Details
		if (m_wDescription)
			m_wDescription.SetVisible(false);

		if (m_wActionHint)
			m_wActionHint.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		super.DisplayStopDraw(owner);

		if (m_RadialMenu)
		{
			m_RadialMenu.GetOnDisplaySizeChange().Remove(OnDisplaySizeChange);
			m_RadialMenu.GetOnSetActionHint().Remove(OnSetActionHint);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_bShown || !m_RadialMenu)
			return;
		
		VisualizeSelection(m_RadialMenu.GetPointingAngle());
			
		// Show hints 
		m_wNumberHints.SetVisible(
			m_RadialMenu && 
			m_RadialMenu.GetUseQuickActions() && 
			GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}

	//------------------------------------------------------------------------------------------------
	override void SetupMenu(SCR_SelectionMenu menu)
	{
		if (m_Menu)
			m_Menu.GetOnOpenCategory().Remove(OnMenuOpenCategory);

		if (m_RadialMenu)
		{
			m_RadialMenu.GetOnDisplaySizeChange().Remove(OnDisplaySizeChange);
			m_RadialMenu.GetOnSetActionHint().Remove(OnSetActionHint);
		}
		
		super.SetupMenu(menu);
				
		m_RadialMenu = SCR_RadialMenu.Cast(m_Menu);
		if (!m_RadialMenu)
			return;
		
		m_RadialMenu.GetOnDisplaySizeChange().Insert(OnDisplaySizeChange);
		m_RadialMenu.GetOnSetActionHint().Insert(OnSetActionHint);
		m_RadialMenu.GetOnOpenCategory().Insert(OnMenuOpenCategory);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuOpen()
	{
		super.OnMenuOpen();
		SetupSelectionVisuals();

		if (m_wSelectedIndicatorFeedback)
			m_wSelectedIndicatorFeedback.SetOpacity(0);

		// Crosshair
		SCR_RadialMenuControllerInputs radialInputs;
		if (m_RadialMenu)
			radialInputs = SCR_RadialMenuControllerInputs.Cast(m_RadialMenu.GetControllerInputs());
		
		if (radialInputs)
		{
			ShowCrosshair(radialInputs.m_bShowCrosshair);
			ShowInnerBackground(radialInputs.m_bShowInnerBackground);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnShownFinished(Widget w, float targetOpacity, WidgetAnimationOpacity anim = null)
	{
		super.OnShownFinished(w, targetOpacity, anim);
		
		if (m_Menu && m_Menu.IsOpened())
			SetupMenuCenter();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupMenuCenter()
	{
		if (!m_RadialMenu || !m_wBase)
			return;
		
		// Screen pos
		float posX, posY;
		m_wBase.GetScreenPos(posX, posY);

		float sizeX, sizeY;
		m_wBase.GetScreenSize(sizeX, sizeY);
		
		float x = posX + sizeX * 0.5;
		float y = posY + sizeY * 0.5;
		m_RadialMenu.SetMenuCenterPos(Vector(x, y, 0));
		GetGame().GetInputManager().SetCursorPosition(x, y);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMenuClose()
	{
		if (!m_RadialMenu)
			return;
		
		if (m_RadialMenu.GetEntryPerformed() && !m_bFastFadeOnPerform)
			Show(false, UIConstants.FADE_RATE_DEFAULT, EAnimationCurve.EASE_IN_EXPO);
		else
			Show(false, UIConstants.FADE_RATE_FAST);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMenuEntriesUpdate(SCR_SelectionMenu menu, array<ref SCR_SelectionMenuEntry> entries)
	{
		if (!m_Menu.IsOpened())
			return;
		
		super.OnMenuEntriesUpdate(menu, entries);
		
		SetupSelectionVisuals();
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetupEntryWidget(notnull SCR_SelectionMenuEntry entry, notnull Widget widget, int id)
	{
		float angle = SetupFrameSlotPosition(widget, id, m_fDynamicEntriesRadius, m_RadialMenu.GetEntriesAngleDistance());

		// Set icon size
		SCR_SelectionMenuEntryIconComponent iconEntryCmp = SCR_SelectionMenuEntryIconComponent.Cast(
			widget.FindHandler(SCR_SelectionMenuEntryIconComponent));
		
		SCR_SelectionMenuCategoryEntry category = SCR_SelectionMenuCategoryEntry.Cast(entry);

		if (iconEntryCmp)
		{
			float entrySize = iconEntryCmp.GetOriginalSize() * m_SizeRatio;
			iconEntryCmp.SetLayoutSize(entrySize);
		}
		
		// Add elements for entry
		CreateEntrySegment(entry, widget, id);
		
		// Add number hint
		if (m_RadialMenu && m_RadialMenu.GetUseQuickActions())
			CreateNumberHint(id);
	}

	//------------------------------------------------------------------------------------------------
	//! Create number hint to the enter to state quick number action to use
	protected void CreateNumberHint(int id)
	{
		// up to 9 
		if (id >= SCR_RadialMenuControllerInputs.MAX_HINTS)
			return;
		
		Widget hint = GetGame().GetWorkspace().CreateWidgets(m_sNumberHintLayout, m_wNumberHints);
		if (!hint)
			return;

		// Position
		float angle = m_RadialMenu.GetEntriesAngleDistance() * id * Math.DEG2RAD;
		vector vec = GetPointOnCircle(m_fNumberHintRadius, angle);
		FrameSlot.SetPos(hint, vec[0], vec[1]);
		
		// Id 
		SCR_RadialMenuNumberHintComponent hintCmp = SCR_RadialMenuNumberHintComponent.Cast(hint.FindHandler(SCR_RadialMenuNumberHintComponent));
		if (!hintCmp)
			return;
		
		hintCmp.SetMessages((id + 1).ToString(), "");
		m_aNumberHints.Insert(hintCmp);
	}
	
	//------------------------------------------------------------------------------------------------
	//! React on selected entry change
	override protected void OnMenuEntrySelected(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry, int id)
	{
		// Selection
		if (m_wSelectedIndicator)
		{
			m_wSelectedIndicator.SetVisible(entry != null && entry.IsEnabled());

			float angle = m_RadialMenu.GetEntriesAngleDistance() * id;
			float offset = m_fSelectedIndicatorOffset - m_RadialMenu.GetEntriesAngleDistance() * 0.5;
			m_wSelectedIndicator.SetRotation(angle + offset);

			if (m_wSelectedIndicatorFeedback)
				m_wSelectedIndicatorFeedback.SetRotation(angle + offset);
		}

		// Info
		if (m_wSelectedName)
		{
			m_wSelectedName.SetVisible(entry != null);

			if (entry)
				m_wSelectedName.SetText(entry.GetName());
		}

		// Visualize segments selection
		if (m_aSegments.IsIndexValid(m_iLastSelectedId))
		{
			if (m_LastSelectedEntry && m_LastSelectedEntry.IsEnabled())
				m_aSegments[m_iLastSelectedId].GetBackgroundWidget().SetVisible(true);
		}

		if (m_aSegments.IsIndexValid(id))
			m_aSegments[id].GetBackgroundWidget().SetVisible(false);
		
		// // Update number hints  
		if (m_aNumberHints.IsIndexValid(m_iLastSelectedId))
			m_aNumberHints[m_iLastSelectedId].Highlight(false);
		
		if (m_aNumberHints.IsIndexValid(id))	
			m_aNumberHints[id].Highlight(true);
		

		// Update selection
		m_LastSelectedEntry = entry;
		m_iLastSelectedId = id;

		// Display description
		if (m_wDescription)
		{
			m_wDescription.SetVisible(entry && !entry.GetDescription().IsEmpty());

			if (entry)
				m_wDescription.SetText(entry.GetDescription());
		}

		// Display action hint
		if (m_wActionHint)
			m_wActionHint.SetVisible(entry && !m_wActionHint.GetText().IsEmpty());
		
		// Place hint
		SetupEntryHint("", id);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuEntryPerform(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry)
	{
		if (m_wSelectedIndicatorFeedback)
		{
			m_wSelectedIndicatorFeedback.SetOpacity(0);
			
			if (!AnimateWidget.IsAnimating(m_wSelectedIndicatorFeedback))
				AnimateWidget.Opacity(m_wSelectedIndicatorFeedback, CHANGE_CONTENT_OPACITY, UIConstants.FADE_RATE_FAST);
		}
			
		// Animate entry 
		if (!entry || !entry.GetEntryComponent())
			return;
		
		Widget entryW = entry.GetEntryComponent().GetRootWidget().GetChildren();
				
		// Prevent starting animation again
		if (AnimateWidget.IsAnimating(entryW))
			return;
		
		float originalSize = entry.GetEntryComponent().GetAdjustedSize(); 
		float size[2] = {originalSize * ENTRY_ANIMATION_SCALE, originalSize * ENTRY_ANIMATION_SCALE};
		
		WidgetAnimationFrameSize anim = AnimateWidget.Size(entryW, size, ENTRY_ANIMATION_SPEED);
		anim.SetRepeat(true);
		anim.GetOnCycleCompleted().Insert(OnEntrySizeAnimCycleCompleted);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntrySizeAnimCycleCompleted(WidgetAnimationBase anim)
	{
		anim.SetRepeat(false);
		anim.Stop();
		
		AnimateWidget.Opacity(m_wSelectedIndicatorFeedback, 0, UIConstants.FADE_RATE_FAST);
		
		anim.GetOnCycleCompleted().Remove(OnEntrySizeAnimCycleCompleted);
	}

	//------------------------------------------------------------------------------------------------
	//! Clear segments and lines
	override protected void ClearEntryWidgets()
	{
		super.ClearEntryWidgets();

		if (m_aSegments.Count() != m_aDividers.Count())
		{
			DebugPrint("ClearEntryWidgets", "Different entries");
		}

		// Clear segments and dividers
		for (int i = 0, count = m_aSegments.Count(); i < count; i++)
		{
			m_aSegments[i].GetRootWidget().RemoveFromHierarchy();
			m_aDividers[i].RemoveFromHierarchy();
			
			if (m_aNumberHints.IsIndexValid(i))
				m_aNumberHints[i].GetRoot().RemoveFromHierarchy();
		}

		m_aSegments.Clear();
		m_aDividers.Clear();
		m_aNumberHints.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Vector X Y positions from radisu (distance) and degrees (angle)
	protected vector GetPointOnCircle(float radius, float degrees)
	{
		// - half PI because 90 deg offset left
		return Vector(
			radius * Math.Cos(degrees - 0.5 * Math.PI),
			radius * Math.Sin(degrees - 0.5 * Math.PI),
			0.0);
	}

	//------------------------------------------------------------------------------------------------
	protected void VisualizeSelection(float selectionAngle)
	{
		// Check widgets
		if (m_wSelectorArrow)
			m_wSelectorArrow.SetVisible(!m_RadialMenu.IsPointingToCenter());
		
		if (m_wInnerSelector)
			m_wInnerSelector.SetVisible(!m_RadialMenu.IsPointingToCenter());

		m_wSelectorLine.SetVisible(!m_RadialMenu.IsPointingToCenter());
		m_wSelectorLine.SetRotation(selectionAngle - 180);
		
		// Gamepad
		if (!m_wSelectorArrow)
			return;

		if (m_fSelectorArrowRadius == 0 && m_wSelectorArrow)
			m_fSelectorArrowRadius = FrameSlot.GetPosY(m_wSelectorArrow) * m_SizeRatio;

		if (m_fSelectorArrowRadius == 0)
			return;

		// Check center
		vector vec = GetPointOnCircle(-m_fSelectorArrowRadius, selectionAngle * Math.DEG2RAD);
		FrameSlot.SetPos(m_wSelectorArrow, vec[0], vec[1]);

		m_wSelectorArrow.SetRotation(selectionAngle);
		
		if (m_wInnerSelector)
		{
			m_wInnerSelector.SetRotation(selectionAngle);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Setup widget position and angle in frame slot
	protected float SetupFrameSlotPosition(out notnull Widget widget, int id, float distance, float angle, float angleOffset = 0)
	{
		// Calculate
		float degs = angle * id + angleOffset;
		vector point = GetPointOnCircle(distance, degs * Math.DEG2RAD);

		// Set position - set entry layout to menu center to more easily place entry around the circle
		FrameSlot.SetPos(widget, point[0], point[1]);
		FrameSlot.SetAlignment(widget, 0.5, 0.5);
		FrameSlot.SetAnchorMin(widget, 0.5, 0.5);
		FrameSlot.SetAnchorMax(widget, 0.5, 0.5);

		return degs;
	}

	//------------------------------------------------------------------------------------------------
	//! Add to circle segments filling entry area
	protected void CreateEntrySegment(notnull SCR_SelectionMenuEntry entry, notnull Widget widget, int id)
	{
		if (!m_wSegmentsParent || m_sSegmentLayout.IsEmpty())
		{
			DebugPrint("CreateEntrySegment", "Can't create segments!");
			return;
		}

		Widget segment = GetGame().GetWorkspace().CreateWidgets(m_sSegmentLayout, m_wSegmentsParent);
		SCR_RadialMenuSegmentComponent segmentCmp = SCR_RadialMenuSegmentComponent.Cast(segment.FindHandler(SCR_RadialMenuSegmentComponent));

		float angleDist = m_RadialMenu.GetEntriesAngleDistance();
		float range = angleDist / 360;
		// distance * list position - half -> to be in entry center
		float rot = angleDist * id - angleDist * 0.5;
		bool isCategory = SCR_SelectionMenuCategoryEntry.Cast(entry);
		
		segmentCmp.SetupSegment(range, angleDist * id, isCategory);
		
		// Register
		m_aSegments.Insert(segmentCmp);
		
		// Add divider
		if (m_wDividersParent && !m_sDividerTexture.IsEmpty())
		{
			CreateDivider(id);
		}

		SCR_SelectionMenuEntryComponent comp = SCR_SelectionMenuEntryComponent.Cast(
			widget.FindHandler(SCR_SelectionMenuEntryComponent));

		// Visual setup
		VisualizeEnableEntry(comp, id, entry.IsEnabled());
	}

	//------------------------------------------------------------------------------------------------
	//! Add divider after entry segments
	protected void CreateDivider(int id)
	{
		if (!m_wDividersParent || m_sDividerTexture.IsEmpty())
		{
			DebugPrint("CreateDivider", "Can't create segments!");
			return;
		}

		Widget divider = GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID,
			WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.BLEND | WidgetFlags.INHERIT_CLIPPING,
			Color.FromInt(Color.WHITE),
			0,
			m_wDividersParent
		);

		// Setup divider widget
		ImageWidget imgDivider = ImageWidget.Cast(divider);

		imgDivider.LoadImageTexture(0, m_sDividerTexture);
		imgDivider.SetOpacity(m_fDividersOpacity);
		FrameSlot.SetSize(divider, m_fDynamicDividerSize, m_fDynamicDividerSize);

		float angleDist = m_RadialMenu.GetEntriesAngleDistance();
		float angle = SetupFrameSlotPosition(divider, id, m_fDynamicDividersRadius, angleDist, angleDist * 0.5);

		imgDivider.SetRotation(angle);

		// Register
		m_aDividers.Insert(divider)
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupEntryHint(string text, int id)
	{
		if (!m_wHintWrapper)
			return;
		
		m_wHintWrapper.SetOpacity(0);
		AnimateWidget.StopAnimation(m_wHintWrapper, WidgetAnimationOpacity);
		
		GetGame().GetCallqueue().Remove(DisplayEntryHint);
		// Call later to display hint only if player stays longer on single entry 
		// Intend is to prevent cluttering screen with hints if playuer knows immidiatelly what action does
		GetGame().GetCallqueue().CallLater(DisplayEntryHint, m_iHintDisplayDelay);
		
		// Set position 
		float angle = m_RadialMenu.GetEntriesAngleDistance() * id * Math.DEG2RAD;
		vector vec = GetPointOnCircle(m_fHintRadius, angle);
		
		float width = FrameSlot.GetSizeX(m_wHintWrapper);
		vec[0] = vec[0] + Math.Sin(angle) * width * 0.5;
		
		// Apply to frame widget 
		FrameSlot.SetPos(m_wHintWrapper, vec[0], vec[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisplayEntryHint()
	{
		AnimateWidget.Opacity(m_wHintWrapper, 1, UIConstants.FADE_RATE_DEFAULT);
	}
	
	//------------------------------------------------------------------------------------------------
	void VisualizeEnableEntry(notnull SCR_SelectionMenuEntryComponent entry, int id, bool enable)
	{
		if (!m_aSegments.IsIndexValid(id))
			return;

		m_aSegments[id].GetBackgroundWidget().SetVisible(enable);
		entry.SetEnabled(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback reacting to changing size from SCR_RadialMenu
	protected void OnDisplaySizeChange(SCR_RadialMenu menu, float size)
	{
		// Ratio to menu large size
		m_SizeRatio = 1;

		if (m_bForceSize)
			m_SizeRatio = size / menu.SIZE_LARGE;

		// Setup sizes
		m_fDynamicEntriesRadius = m_iEntriesRadius * m_SizeRatio;
		m_fDynamicDividerSize = m_iDividerSize * m_SizeRatio;
		m_fDynamicDividersRadius = m_iDividersRadius * m_SizeRatio;

		if (m_bForceSize)
			FrameSlot.SetSize(m_wBase, size, size);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback to display action hint icon
	protected void OnSetActionHint(SCR_RadialMenu menu, string action)
	{
		// Setup widget
		if (!action.IsEmpty())
			m_wActionHint.SetTextFormat(HINT_BASE, action);
		else
			m_wActionHint.SetTextFormat(string.Empty);

		m_wActionHint.SetVisible(m_RadialMenu.GetSelectionEntry() && !m_wActionHint.GetText().IsEmpty());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuOpenCategory(SCR_SelectionMenu menu, SCR_SelectionMenuCategoryEntry category, int level)
	{
		if (!category)
		{
			if (m_BreadCrumbs)
				m_BreadCrumbs.Clear();

			m_iPrevCategoryLevel = 0;

			return;
		}

		if (m_BreadCrumbs)
		{
			if (m_iPrevCategoryLevel < level)
				m_BreadCrumbs.AddBreadCrumbElement(category);
			else
				m_BreadCrumbs.RemoveLastBreadCrumbElement();
		}

		// Update level
		m_iPrevCategoryLevel = level;
		
		// Call update later to update it once fadeout is done so fadein can start after animation is done
		GetGame().GetCallqueue().CallLater(LateMenuEntriesUpdate, 1000 / UIConstants.FADE_RATE_SUPER_FAST);

		if (m_wRoot)
			m_wRoot.SetOpacity(CHANGE_CONTENT_OPACITY);

		if (m_wBlur)
			m_wBlur.SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void LateMenuEntriesUpdate()
	{
		SetupSelectionVisuals();

		if (m_wRoot)
			AnimateWidget.Opacity(m_wRoot, 1, UIConstants.FADE_RATE_SUPER_FAST);

		if (m_wBlur)
			m_wBlur.SetOpacity(0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ChangeCategoryOpen(SCR_SelectionMenu menu, SCR_SelectionMenuCategoryEntry category, int level)
	{
		if (!category)
		{
			if (m_BreadCrumbs)
				m_BreadCrumbs.Clear();

			m_iPrevCategoryLevel = 0;

			return;
		}

		if (m_BreadCrumbs)
		{
			if (m_iPrevCategoryLevel < level)
				m_BreadCrumbs.AddBreadCrumbElement(category);
			else
				m_BreadCrumbs.RemoveLastBreadCrumbElement();
		}

		// Update level
		m_iPrevCategoryLevel = level;

		Show(true, UIConstants.FADE_RATE_FAST);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupSelectionVisuals()
	{
		if (!m_wSelectedIndicator || !m_wSelectorLine)
			return;

		// Set selection segment from angles to percents
		float range = m_RadialMenu.GetEntriesAngleDistance() / 360;

		m_wSelectedIndicator.SetMaskProgress(range);

		m_wSelectorLine.SetMaskRange(range * 0.5);
		m_wSelectorLine.SetMaskTransitionWidth(range * 0.5);

		if (m_wSelectedIndicatorFeedback)
			m_wSelectedIndicatorFeedback.SetMaskProgress(range);
	}

	//------------------------------------------------------------------------------------------------
	void ShowInnerBackground(bool show)
	{
		if (m_wInnerBackround)
			m_wInnerBackround.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowCrosshair(bool show)
	{
		if (m_wCrosshairWrap)
			m_wCrosshairWrap.SetVisible(show);
	}
};
