// Spawns Track Decal

class TrackDecalInfo
{
	TrackDecal m_Decal;
	vector m_vLastAxlePos;
	vector m_vLastTracePos;
	vector m_vLastTraceNormal;
	float m_fLength;
	bool m_bConnectToPrevious;
	
	void TrackDecalInfo()
	{
		m_Decal = null;
		m_vLastAxlePos = vector.Zero;
		m_vLastTracePos = vector.Zero;
		m_vLastTraceNormal = vector.Zero;
		m_fLength = 0;
		m_bConnectToPrevious = false;
	}
	
	void Finalize(float dist)
	{
		if(!m_Decal) return;
		
		if(dist < 0.01)
		{
			m_Decal.FinalizeTrackDecal(false, 0);
			m_bConnectToPrevious = true;
		}
		else
		{
			m_Decal.FinalizeTrackDecal(true, dist);
			m_bConnectToPrevious = false;
		}
		
		m_Decal = null;
		m_fLength = 0;
	}
};

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

	ref array<ref TrackDecalInfo> m_TrackDecalsInfo;
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
		
		m_TrackDecalsInfo = new array<ref TrackDecalInfo>();
		m_TrackDecalsInfo.Resize(m_iWheelCount);
		
		for(int i = 0; i < m_iWheelCount; i++)
		{
			m_TrackDecalsInfo[i] = new TrackDecalInfo();
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
		TrackDecalInfo trackInfo = m_TrackDecalsInfo[wheelIdx];
		
		float lateralSlip = m_VehicleWheeledSimulation.WheelGetLateralSlip(wheelIdx);
		float latitudeSlip = m_VehicleWheeledSimulation.WheelGetLongitudinalSlip(wheelIdx);

		bool shouldAddTrackDecal = m_VehicleWheeledSimulation.WheelHasContact(wheelIdx) && (lateralSlip >= m_fMinimalLateralSlip || latitudeSlip >= m_fMinimalLongitudinalSlip);

		if(!shouldAddTrackDecal)
		{
			if(trackInfo.m_Decal)
			{
				Print("No contact");
				trackInfo.Finalize(0.25);
			}
			return;
		}
		
		vector position = m_VehicleWheeledSimulation.WheelGetContactPosition(wheelIdx);
		vector normal = m_VehicleWheeledSimulation.WheelGetContactNormal(wheelIdx);
		IEntity contactEntity = m_VehicleWheeledSimulation.WheelGetContactEntity(wheelIdx);
		
		if(!trackInfo.m_Decal)
		{
			if(trackInfo.m_bConnectToPrevious)
			{
				trackInfo.m_Decal = GetOwner().GetWorld().CreateTrackDecal(contactEntity, trackInfo.m_vLastTracePos, trackInfo.m_vLastTraceNormal, 0.25, 120.0, m_TrackMaterial, null, 1.0);
				trackInfo.m_bConnectToPrevious = false;
				Print("Connected");
			}
			else
			{
				trackInfo.m_Decal = GetOwner().GetWorld().CreateTrackDecal(contactEntity, position, normal, 0.25, 120.0, m_TrackMaterial, null, 0.0);
				Print("New");
			}
		}
		else if(vector.DistanceSq(trackInfo.m_vLastAxlePos, position) > 0.01)
		{
			trackInfo.m_vLastAxlePos = position;
			
			int validationEnum = trackInfo.m_Decal.CanAddToTrackDecal(contactEntity, m_TrackMaterial, position);
			
			switch(validationEnum)
			{
				case -1:
				Print("Track error");
				break;
				case 0: //Valid
				{
					trackInfo.m_fLength += vector.Distance(position, trackInfo.m_vLastTracePos);
					
					if(!trackInfo.m_Decal.AddPointToTrackDecal(position, normal, 1.0))
					{
						trackInfo.Finalize(0.0);
						Print("Finalized point");
					}
				}
				break;
				case 1: // Different entity
				{
					TrackDecal oldDecal = trackInfo.m_Decal;
					oldDecal.FinalizeTrackDecal(false, 0);
					
					trackInfo.m_Decal = GetOwner().GetWorld().CreateTrackDecal(contactEntity, position, normal, 0.25, 120.0, m_TrackMaterial, oldDecal, 1.0);
					trackInfo.m_bConnectToPrevious = false;
					Print("Diff ent");
				}
				break;
				case 2: // Too far from last point
				{
					trackInfo.Finalize(0.1);
					trackInfo.m_fLength = 0.0;
					trackInfo.m_Decal = GetOwner().GetWorld().CreateTrackDecal(contactEntity, position, normal, 0.25, 120.0, m_TrackMaterial, null, 0.0);
					trackInfo.m_bConnectToPrevious = false;
					Print("Too far");
				}
				break;
			}
		}
		
		trackInfo.m_vLastTracePos = position;
		trackInfo.m_vLastTraceNormal = normal;
	}
};