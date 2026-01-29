/**
Displays current session duration of the gamemode
*/	
class SCR_SessionDurationUIComponent : MenuRootSubComponent 
{
	[Attribute("#AR-ValueUnit_Short_Plus", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sSessionDurationFormatting;
	
	protected TextWidget m_wSessionDurationText;	
	
	protected SCR_BaseGameMode m_GameMode;
	
	//------------------------------------------------------------------------------------------------
	protected void SetSessionDurationTime()
	{		
		if (m_GameMode)
			m_wSessionDurationText.SetTextFormat(m_sSessionDurationFormatting, SCR_FormatHelper.GetTimeFormatting(m_GameMode.GetElapsedTime(), ETimeFormatParam.DAYS));
		else //~ Fallback if no gamemode
		{
			ChimeraWorld world = GetGame().GetWorld();
			float elapsedMs = world.GetServerTimestamp().DiffMilliseconds(null);
			m_wSessionDurationText.SetTextFormat(m_sSessionDurationFormatting, SCR_FormatHelper.GetTimeFormatting(elapsedMs * 0.001, ETimeFormatParam.DAYS));
		}
	}
	
	//======================== ATTACH/DEATTACH HANDLER ========================\\
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{		
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!m_GameMode)
		{
			//~ Only show error if replication is running else using replication time is fine
			if (Replication.IsRunning())
				Print("'SCR_SessionDurationUIComponent' could not find 'SCR_BaseGameMode' so will show replication time instead which is not synced with players", LogLevel.WARNING);
		}
		
		m_wSessionDurationText = TextWidget.Cast(w);
		if (!m_wSessionDurationText)
		{
			Print("'SCR_SessionDurationUIComponent' needs to be attached to a 'TextWidget'!", LogLevel.ERROR);
			return;
		}
		
		SetSessionDurationTime();
		
		//~ Called every frame to update session duration
		GetGame().GetCallqueue().CallLater(SetSessionDurationTime, 1000, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//On Destroy
	override void HandlerDeattached(Widget w)
	{
		if (!m_wSessionDurationText)
			return;
		
		GetGame().GetCallqueue().Remove(SetSessionDurationTime);
	}
};