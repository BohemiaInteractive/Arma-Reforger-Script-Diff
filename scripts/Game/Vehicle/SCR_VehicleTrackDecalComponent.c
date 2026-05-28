// Spawns Track Decal

[ComponentEditorProps(category: "GameScripted/Test", description:"SCR_VehicleTrackDecal")]
class SCR_VehicleTrackDecalClass : ScriptComponentClass
{
};

class SCR_VehicleTrackDecal : ScriptComponent
{	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Material for track decal", params: "emat")]
	private ResourceName m_TrackMaterial;
	
	[Attribute( "0.5", UIWidgets.EditBox, "Minimal lateral slip of wheel" )]
	float	m_fMinimalLateralSlip;
	
	[Attribute( "0.5", UIWidgets.EditBox, "Minimal longitudinal slip of wheel" )]
	float	m_fMinimalLongitudinalSlip;
	
	VehicleWheeledSimulation m_VehicleWheeledSimulation;

	ref array<ref TrackDecalFactory> m_TrackDecals;
	int m_iWheelCount = 0;
	
	//------------------------------------------------------------------------------------------------  
	override void OnPostInit(IEntity owner)
	{
		// Don't run this component on console app
		if (System.IsConsoleApp())
		{
			Deactivate(owner);
			return;
		}
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------  
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		GenericEntity generic_entity = GenericEntity.Cast(owner);
				
		if(generic_entity)
		{
			m_VehicleWheeledSimulation = VehicleWheeledSimulation.Cast(generic_entity.FindComponent(VehicleWheeledSimulation));
			m_iWheelCount = m_VehicleWheeledSimulation.WheelCount();
		}
		
		m_TrackDecals = new array<ref TrackDecalFactory>();
		m_TrackDecals.Resize(m_iWheelCount);
		
		for(int i = 0; i < m_iWheelCount; i++)
		{
			m_TrackDecals[i] = new TrackDecalFactory(m_TrackMaterial, 0.25, 10);
		}
		
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------  
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		for(int i = 0; i < m_iWheelCount; i++)
		{
			UpdateTrack(owner, i);
		}
	}
	
	void UpdateTrack(IEntity owner, int wheelIdx)
	{		
		float lateralSlip = m_VehicleWheeledSimulation.WheelGetLateralSlip(wheelIdx);
		float latitudeSlip = m_VehicleWheeledSimulation.WheelGetLongitudinalSlip(wheelIdx);

		bool shouldAddTrackDecal = m_VehicleWheeledSimulation.WheelHasContact(wheelIdx) && (lateralSlip >= m_fMinimalLateralSlip || latitudeSlip >= m_fMinimalLongitudinalSlip);

		if(!shouldAddTrackDecal)
		{
			m_TrackDecals[wheelIdx].FinalizeDecal(0.25);
			return;
		}
		
		vector position = m_VehicleWheeledSimulation.WheelGetContactPosition(wheelIdx);
		vector normal = m_VehicleWheeledSimulation.WheelGetContactNormal(wheelIdx);
		IEntity contactEntity = m_VehicleWheeledSimulation.WheelGetContactEntity(wheelIdx);
		
		m_TrackDecals[wheelIdx].AddPoint(contactEntity, position, normal, 1.0);
	}
};