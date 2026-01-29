[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_DisassembleBunkerClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_DisassembleBunker : SCR_BaseTutorialStage
{
	protected IEntity m_Entity;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_Entity = GetGame().GetWorld().FindEntityByName("SEIZING_Bunker");
		if (m_Entity)
			RegisterWaypoint(m_Entity, "", "DISASSEMBLY");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_Entity;
	}
};