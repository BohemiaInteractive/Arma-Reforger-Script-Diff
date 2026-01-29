//#define FAKE_PLAYER //--- Allow placing AIs which are pretending to be players. Used for video capture.

class SCR_PlayerEditableEntityUIComponent : SCR_BaseEditableEntityUIComponent
{
	[Attribute("1 1 1 1")]
	protected ref Color m_ColorPossessed;
	
	[Attribute("Name")]
	protected string m_sPlayerNameWidgetName;
	
	[Attribute("PlatformIcon")]
	protected string m_sPlatformIconWidgetName;
	
	//------------------------------------------------------------------------------------------------
	protected void SetName(TextWidget nameWidget, int playerID)
	{
		string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);
		if (playerName.IsEmpty())
			return;

		nameWidget.SetText(playerName);
		GetGame().GetCallqueue().Remove(SetName);
	}
	
	override void OnShownOffScreen(bool offScreen)
	{
		Widget nameWidget = GetWidget().FindAnyWidget(m_sPlayerNameWidgetName);
		if (!nameWidget)
			return;

		if (offScreen)
			nameWidget.SetOpacity(0);
		else
			nameWidget.SetOpacity(1);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		Widget widget = GetWidget();
		if (!widget)
			return;
		
		TextWidget nameWidget = TextWidget.Cast(widget.FindAnyWidget(m_sPlayerNameWidgetName));
		if (!nameWidget)
			return;
		
		int playerID;
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
		if (delegate)
		{
			playerID = delegate.GetPlayerID();
		}
		else
		{
			SCR_PlayersManagerEditorComponent playersManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent));
			if (playersManager)
			{
				playerID = playersManager.GetPlayerID(entity);
				if (playersManager.IsPossessed(entity))
					widget.SetColor(m_ColorPossessed);
			}
		}

		if (playerID == 0)
			return;

		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		//--- Assign name after it's been initialized (ToDo: Better solution? Callback?)
		if (game.GetCallqueue())
			game.GetCallqueue().CallLater(SetName, 0, true, nameWidget, playerID);
		
		ImageWidget platformImage = ImageWidget.Cast(widget.FindAnyWidget(m_sPlatformIconWidgetName));
		if (platformImage)
			SCR_PlayerController.Cast(game.GetPlayerController()).SetPlatformImageTo(playerID, platformImage);
		
		SCR_EditableEntitySceneSlotUIComponent sceneSlot = SCR_EditableEntitySceneSlotUIComponent.Cast(slot);
		if (sceneSlot && sceneSlot.GetOffScreenWidget())
			sceneSlot.GetOffScreenWidget().SetColor(widget.GetColor());
	}
}
