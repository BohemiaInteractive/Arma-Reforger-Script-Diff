class SCR_MortarShootAction : SCR_ScriptedUserAction
{
	[Attribute("0", desc: "If action should be performed from the left side of the mortar")]
	protected bool m_bPerformFromTheLeftSide;
	
	[Attribute("0", desc: "Check if turret is empty to ensure that loader will not clip into the gunner of the mortar")]
	protected bool m_bCheckIfTurretIsEmpty;

	protected BaseMagazineWell m_MortarMagWell;
	protected SCR_MortarMuzzleComponent m_MortarMuzzleComponent;
	protected SCR_MortarShellGadgetComponent m_ShellComp;
	protected ChimeraCharacter m_Loader;
	protected float m_fFuzeTime = -1;

	protected const LocalizedString WRONG_SHELL_TYPE = "#AR-UserAction_WrongShellType";
	protected const LocalizedString ALREADY_LOADED = "#AR-UserAction_AlreadyLoaded";
	protected const LocalizedString FIRE_WITH_MANUAL_FUZE = "#AR-UserAction_MortarFire_ManualFuze";
	protected const LocalizedString FIRE_WITH_AUTOMATIC_FUZE = "#AR-UserAction_MortarFire_AutomaticFuze";
	protected const LocalizedString AREA_OBSTRUCTED = "#AR-UserAction_SeatObstructed";

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_MortarMuzzleComponent = SCR_MortarMuzzleComponent.Cast(pOwnerEntity.FindComponent(SCR_MortarMuzzleComponent));
		if (m_MortarMuzzleComponent)
			m_MortarMagWell = m_MortarMuzzleComponent.GetMagazineWell();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_MortarMuzzleComponent && !m_MortarMagWell)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterControllerComponent controllerComponent = character.GetCharacterController();
		if (!controllerComponent || !controllerComponent.CanPlayItemGesture())
			return false;

		m_ShellComp = GetHeldShellGadgetComp(character);
		if (!m_ShellComp || m_ShellComp.IsLoaded())
			return false;

		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterControllerComponent controllerComponent = character.GetCharacterController();
		if (!controllerComponent || !controllerComponent.CanPlayItemGesture())
			return false;

		IEntity shell = controllerComponent.GetAttachedGadgetAtLeftHandSlot();
		if (!shell)
			return false;

		MagazineComponent shellMagazine = MagazineComponent.Cast(shell.FindComponent(MagazineComponent));
		if (!shellMagazine || !shellMagazine.GetMagazineWell() || shellMagazine.GetMagazineWell().Type() != m_MortarMagWell.Type())
		{
			SetCannotPerformReason(WRONG_SHELL_TYPE);
			return false;
		}

		if (m_MortarMuzzleComponent.GetAmmoCount() >= 1 || m_MortarMuzzleComponent.IsBeingLoaded())
		{
			SetCannotPerformReason(ALREADY_LOADED);
			return false;
		}

		if (m_bCheckIfTurretIsEmpty)
		{
			SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
			if (compartmentManager && compartmentManager.AnyCompartmentsOccupiedOrLocked())
			{
				SetCannotPerformReason(AREA_OBSTRUCTED);
				return false;
			}
		}

		SetCannotPerformReason(string.Empty);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (m_MortarMuzzleComponent.GetAmmoCount() >= m_MortarMuzzleComponent.GetMaxAmmoCount())
			return;

		if (SCR_PlayerController.GetLocalControlledEntity() == pUserEntity || SCR_CharacterHelper.IsAPlayer(pUserEntity))
			return;//filter out all players as this method is intended only for AI

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		if (!m_ShellComp)
		{
			CharacterControllerComponent controllerComponent = character.GetCharacterController();
			if (!controllerComponent || !controllerComponent.IsGadgetInHands())
				return;
	
			IEntity shell = controllerComponent.GetAttachedGadgetAtLeftHandSlot();
			m_ShellComp = SCR_MortarShellGadgetComponent.Cast(shell.FindComponent(SCR_MortarShellGadgetComponent));
			if (!m_ShellComp)
				return;
		}

		m_MortarMuzzleComponent.LoadShell(m_ShellComp, character, fromLeftSide: m_bPerformFromTheLeftSide);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_MortarShellGadgetComponent GetHeldShellGadgetComp(notnull ChimeraCharacter character)
	{
		CharacterControllerComponent controllerComponent = character.GetCharacterController();
		if (!controllerComponent || !controllerComponent.IsGadgetInHands())
			return null;

		IEntity shell = controllerComponent.GetAttachedGadgetAtLeftHandSlot();
		if (!shell)
			return null;

		return SCR_MortarShellGadgetComponent.Cast(shell.FindComponent(SCR_MortarShellGadgetComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_ShellComp || !m_ShellComp.IsUsingTimeFuze() || !GetCannotPerformReason().IsEmpty())
			return false;

		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return false;

		SCR_ShellConfig timeFuzeConfig = m_ShellComp.GetSavedConfig(character);
		if (!timeFuzeConfig || !timeFuzeConfig.IsUsingManualTime())
		{
			float elevation = m_MortarMuzzleComponent.GetMuzzleElevation();
			float timeToDetonation = m_ShellComp.GetTimeToDetonation(elevation, false);
			string fuzeTime = WidgetManager.Translate(UIConstants.VALUE_UNIT_SECONDS, timeToDetonation.ToString(-1, 1));
			outName = WidgetManager.Translate(FIRE_WITH_AUTOMATIC_FUZE, fuzeTime);
		}
		else
		{
			string fuzeTime = WidgetManager.Translate(UIConstants.VALUE_UNIT_SECONDS, timeFuzeConfig.GetSavedTime().ToString(-1, 1));
			outName = WidgetManager.Translate(FIRE_WITH_MANUAL_FUZE, fuzeTime);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		m_Loader = ChimeraCharacter.Cast(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		if (!m_ShellComp)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return false;

		float fuzeTime = -1;
		SCR_ShellConfig timeFuzeConfig = m_ShellComp.GetSavedConfig(character);
		if (timeFuzeConfig && timeFuzeConfig.IsUsingManualTime())
			fuzeTime = timeFuzeConfig.GetSavedTime();

		writer.WriteFloat(fuzeTime);

		return super.OnSaveActionData(writer);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		float fuzeTime;
		reader.ReadFloat(fuzeTime);

		if (!m_Loader)
			return false;

		if (m_MortarMuzzleComponent.GetAmmoCount() >= m_MortarMuzzleComponent.GetMaxAmmoCount())
			return false;

		m_ShellComp = GetHeldShellGadgetComp(m_Loader);
		if (!m_ShellComp)
			return false;

		m_MortarMuzzleComponent.LoadShell(m_ShellComp, m_Loader, fuzeTime, m_bPerformFromTheLeftSide);
		return super.OnLoadActionData(reader);
	}
}
