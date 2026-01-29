[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_PlayerEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	private SCR_PlayersManagerEditorComponent m_PlayersManager;

	//------------------------------------------------------------------------------------------------
	void OnPlayerEnterCompartment(ChimeraCharacter playerCharacter, IEntity compartmentEntity)
	{
		Vehicle vehicle = Vehicle.Cast(compartmentEntity);
		if (!vehicle)
			return;

		SCR_EditableVehicleComponent editableVehicle = SCR_EditableVehicleComponent.Cast(vehicle.FindComponent(SCR_EditableVehicleComponent));
		if (!editableVehicle)
			return;

		SCR_EditableCharacterComponent characterComponent = SCR_EditableCharacterComponent.Cast(playerCharacter.FindComponent(SCR_EditableCharacterComponent));
		if (characterComponent)
			characterComponent.OnPlayerCharacterEnterCompartment(compartmentEntity);

		Add(editableVehicle);
		editableVehicle.UpdatePlayerCountInVehicle();
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerExitCompartment(ChimeraCharacter playerCharacter, IEntity compartmentEntity)
	{
		// During a GetOnPlayerCompartmentExit ScriptInvoker, the compartment still has the leaving character within the Compartment.
		// On the next frame, it's properly removed from the compartment.
		GetGame().GetCallqueue().CallLater(ProcessPlayerExitingCompartment, 0, false, playerCharacter, compartmentEntity);
	}

	//------------------------------------------------------------------------------------------------
	void ProcessPlayerExitingCompartment(ChimeraCharacter playerCharacter, IEntity compartmentEntity)
	{
		Vehicle vehicle = Vehicle.Cast(compartmentEntity);
		if (!vehicle)
			return;

		SCR_EditableVehicleComponent editableVehicle = SCR_EditableVehicleComponent.Cast(vehicle.FindComponent(SCR_EditableVehicleComponent));
		if (!editableVehicle)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerCharacterId = playerManager.GetPlayerIdFromControlledEntity(playerCharacter);

		editableVehicle.UpdatePlayerCountInVehicle();
		if (editableVehicle.GetPlayerCountInVehicle() <= 0)
			Remove(editableVehicle);

		if (!playerCharacter)
			return;

		SCR_EditableCharacterComponent characterComponent = SCR_EditableCharacterComponent.Cast(playerCharacter.FindComponent(SCR_EditableCharacterComponent));
		if (characterComponent)
			characterComponent.OnPlayerCharacterExitCompartment(compartmentEntity);
	}

	protected void OnSpawn(int playerID, SCR_EditableEntityComponent entity, SCR_EditableEntityComponent entityPrev)
	{
		Set(entity, entityPrev);

		SCR_EditableCharacterComponent character = SCR_EditableCharacterComponent.Cast(entity);
		if (!character)
			return;

		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(character.GetOwner().FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return;

		compartmentAccess.GetOnPlayerCompartmentEnter().Insert(OnPlayerEnterCompartment);
		compartmentAccess.GetOnPlayerCompartmentExit().Insert(OnPlayerExitCompartment);
	}

	protected void OnDeath(int playerID, SCR_EditableEntityComponent entity, SCR_EditableEntityComponent killerEntity)
	{
		Remove(entity);

		SCR_EditableCharacterComponent character = SCR_EditableCharacterComponent.Cast(entity);
		if (!character)
			return;

		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(character.GetOwner().FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return;

		compartmentAccess.GetOnPlayerCompartmentEnter().Remove(OnPlayerEnterCompartment);
		compartmentAccess.GetOnPlayerCompartmentExit().Remove(OnPlayerExitCompartment);
	}

	protected void OnPossessed(int playerID, SCR_EditableEntityComponent entity, bool isPossessing)
	{
		if (isPossessing)
			Add(entity);
		else
			Remove(entity);
	}

	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		if (!entity || entity.IsDestroyed())
			return false;

		return m_PlayersManager.GetPlayerID(entity) != 0 || entity.GetPlayerID() != 0;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EditableCharacterComponent GetVehiclePlayer(notnull SCR_EditableEntityComponent entity)
	{
		SCR_EditableVehicleComponent vehicle = SCR_EditableVehicleComponent.Cast(entity);
		if (!vehicle)
			return null;

		int playerId = vehicle.GetPlayerID();
		if (playerId == 0)
			return null;

		IEntity characterEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!characterEntity)
			return null;

		return SCR_EditableCharacterComponent.Cast(characterEntity.FindComponent(SCR_EditableCharacterComponent));
	}

	override void EOnEditorActivate()
	{
		m_PlayersManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent, true));
		if (!m_PlayersManager)
			return;

		m_PlayersManager.GetOnSpawn().Insert(OnSpawn);
		m_PlayersManager.GetOnDeath().Insert(OnDeath);
		m_PlayersManager.GetOnPossessed().Insert(OnPossessed);
	}

	override void EOnEditorDeactivate()
	{
		if (!m_PlayersManager)
			return;

		m_PlayersManager.GetOnSpawn().Remove(OnSpawn);
		m_PlayersManager.GetOnDeath().Remove(OnDeath);
		m_PlayersManager.GetOnPossessed().Remove(OnPossessed);
	}
}
