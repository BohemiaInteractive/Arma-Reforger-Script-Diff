[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_EffectsModuleAreaMeshComponentClass: SCR_BaseAreaMeshComponentClass
{
};
class SCR_EffectsModuleAreaMeshComponent: SCR_BaseAreaMeshComponent
{
	[Attribute("10")]
	protected float m_fWidth;
	
	[Attribute("10")]
	protected float m_fLenght;
	
	//------------------------------------------------------------------------------------------------
	override void GetDimensions2D(out float width, out float length)
	{
		width = m_fWidth;
		length = m_fLenght;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_EffectsModuleComponent effectModule = SCR_EffectsModuleComponent.Cast(owner.FindComponent(SCR_EffectsModuleComponent));
		if (!effectModule && owner.GetParent())
			effectModule = SCR_EffectsModuleComponent.Cast(owner.GetParent().FindComponent(SCR_EffectsModuleComponent));
		
		if (!effectModule)
		{
			Print("'SCR_EffectsModuleAreaMeshComponent' Component must be attached to entity with SCR_EffectsModuleComponent!", LogLevel.ERROR);
			return;
		}
		
		SCR_EffectsModule effectModuleData = effectModule.GetEffectsModuleConfig();
		if (!effectModuleData)
		{
			Print("'SCR_EffectsModuleAreaMeshComponent' SCR_EffectsModuleComponent has no SCR_EffectsModule assinged!", LogLevel.ERROR);
			return;
		}
		
		SCR_EffectsModulePositionData_Radius radiusData = SCR_EffectsModulePositionData_Radius.Cast(effectModuleData.GetEffectsModuleZoneData());
		if (radiusData)
		{
			if (m_fWidth != radiusData.GetRadius() || m_fLenght != radiusData.GetRadius())
			{
				Print("'SCR_EffectsModuleAreaMeshComponent' has width and/or lenght that is not set equal to the radius of the effect module!", LogLevel.ERROR);
				m_fWidth = radiusData.GetRadius();
				m_fLenght = m_fWidth;
			}
				
			GenerateAreaMesh();
			return;
		}
		
		SCR_EffectsModulePositionData_Rectangle rectangleData = SCR_EffectsModulePositionData_Rectangle.Cast(effectModuleData.GetEffectsModuleZoneData());
		if (rectangleData)
		{	
			float width, lenght;
			rectangleData.GetDimensions2D(width, lenght);
			
			if (m_fWidth != width || m_fLenght != lenght)
			{
				Print("'SCR_EffectsModuleAreaMeshComponent' (Square) has width and/or lenght that is not set equal to the width and/or lenght of the effect module!", LogLevel.ERROR);
				m_fWidth = width;
				m_fLenght = lenght;
			}
				

			GenerateAreaMesh();
			return;
		}
		
		Print("'SCR_EffectsModuleAreaMeshComponent' SCR_EffectsModuleComponent has invalid 'SCR_BaseEffectsModulePositionData' which is not supported for the AreaMesh Generation!", LogLevel.ERROR);
	}
};