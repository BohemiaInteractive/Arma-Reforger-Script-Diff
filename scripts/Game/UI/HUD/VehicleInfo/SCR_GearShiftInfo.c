// Will print every change to gear shift; CLI -scrDefine DEBUG_GEARSHIFT
//#define DEBUG_GEARSHIFT

class SCR_GearShiftInfo: SCR_BaseVehicleInfo
{	
	protected CarControllerComponent m_pCarController;

	protected SCR_InfoDisplayExtended m_pParentDisplayExtended;
	
	// Widgets
	ref SCR_VehicleGearShiftWidgets m_Widgets;
	
	protected int m_iFutureGear;
	protected int m_iCurrentGear;
	protected EVehicleDrivingAssistanceMode m_eDrivingAssistance;
	protected bool m_bAutomaticGearbox;
	
	//------------------------------------------------------------------------------------------------
	//! Present proper symbol for selected gear
	string GetGearSymbol(bool automatic, int gear, int gearCount)
	{
		if (gear < EVehicleGearboxGear.REVERSE)
			return " ";
		else if (gear >= gearCount)
			return " ";
		else if (gear == EVehicleGearboxGear.REVERSE)
			return "R";
		else if (gear == EVehicleGearboxGear.NEUTRAL)
			return "N";
		else if (automatic)
			return "D";
		else
			return (gear-1).ToString();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Construct the string to be displayed in the window
	void GetGearboxModeDisplay(bool automatic, int futureGear, int currentGear)
	{
		int gearCount = 0;

		VehicleWheeledSimulation simulation = m_pCarController.GetSimulation();
		if (simulation)
			gearCount = simulation.GearboxGearsCount();
	
		if (automatic)
		{
			futureGear = EVehicleGearboxGear.NEUTRAL;
			currentGear = Math.ClampInt(currentGear, EVehicleGearboxGear.REVERSE, EVehicleGearboxGear.FORWARD);
		}
		else
		{
			futureGear = Math.ClampInt(futureGear, EVehicleGearboxGear.REVERSE, gearCount);
		}
		
		int lowerGear = Math.ClampInt(futureGear - 1, EVehicleGearboxGear.REVERSE, gearCount);
		int middleGear = lowerGear + 1;
		int upperGear = middleGear + 1;
		
		string lower = GetGearSymbol(automatic, lowerGear, gearCount);
		string future = GetGearSymbol(automatic, middleGear, gearCount);
		string upper = GetGearSymbol(automatic, upperGear, gearCount);
		
		m_Widgets.m_LowerSelectedText.SetText(lower);
		m_Widgets.m_LowerText.SetText(lower);
		m_Widgets.m_MiddleSelectedText.SetText(future);
		m_Widgets.m_MiddleText.SetText(future);
		m_Widgets.m_UpperSelectedText.SetText(upper);
		m_Widgets.m_UpperText.SetText(upper);

		m_Widgets.m_LowerSelected.SetVisible(currentGear == lowerGear);
		m_Widgets.m_Lower.SetVisible(currentGear != lowerGear);
		m_Widgets.m_MiddleSelected.SetVisible(currentGear == middleGear);
		m_Widgets.m_Middle.SetVisible(currentGear != middleGear);
		m_Widgets.m_UpperSelected.SetVisible(currentGear == upperGear);
		m_Widgets.m_Upper.SetVisible(currentGear != upperGear);
		
		#ifdef DEBUG_GEARSHIFT
		string spacer1 = " ";
		string spacer2 = " ";
		string spacer3 = " ";
		string spacer4 = " ";
		
		if (currentGear == lowerGear)
		{
			spacer1 = "[";
			spacer2 = "]";
		}
		else if (currentGear == middleGear)
		{
			spacer2 = "[";
			spacer3 = "]";
		}
		else if (currentGear == upperGear)
		{
			spacer3 = "[";
			spacer4 = "]";
		}
		
		PrintFormat("%1%2%3%4%5%6%7", spacer1, lower, spacer2, future, spacer3, upper, spacer4);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	private void ScalePaddings(Widget widget, float scale)
	{
		if (!widget)
			return;
		
		Widget parent = widget.GetParent();

		if (!(OverlayWidget.Cast(parent) || HorizontalLayoutWidget.Cast(parent) || VerticalLayoutWidget.Cast(parent)))
			return;		
				
		float left;
		float top;
		float right;
		float bottom;
		
		AlignableSlot.GetPadding(widget, left, top, right, bottom);
		AlignableSlot.SetPadding(widget, left * scale, top * scale, right * scale, bottom * scale);
	}	
		
	//------------------------------------------------------------------------------------------------	
	override void Scale(TextWidget widget, float scale)
	{
		if (!widget)
			return;

		float width;
		float height;
		
		widget.GetTextSize(width, height);
		
		height = Math.Round(height * scale);
		int size = height;
		
		widget.SetExactFontSize(size);
	}
	
	//------------------------------------------------------------------------------------------------	
	private void ScaleChildren(Widget widget)
	{
        Widget child = widget.GetChildren();
		
        while (child)
        {
			ScaleChildren(child);

			ScalePaddings(child, m_fWidgetScale);
					
			TextWidget text = TextWidget.Cast(child);
			ImageWidget image = ImageWidget.Cast(child);
			
			if (text)
				Scale(text, m_fWidgetScale);
			
			if (image)
				Scale(image, m_fWidgetScale);				
				
            child = child.GetSibling();
        }
	}			
	
	//------------------------------------------------------------------------------------------------
	//! Update widget
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot)
			return;
		
		if (!m_pCarController)
			return;
			
		EVehicleDrivingAssistanceMode drivingAssistance = CarControllerComponent.GetDrivingAssistanceMode();
		
		bool assistanceChanged = m_eDrivingAssistance != drivingAssistance;
		
		if (assistanceChanged)
		{
			#ifdef DEBUG_GEARSHIFT
			PrintFormat("Gear assistance changed from %1 -> %2", m_eDrivingAssistance, drivingAssistance);
			#endif			
			
			m_eDrivingAssistance = drivingAssistance;
			
			bool show = drivingAssistance != EVehicleDrivingAssistanceMode.FULL;
			
			Show(show);
			
			if (m_pParentDisplayExtended)
				m_pParentDisplayExtended.Show(show);
			
			if (!show)			
				return;
		}
			
		bool automatic = drivingAssistance == EVehicleDrivingAssistanceMode.PARTIAL || m_pCarController.HasAutomaticGearbox();

		bool automaticChanged = m_bAutomaticGearbox != automatic;
		
		if (automaticChanged)
		{
			#ifdef DEBUG_GEARSHIFT
			PrintFormat("Gearbox manual vs. automatic changed! automatic: %1", automaticChanged);
			#endif	
			
			m_bAutomaticGearbox = automatic;	
		}
		
		int futureGear = m_pCarController.GetFutureGear();
		int currentGear = m_pCarController.GetCurrentGear();
				
		// Prevent unneeded execution and UI updates, if gears didn't change
		if (!assistanceChanged && futureGear == m_iFutureGear && currentGear == m_iCurrentGear)
			return;
		
		m_iFutureGear = futureGear;
		m_iCurrentGear = currentGear;
		
		GetGearboxModeDisplay(automatic, futureGear, currentGear);
	}

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// Terminate if widget already exists
		if (m_wRoot)
			return false;
		
		// Should probably kill the display if there is no controller
		if (!m_pCarController)
			return false;
		
		// Fallback to avoid the need to fill-in always the same layout filename
		if (m_LayoutPath == "")
			m_LayoutPath = "{5FBB1623E3CD1DF2}UI/layouts/HUD/VehicleInfo/VehicleGearShift.layout";
		
		if (m_pParentDisplay)
			m_pParentDisplayExtended = SCR_InfoDisplayExtended.Cast(m_pParentDisplay);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Create the UI
	override void DisplayStartDraw(IEntity owner)
	{				
		if (!m_wRoot)
			return;

		m_Widgets = new SCR_VehicleGearShiftWidgets();
		m_Widgets.Init(m_wRoot);		
		
		ScaleChildren(m_wRoot);
		
		m_iFutureGear = -1;
		m_iCurrentGear = -1;
		m_eDrivingAssistance = -1;
		m_bAutomaticGearbox = false;		
	}

	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		
		m_pCarController = CarControllerComponent.Cast(owner.FindComponent(CarControllerComponent));
	}
};