[ComponentEditorProps(category: "GameScripted/Editor", description: "Camera for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingCameraEditorComponentClass: SCR_CameraEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
class SCR_CampaignBuildingCameraEditorComponent : SCR_CameraEditorComponent
{
	override protected void CreateCamera()
	{
		super.CreateCamera();	
		
		SCR_AttachManualCameraComponent attachComponent = SCR_AttachManualCameraComponent.Cast(m_Camera.FindCameraComponent(SCR_AttachManualCameraComponent));
		if (!attachComponent)
			return; 
		
		SCR_CampaignBuildingEditorComponent buildingComp = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
		if (buildingComp)
			attachComponent.AttachTo(buildingComp.GetTrigger());
	}
};