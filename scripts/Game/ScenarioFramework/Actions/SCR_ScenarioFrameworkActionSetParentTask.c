[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetParentTask : SCR_ScenarioFrameworkActionBase
{

    [Attribute(desc: "Task Getter")]
    ref SCR_ScenarioFrameworkGet m_Getter;

    [Attribute(defvalue: "", desc: "Parent Task Layer Name")]
    string m_sParentTaskLayerName;
	
	[Attribute(defvalue: "0", desc: "Wether or not the subtask is optional or not.", category: "Subtask")]
	bool m_bIsOptional;

    //------------------------------------------------------------------------------------------------
    override void OnActivate(IEntity object)
    {
        if (!CanActivate())
            return;

        IEntity entity;
        if (!ValidateInputEntity(object, m_Getter, entity))
            return;

        SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
        if (!layerTask)
            return;

        layerTask.m_bIsSubtask = true;
        layerTask.m_bIsOptional = m_bIsOptional;
        layerTask.m_sParentLayerTask = m_sParentTaskLayerName;
		
		if (!layerTask.GetIsInitiated())
			return;
		
		layerTask.SetupSubtask();
    }
}