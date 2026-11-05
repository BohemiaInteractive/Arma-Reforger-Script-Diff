//------------------------------------------------------------------------------------------------
// all AI re-gistered animations, must specify In and Out to be possible to use in "AnimationWaypoint"
[BaseContainerProps(visible: false)]
class SCR_AIAnimation_Base
{
	ref SCR_CustomAnimDataConfig_Base m_CustomAnimData;	//container with animation properties
	protected ResourceName m_sConfigPath;
	
	//------------------------------------------------------------------------------------------------
	bool GetAnimDataFromConfig();
	bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4]);
	bool StopAnimation(IEntity pUserEntity, bool performFast);
}

//----------------------------------------------------------------------------------------------------
[BaseContainerProps(visible: false)]
class SCR_AILoiterAnimation_Base : SCR_AIAnimation_Base
{
	//------------------------------------------------------------------------------------------------
	override bool GetAnimDataFromConfig()
	{
		Resource holder = BaseContainerTools.LoadContainer(m_sConfigPath);
		if (!holder || !holder.IsValid())
			return false;
		
		m_CustomAnimData = new SCR_CustomAnimDataConfig_Base();
		BaseContainer container = holder.GetResource().ToBaseContainer();
		m_CustomAnimData = SCR_CustomAnimDataConfig_Base.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!m_CustomAnimData)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4])
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		CharacterAnimationComponent animComp = controller.GetAnimationComponent();
		if (!animComp)
			return false;
		
		if (!m_CustomAnimData && !GetAnimDataFromConfig())
			return false;
		
		SCR_CustomAnimData_Properties customAnimationData = m_CustomAnimData.m_StartAnimation;
		
		SCR_LoiterCustomAnimData startAnimationData = SCR_LoiterCustomAnimData.CreateInstanceFromParent(customAnimationData);
		PrepareStartAnimationData(startAnimationData);
		
		if (startAnimationData.m_bAllowRootMotion)
		{
			SCR_CharacterCommandHandlerComponent cmdHandler = SCR_CharacterCommandHandlerComponent.Cast(animComp.GetCommandHandler());
			if (!cmdHandler)
				return false;
			
			cmdHandler.StartCommand_Move();
		}
		
		controller.StartLoitering(null, ELoiteringType.CUSTOM, true, true, true, vAnimationTransform, true, startAnimationData);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on the owner of the character before the start animation data is sent to the other machines.
	//! \param[in] startAnimationData copy of the configured start animation data, safe to modify
	protected void PrepareStartAnimationData(notnull SCR_LoiterCustomAnimData startAnimationData);
	
	//------------------------------------------------------------------------------------------------
	override bool StopAnimation(IEntity pUserEntity, bool performFast)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		CharacterAnimationComponent animComp = controller.GetAnimationComponent();
		if (!animComp)
			return false;
		
		if (!m_CustomAnimData && !GetAnimDataFromConfig())
			return false;
		
		SCR_CustomAnimData_Properties customAnimationData;
		
		if (performFast)
			customAnimationData = m_CustomAnimData.m_HardCancelAnimation;
		else
			customAnimationData = m_CustomAnimData.m_StopSlowAnimation;
		
		if (customAnimationData.m_bAllowRootMotion)
		{
			SCR_CharacterCommandHandlerComponent cmdHandler = SCR_CharacterCommandHandlerComponent.Cast(animComp.GetCommandHandler());
			if (!cmdHandler)
				return false;
			
			cmdHandler.StartCommand_Move();
		}
		
		SCR_LoiterCustomAnimData stopAnimationData = SCR_LoiterCustomAnimData.CreateInstanceFromParent(customAnimationData);
		
		controller.StopLoitering(performFast, customAnimData: stopAnimationData);
		return true;
	}
}

