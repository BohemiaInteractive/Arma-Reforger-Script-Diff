[BaseContainerProps()]
class SCR_ScenarioFrameworkResourceComponentActionTransfer : SCR_ScenarioFrameworkResourceComponentActionBase
{
	[Attribute(desc: "Target entity for Resource Action")];
	ref SCR_ScenarioFrameworkGet m_TargetGetter;
	
	[Attribute(defvalue: "0", desc: "Transfer Amount", params: "0 inf 0.01")]
	float m_fTransferAmount;
	
	[Attribute(defvalue: "1", desc: "Allow Partial Transfer")]
	bool m_bAllowPartialTransfer;
	
	SCR_ResourceComponent m_TargetResourceComponent;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	override void Init(SCR_ResourceComponent resourceComp, EResourceType resourceType)
	{
		IEntity entity = GetInputEntity(m_TargetGetter);
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Target Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_TargetResourceComponent = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		if (!m_TargetResourceComponent)
		{
			Print(string.Format("ScenarioFramework Action: Target Entity SCR_ResourceComponent not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		super.Init(resourceComp, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		SCR_ResourceSystemHelper.SimpleResourceTransfer(m_ResourceComponent, m_TargetResourceComponent, m_fTransferAmount, m_bAllowPartialTransfer, m_eResourceType);
	}
}