//#define FAKE_PLAYER //--- Allow placing AIs which are pretending to be players. Used for video capture.

class SCR_PlayerEditableEntityUIComponent : SCR_BaseEditableEntityUIComponent
{
	[Attribute("1 1 1 1")]
	protected ref Color m_ColorPossessed;
	
	[Attribute("Name")]
	protected string m_sPlayerNameWidgetName;
	
	[Attribute("PlatformIcon")]
	protected string m_sPlatformIconWidgetName;
	
	protected SCR_EditablePlayerDelegateComponent m_EditablePlayerDelegateComp;

	//------------------------------------------------------------------------------------------------
	protected void AttemptSetName(bool success)
	{
		Widget widget = GetWidget();
		if (!widget)
			return;

		TextWidget nameWidget = TextWidget.Cast(widget.FindAnyWidget(m_sPlayerNameWidgetName));
		if (!nameWidget)
			return;

		int playerID;
		if (m_EditablePlayerDelegateComp)
			playerID = m_EditablePlayerDelegateComp.GetPlayerID();

		if (playerID <= 0)
			return;

		if(SetName(nameWidget, playerID))
			PlayerManager.s_OnPlayerNameCacheUpdateInvoker.Remove(AttemptSetName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool SetName(TextWidget nameWidget, int playerID)
	{
		string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);
		if (playerName.IsEmpty())
			return false;

		nameWidget.SetText(playerName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
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
		m_EditablePlayerDelegateComp = SCR_EditablePlayerDelegateComponent.Cast(entity);
		if (m_EditablePlayerDelegateComp)
		{
			playerID = m_EditablePlayerDelegateComp.GetPlayerID();
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

		if (playerID <= 0)
			return;

		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;

		ImageWidget platformImage = ImageWidget.Cast(widget.FindAnyWidget(m_sPlatformIconWidgetName));
		if (platformImage)
			SCR_PlayerController.Cast(game.GetPlayerController()).SetPlatformImageTo(playerID, platformImage);

		SCR_EditableEntitySceneSlotUIComponent sceneSlot = SCR_EditableEntitySceneSlotUIComponent.Cast(slot);
		if (sceneSlot && sceneSlot.GetOffScreenWidget())
			sceneSlot.GetOffScreenWidget().SetColor(widget.GetColor());

		if(!SetName(nameWidget, playerID))
			PlayerManager.s_OnPlayerNameCacheUpdateInvoker.Insert(AttemptSetName);
	}
}