//----------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AIAnimation_Sitting : SCR_AIAnimation_Base
{
	//------------------------------------------------------------------------------------------------
	override bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4])
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StartLoitering(null, ELoiteringType.SIT, false, true, true, vAnimationTransform);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool StopAnimation(IEntity pUserEntity, bool performFast)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StopLoitering(performFast);
		return true;
	}	
}

[BaseContainerProps()]
class SCR_AIAnimation_Pushups : SCR_AIAnimation_Base
{
	//------------------------------------------------------------------------------------------------
	override bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4])
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StartLoitering(null, ELoiteringType.PUSHUPS, true, true, true, vAnimationTransform);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool StopAnimation(IEntity pUserEntity, bool performFast)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StopLoitering(performFast);
		return true;
	}
};

//----------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AIAnimation_Loitering : SCR_AIAnimation_Base
{
	//------------------------------------------------------------------------------------------------
	override bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4])
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StartLoitering(null, ELoiteringType.LOITERING, true, true, true, vAnimationTransform);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool StopAnimation(IEntity pUserEntity, bool performFast)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StopLoitering(performFast);
		return true;
	}
};

//----------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AIAnimation_OfficerMission_Table : SCR_AIAnimation_Base
{
	//------------------------------------------------------------------------------------------------
	override bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4])
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		CharacterAnimationComponent animComp = controller.GetAnimationComponent();
		if (!animComp)
			return false;
		
		SCR_CharacterCommandHandlerComponent cmdHandler = SCR_CharacterCommandHandlerComponent.Cast(animComp.GetCommandHandler());
		if (!cmdHandler)
			return false;
		
		SCR_ScriptedCommandsStaticTable staticTable = cmdHandler.GetScriptedStaticTable();
		if (!staticTable)
			return false;
		
		SCR_LoiterCustomAnimData customAnimData = SCR_LoiterCustomAnimData.CreateInstance(
			staticTable.m_CustomCinematicCommand, ELoiteringType.CUSTOM,
			graphName : "{312D2589E4BF5BC8}anims/anm/NPC/workspaces/Officier_Mission01.agr",
			graphInstanceName: "{DC477052F8B60926}anims/anm/NPC/workspaces/Officier_Mission01.asi",
			controlVariableID: staticTable.m_OfficerMissionSpeech,
			controlVariableValue: 2);
		
		controller.StartLoitering(null, ELoiteringType.CUSTOM, true, true, true, vAnimationTransform, true, customAnimData);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool StopAnimation(IEntity pUserEntity, bool performFast)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StopLoitering(performFast);
		return true;
	}	
};

//----------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AIAnimation_OfficerMission_Walking : SCR_AIAnimation_Base
{
	//------------------------------------------------------------------------------------------------
	override bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4])
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		CharacterAnimationComponent animComp = controller.GetAnimationComponent();
		if (!animComp)
			return false;
		
		SCR_CharacterCommandHandlerComponent cmdHandler = SCR_CharacterCommandHandlerComponent.Cast(animComp.GetCommandHandler());
		if (!cmdHandler)
			return false;
		
		SCR_ScriptedCommandsStaticTable staticTable = cmdHandler.GetScriptedStaticTable();
		if (!staticTable)
			return false;
		
		SCR_LoiterCustomAnimData customAnimData = SCR_LoiterCustomAnimData.CreateInstance(
			staticTable.m_CustomCinematicCommand, ELoiteringType.CUSTOM,
			graphName : "{312D2589E4BF5BC8}anims/anm/NPC/workspaces/Officier_Mission01.agr",
			graphInstanceName: "{DC477052F8B60926}anims/anm/NPC/workspaces/Officier_Mission01.asi",
			controlVariableID: staticTable.m_OfficerMissionSpeech,
			controlVariableValue: 1);
		
		controller.StartLoitering(null, ELoiteringType.CUSTOM, true, true, true, vAnimationTransform, true, customAnimData);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool StopAnimation(IEntity pUserEntity, bool performFast)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StopLoitering(performFast);
		return true;
	}
};