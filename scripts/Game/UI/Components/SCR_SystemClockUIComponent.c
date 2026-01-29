// Script File


/**
Gets the System time of the player's pc and displays it on an UI text element.
*/
class SCR_SystemClockUIComponent : ScriptedWidgetComponent 
{
	//References
	protected TextWidget m_ClockTimeText;

	
	//======================== CLOCK UPDATE ========================\\
	//Sets clock as System clock, Called every seconds
	protected void OnSystemClockUpdate()
	{
		//Get time
		int hour, minute, sec;
		System.GetHourMinuteSecond(hour, minute, sec);
		
		//Set text
		m_ClockTimeText.SetText(SCR_FormatHelper.GetTimeFormattingHoursMinutes(hour, minute));
	}
	

	//======================== ATTACH/DEATTACH HANDLER ========================\\
	//On Init
	override void HandlerAttached(Widget w)
	{
		m_ClockTimeText = TextWidget.Cast(w);

		//Add update each second for system clock
		if (m_ClockTimeText)
		{
			OnSystemClockUpdate();
			GetGame().GetCallqueue().CallLater(OnSystemClockUpdate, 1000, true);
		}
		else 
		{
			Print("System Clock not attached to Text Widget!", LogLevel.ERROR);
		}
	}
	
	
	//On Destroy
	override void HandlerDeattached(Widget w)
	{
		//Remove every second update System Clock
		if (m_ClockTimeText)
		{
			GetGame().GetCallqueue().Remove(OnSystemClockUpdate);
		}
	}

	
};