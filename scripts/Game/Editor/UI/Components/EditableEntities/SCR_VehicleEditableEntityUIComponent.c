class SCR_VehicleEditableEntityUIComponent : SCR_BaseEditableEntityUIComponent
{
	[Attribute(defvalue: "Name")]
	protected string m_sNameTextWidgetName;

	[Attribute(defvalue: "#AR-Editor_PlayerVehicle_Name")]
	protected string m_sVehicleNameText;

	protected RichTextWidget m_wNameTextWidget;

	protected SCR_EditableVehicleComponent m_VehicleComponent;
	protected SCR_EditableCharacterComponent m_VehicleOwningPlayer;
	protected BaseWorld m_World;

	protected SCR_RenderedEditableEntityFilter m_RenderFilter;

	//------------------------------------------------------------------------------------------------
	//!
	void DetermineNameVisibility()
	{
		if (!m_World || !m_VehicleComponent || !m_VehicleOwningPlayer || !m_wNameTextWidget || !m_RenderFilter)
			return;

		vector cameraMatrix[4];
		vector playerPosition;
		float playerMaxViewDistance = m_VehicleOwningPlayer.GetMaxDrawDistanceSq();

		m_VehicleOwningPlayer.GetPos(playerPosition);
		m_World.GetCurrentCamera(cameraMatrix);

		float distanceSquared = vector.DistanceSq(playerPosition, cameraMatrix[3]);

		if (distanceSquared < playerMaxViewDistance * m_RenderFilter.GetCameraDisCoef())
			m_wNameTextWidget.SetVisible(false);
		else
			m_wNameTextWidget.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		m_wNameTextWidget = RichTextWidget.Cast(GetWidget().FindAnyWidget(m_sNameTextWidgetName));
		if (!m_wNameTextWidget)
		{
			Print("SCR_VehicleEditableEntityUIComponent: Failed to find Name Widget: " + m_sNameTextWidgetName, LogLevel.ERROR);
			return;
		}

		m_World = GetGame().GetWorld();
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (entitiesManager)
			m_RenderFilter = SCR_RenderedEditableEntityFilter.Cast(entitiesManager.GetFilter(EEditableEntityState.RENDERED));

		OnRefresh(slot);
	}

	//------------------------------------------------------------------------------------------------
	override void OnRefresh(SCR_EditableEntityBaseSlotUIComponent slot)
	{
		if (!slot)
			return;

		if (!m_VehicleComponent)
		{
			m_VehicleComponent = SCR_EditableVehicleComponent.Cast(slot.GetEntity());
			if (!m_VehicleComponent)
				return;
		}

		int playerID = m_VehicleComponent.GetPlayerID();
		if (playerID <= 0)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		IEntity playerEntity = playerManager.GetPlayerControlledEntity(playerID);
		if (playerEntity)
			m_VehicleOwningPlayer = SCR_EditableCharacterComponent.Cast(playerEntity.FindComponent(SCR_EditableCharacterComponent));

		string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);
		int playerCount = m_VehicleComponent.GetPlayerCountInVehicle();

		if (playerCount > 1)
			m_wNameTextWidget.SetTextFormat(m_sVehicleNameText, playerName, playerCount - 1);
		else
			m_wNameTextWidget.SetText(playerName);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		// Making changes to the Widget during OnUpdate() isn't advised by Peter Čimo.
		GetGame().GetCallqueue().CallLater(DetermineNameVisibility, 0, false);

		return false;
	}
}
