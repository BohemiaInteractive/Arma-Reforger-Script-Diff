[BaseContainerProps(visible: false)]
class SCR_AIAnimation_Gestures : SCR_AILoiterAnimation_Base
{
}

[BaseContainerProps()]
class SCR_AIAnimation_Cheer : SCR_AIAnimation_Gestures
{
	//------------------------------------------------------------------------------------------------
	void SCR_AIAnimation_Cheer()
	{
		m_sConfigPath = "{10321EEDA009B27C}Configs/AI/SCR_AIAnimation_Cheer.conf";
	}
}

[BaseContainerProps()]
class SCR_AIAnimation_Boo : SCR_AIAnimation_Gestures
{
	//------------------------------------------------------------------------------------------------
	void SCR_AIAnimation_Boo()
	{
		m_sConfigPath = "{4DF1C506A62E5183}Configs/AI/SCR_AIAnimation_Boo.conf";
	}
}

[BaseContainerProps()]
class SCR_AIAnimation_SignUp : SCR_AIAnimation_Gestures
{
	//------------------------------------------------------------------------------------------------
	void SCR_AIAnimation_SignUp()
	{
		m_sConfigPath = "{1567F1DE25A5ED33}Configs/AI/SCR_AIAnimation_SignUp.conf";
	}
}
