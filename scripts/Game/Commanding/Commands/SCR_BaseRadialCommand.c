[BaseContainerProps("", "Entry of one command"), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_BaseRadialCommand
{
	[Attribute("", UIWidgets.EditBox, "Unique name of the command")]
	protected string m_sCommandName;
	
	[Attribute("", UIWidgets.EditBox, "Display name of the command, in future will replace the one in commanding menu conf")]
	protected string m_sCommandDisplayName;
	
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", UIWidgets.ResourceNamePicker, "Imageset source for display icon" )]
	protected ResourceName m_sImageset;
	
	[Attribute("", UIWidgets.EditBox, "Name of the icon associated to the command, taken from appropriate imageset set in the radial menu" )]
	protected string m_sIconName;

	[Attribute(SCR_ECharacterRank.INVALID.ToString(), UIWidgets.ComboBox, desc: "Rank that is requried in order to use this command.\nINVALID == no requirement", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_eRequiredRank;

	protected LocalizedString m_sCannotPerformReason;
	protected LocalizedString m_sCannotExecuteReason;

	protected static const LocalizedString CANNOT_PERFORM_RANK_TOO_LOW 	= "#AR-Commanding_CannotPefrorm_RankTooLow";
	protected static const LocalizedString CANNOT_EXECUTE_SOLDIER_NA 	= "#AR-Commanding_CannotExecute_SoldierNotAvailable";

	//------------------------------------------------------------------------------------------------
	//! method that will be executed when the command is selected in the menu
	//! This method is right now broadcaasted to all clients, so if the command 
	//! is intended only for specific clients, it needs to be filtered inside 
	// Returns true if the command was executed succesfully, false otherwise.
	bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void VisualizeCommand(vector targetPosition);
	
	//------------------------------------------------------------------------------------------------
	void VisualizeCommandPreview(vector targetPosition);
	
	//------------------------------------------------------------------------------------------------
	//!
	string GetIconName()
	{
		return m_sIconName;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	string GetCommandDisplayName()
	{
		return m_sCommandDisplayName;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	ResourceName GetIconImageset()
	{
		return m_sImageset;
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	string GetCommandName()
	{
		return m_sCommandName;
	}
			
	//------------------------------------------------------------------------------------------------
	//!
	bool CanRoleShow()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool CanBeShownInCurrentLifeState()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool CanBeShown()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool CanShowOnMap()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns true if user (player) of the command can issue the command
	\param[in] user - the commander's chimera character
	*/	
	bool CanBePerformed(notnull SCR_ChimeraCharacter user)
	{
		if (m_eRequiredRank == SCR_ECharacterRank.INVALID)
			return true;

		if (!SCR_XPHandlerComponent.IsXpSystemEnabled())
			return true;

		SCR_ECharacterRank currentRank = SCR_CharacterRankComponent.GetCharacterRank(user);
		if (currentRank < m_eRequiredRank)
		{
			SCR_Faction userFaction = SCR_Faction.Cast(user.GetFaction());
			if (!userFaction)
				return false;

			string requiredRankName = userFaction.GetRankName(m_eRequiredRank);
			string currentRankName = userFaction.GetRankName(currentRank);
			SetCannotPerformReason(WidgetManager.Translate(CANNOT_PERFORM_RANK_TOO_LOW, requiredRankName, currentRankName));
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns true if target of the command - typically cursortarget - can execute the command
	\param[in] target entity, that should execute the command
	*/	
	bool CanBeExecuted(notnull IEntity target)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetCannotExecuteReason(LocalizedString newReason)
	{
		m_sCannotExecuteReason = newReason;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetCannotPerformReason(LocalizedString newReason)
	{
		m_sCannotPerformReason = newReason;
	}
	
	//------------------------------------------------------------------------------------------------
	LocalizedString GetCannotExecuteReason()
	{
		return m_sCannotExecuteReason;
	}
	
	//------------------------------------------------------------------------------------------------
	LocalizedString GetCannotPerformReason()
	{
		return m_sCannotPerformReason;
	}
}