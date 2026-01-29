[BaseContainerProps(configRoot: true)]
class SCR_GroupIdentityCore: SCR_GameCoreBase
{
	[Attribute()]
	protected ref SCR_GroupNameConfig m_Names;
	
	[Attribute()]
	protected ref SCR_MilitarySymbolConfig m_Symbols;
	
	[Attribute()]
	protected ref SCR_MilitarySymbolRuleSet m_SymbolRuleSet;
	
	SCR_GroupNameConfig GetNames()
	{
		return m_Names;
	}
	SCR_MilitarySymbolConfig GetSymbols()
	{
		return m_Symbols;
	}
	SCR_MilitarySymbolRuleSet GetSymbolRuleSet()
	{
		return m_SymbolRuleSet;
	}
};