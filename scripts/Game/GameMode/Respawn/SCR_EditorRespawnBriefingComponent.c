[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "")]
class SCR_EditorRespawnBriefingComponentClass : SCR_RespawnBriefingComponentClass
{
}

class SCR_EditorRespawnBriefingComponent : SCR_RespawnBriefingComponent
{
	[Attribute()]
	protected ref SCR_UIInfo m_InfoGameMaster;
	
	[Attribute()]
	protected ref SCR_UIInfo m_InfoHost;
	
	//------------------------------------------------------------------------------------------------
	override SCR_UIInfo GetInfo()
	{
		//~ Check if player is Server
		if (Replication.IsServer())
		{
			return m_InfoHost;
		}
		else
		{
			//~ Check if player is Game Master
			SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
			if (!editorManager || editorManager.IsLimited())
				return m_Info;
			else 
				return m_InfoGameMaster;
		}	
	}
}
