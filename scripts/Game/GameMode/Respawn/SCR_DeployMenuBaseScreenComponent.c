[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for the deploy menu screen components")]
class SCR_DeployMenuBaseScreenComponentClass : SCR_BaseGameModeComponentClass
{
}

//! Base class for Deploy menu components.
class SCR_DeployMenuBaseScreenComponent : SCR_BaseGameModeComponent
{
	[Attribute()]
	protected string m_sHeaderTitle;

	[Attribute()]
	protected string m_sHeaderSubtitle;

	[Attribute()]
	protected ref SCR_DeployMenuBaseScreenLayout m_BaseLayout;

	//------------------------------------------------------------------------------------------------
	//! \return header title
	string GetHeaderTitle()
	{
		return m_sHeaderTitle;
	}

	//------------------------------------------------------------------------------------------------
	//! \return header subtitle
	string GetHeaderSubtitle()
	{
		return m_sHeaderSubtitle;
	}

	//------------------------------------------------------------------------------------------------
	//! Get base layout of this component
	//! \return base layout
	SCR_DeployMenuBaseScreenLayout GetBaseLayout()
	{
		return m_BaseLayout;
	}
}

//! Base class for Deploy menu layouts.
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_DeployMenuBaseScreenLayout : ScriptAndConfig
{
	protected ref array<ref SCR_WelcomeScreenBaseContent> m_aScreenBaseContents = {};
	
	//------------------------------------------------------------------------------------------------
	//! Initialises content for given menu.
	//! \param[in] menu
	void InitContent(SCR_WelcomeScreenMenu menu);
	
	//------------------------------------------------------------------------------------------------
	//! Get base content of this layout
	//! \param[out] screenBaseContents array of base contents
	//! \return number of base contents
	int GetScreenBaseContents(out array<ref SCR_WelcomeScreenBaseContent> screenBaseContents)
	{
		screenBaseContents = m_aScreenBaseContents;

		return m_aScreenBaseContents.Count();
	}
}
