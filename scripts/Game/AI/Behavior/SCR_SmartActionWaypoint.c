class SCR_SmartActionWaypointClass: SCR_AIWaypointClass
{
};
class SCR_SmartActionWaypoint : SCR_AIWaypoint
{
    [Attribute("", UIWidgets.EditBox, "Static reference to entity with smart action")]
    string m_sStaticEntityName;
	
    [Attribute("", UIWidgets.EditBox, "Tag of the smart action")]
    string m_sSmartActionTag;
    
    IEntity m_SmartActionEntity;  
    
    void GetSmartActionEntity(out IEntity smartActionEntity, out string smartActionTag)
    {
    	if (m_SmartActionEntity)
		{
			smartActionEntity = m_SmartActionEntity;		
		}
		else
		{
			IEntity ent = GetGame().GetWorld().FindEntityByName(m_sStaticEntityName);
			if (ent)
				smartActionEntity = ent;
		}		
		if (m_sSmartActionTag == string.Empty)
			Debug.Error("Unspecified smart action tag");
		smartActionTag = m_sSmartActionTag;	
    }
    
    void SetSmartActionEntity(IEntity entityWithSmartAction, string smartActionTag)
    {
        m_SmartActionEntity = entityWithSmartAction;
		m_sSmartActionTag = smartActionTag;
    }    	
};