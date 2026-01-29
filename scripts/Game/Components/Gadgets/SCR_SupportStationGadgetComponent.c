[ComponentEditorProps(category: "GameScripted/Gadgets", description: "")]
class SCR_SupportStationGadgetComponentClass : SCR_GadgetComponentClass
{
}

class SCR_SupportStationGadgetComponent : SCR_GadgetComponent
{	
	[Attribute(desc: "Which support station this gadget belongs to", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ESupportStationType), category: "Support Gadget")]
	protected ref array<ESupportStationType> m_aSupportStationTypes;
	
	[Attribute("1", desc: "If true then the animation target will be the gadget Offset in world position", category: "Support Gadget")]
	protected bool m_bUseActionAsAnimationTarget;
	
	protected ref map<ESupportStationType, SCR_BaseSupportStationComponent> m_mSupportStations = new map<ESupportStationType, SCR_BaseSupportStationComponent>();
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] Get List of all support station types this gadget has
	//! \return Count of support station type array
	int GetSupportStationTypes(out notnull array<ESupportStationType> types)
	{
		types.Copy(m_aSupportStationTypes);
		
		return types.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] Type to check
	//! \return True if gadget is of the given type
	bool IsGadgetOfSupportStationType(ESupportStationType type)
	{
		return m_aSupportStationTypes.Contains(type);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] character Character that animates
	//! \return True if character is currently in an animation loop
	bool InUseAnimationLoop(IEntity character)
	{
		ChimeraCharacter chimeraCharacter = ChimeraCharacter.Cast(character);
		if (!chimeraCharacter)
			return false;
		
		CharacterControllerComponent charController = chimeraCharacter.GetCharacterController();
		if (!charController)
			return false;
		
		CharacterAnimationComponent animationComponent = charController.GetAnimationComponent();
		if (!animationComponent)
			return false;
		
		CharacterCommandHandlerComponent commandHandlerComponent = CharacterCommandHandlerComponent.Cast(animationComponent.FindComponent(CharacterCommandHandlerComponent));
		return commandHandlerComponent && commandHandlerComponent.IsItemActionLoopTag();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Start the animation if animation 'in' and looping than it will go to the loop animation and StopGadgetAnimation() needs to be called
	//! \param[in] character Character to animate
	//! \param[in] animationCommand
	//! \param[in] action Action that is used for the animation
	void StartGadgetAnimation(notnull IEntity character, int animationCommand, notnull SCR_ScriptedUserAction action)
	{
		if (animationCommand < 0)
			return;
		
		if (m_bUseActionAsAnimationTarget)
			AnimateCharacterWithGadget(character, animationCommand, action.GetWorldPositionAction());
		else 
			AnimateCharacterWithGadget(character, animationCommand);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Start the animation if animation 'out'. Used when animation is looping
	//! \param[in] character Character to animate
	//! \param[in] animationCommand
	void StopGadgetAnimation(notnull IEntity character, int animationCommand)
	{
		if (animationCommand < 0)
			return;
		
		ChimeraCharacter chimeraCharacter = ChimeraCharacter.Cast(character);
		if (!chimeraCharacter)
			return;
		
		CharacterControllerComponent charController = chimeraCharacter.GetCharacterController();
		if (!charController)
			return;
		
		CharacterAnimationComponent animationComponent = charController.GetAnimationComponent();
		if (!animationComponent)
			return;
		
		CharacterCommandHandlerComponent commandHandlerComponent = CharacterCommandHandlerComponent.Cast(animationComponent.FindComponent(CharacterCommandHandlerComponent));
		if (commandHandlerComponent)
			commandHandlerComponent.FinishItemUse(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AnimateCharacterWithGadget(notnull IEntity character, int animationCMD, vector animationTarget = vector.Zero)
	{
		ChimeraCharacter chimeraCharacter = ChimeraCharacter.Cast(character);
		if (!chimeraCharacter)
			return;
		
		CharacterControllerComponent charController = chimeraCharacter.GetCharacterController();
		if (!charController)
			return;
		
		CharacterAnimationComponent animationComponent = charController.GetAnimationComponent();
		if (!animationComponent)
			return;
		
		PointInfo ptWS = null;
		
		//~ Set target look position
		if (animationTarget != vector.Zero)
		{
			vector charWorldMat[4];
			character.GetWorldTransform(charWorldMat);
			charWorldMat[3] = animationTarget;
			ptWS = new PointInfo();
			ptWS.Set(null, "", charWorldMat);
		}
		
		int itemActionId = animationComponent.BindCommand("CMD_Item_Action");

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(GetOwner());
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(true);
		params.SetCommandID(itemActionId);
		params.SetCommandIntArg(animationCMD);
		params.SetCommandFloatArg(2.0);
		params.SetAlignmentPoint(ptWS);

		charController.TryUseItemOverrideParams(params);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get action support station of given type
	//! \param[in] supportStationType Type of gadget to find
	//! \return Support Station on gadget (if any)
	SCR_BaseSupportStationComponent GetSupportStation(ESupportStationType supportStationType)
	{
		SCR_BaseSupportStationComponent supportStation;
		m_mSupportStations.Find(supportStationType, supportStation);
		
		return supportStation;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get held gadget of given type from character
	//! \param[in] supportStationType Type of gadget to find
	//! \param[in] character Character to find gadget on
	//! \return Support Station gadget component (if any)
	static SCR_BaseSupportStationComponent GetHeldSupportStation(ESupportStationType supportStationType, notnull IEntity character)
	{
		SCR_SupportStationGadgetComponent gadgetComponent = GetHeldSupportStationGadget(supportStationType, character);
		if (!gadgetComponent)
			return null;
		
		return gadgetComponent.GetSupportStation(supportStationType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get support station of the given type from held gadget of the character
	//! \param[in] supportStationType Type of support station to find
	//! \param[in] character Character to get support station gadget off
	//! \return Support Station component (if any)
	static SCR_SupportStationGadgetComponent GetHeldSupportStationGadget(ESupportStationType supportStationType, notnull IEntity character)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(character);
		if (!gadgetManager)
			return null;

		SCR_SupportStationGadgetComponent gadgetComponent = SCR_SupportStationGadgetComponent.Cast(gadgetManager.GetHeldGadgetComponent());
		if (!gadgetComponent)
			return null;
		
		if (!gadgetComponent.IsGadgetOfSupportStationType(supportStationType))
			return null;
		
		return gadgetComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get support station of the given type from held gadget as well as Support statioon gadget
	//! \param[in] supportStationType Type of support station and gadget to find
	//! \param[in] character Character to get support station gadget off
	//! \param[out] supportStation Found support station
	//! \param[out] supportStationGadget Found gadget
	//! \return True if both Support station and gadget were found of given type
	static bool GetHeldSupportStationAndGadget(ESupportStationType supportStationType, notnull IEntity character, out SCR_BaseSupportStationComponent supportStation, out SCR_SupportStationGadgetComponent supportStationGadget)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(character);
		if (!gadgetManager)
			return false;

		supportStationGadget = SCR_SupportStationGadgetComponent.Cast(gadgetManager.GetHeldGadgetComponent());
		if (!supportStationGadget)
			return false;
		
		if (!supportStationGadget.IsGadgetOfSupportStationType(supportStationType))
			return false;
		
		supportStation = supportStationGadget.GetSupportStation(supportStationType);
		return supportStation;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		array<Managed> supportStations = {};
		GetOwner().FindComponents(SCR_BaseSupportStationComponent, supportStations);
		
		//~ Cache support stations for optimization
		SCR_BaseSupportStationComponent supportStation;
		foreach (Managed supportStationManaged : supportStations)
		{
			supportStation = SCR_BaseSupportStationComponent.Cast(supportStationManaged);
			if (!supportStation)
				continue;
			
			if (m_mSupportStations.Contains(supportStation.GetSupportStationType()))
			{
				Print("Support Station Gadget has two or more support stations of type: '" + typename.EnumToString(ESupportStationType, supportStation.GetSupportStationType()) + "' it can only have one of the same type!", LogLevel.WARNING);
				continue;
			}
			
			m_mSupportStations.Insert(supportStation.GetSupportStationType(), supportStation);
		}
	}

	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.SPECIALIST_ITEM;
	}
}
