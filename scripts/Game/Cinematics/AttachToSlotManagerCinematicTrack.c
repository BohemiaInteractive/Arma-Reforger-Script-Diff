[CinematicTrackAttribute(name:"AttachToSlot Track", description:"Track used for attaching entities to Slot Manager")]
class AttachToSlotManagerCinematicTrack : CinematicTrackBase
{
	
	[Attribute("")]
	string m_sEntityToAttach;
	
	[Attribute("")]
	string m_sSlotName;
	
	[Attribute("0 0 0")]
	vector m_Position;
	
	[Attribute("0 0 0")]
	vector m_Rotation;
	
	[Attribute("1.0", params:"0.01 100.0")]
	float m_fScale;
	
	private GenericEntity m_GeneralEntity;
	private GenericEntity m_EntityToAttach;
	private World globalWorld;
	
	override void OnInit(World world)
	{
		// Find particle entity by using name of track
		findEntity(world);
		m_EntityToAttach = GenericEntity.Cast(world.FindEntityByName(m_sEntityToAttach));		
		globalWorld = world;
	}
	
	void findEntity(World world)
	{
		// Find particle entity by using name of track
		TStringArray strs = new TStringArray;
		GetTrackName().Split("_", strs, true);
		
		m_GeneralEntity = GenericEntity.Cast(world.FindEntityByName(strs.Get(0)));
	}
	
	override void OnApply(float time)
	{
		
		if (globalWorld)
		{
			//Finding effect entity each frame for keep working in edit time
			findEntity(globalWorld);
			
			if (m_sEntityToAttach == "player" && GetGame().GetPlayerController())
				m_EntityToAttach = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
			else
				m_EntityToAttach = GenericEntity.Cast(globalWorld.FindEntityByName(m_sEntityToAttach));	
		}
		
		//Attaching entity to slot
		if (m_GeneralEntity && m_EntityToAttach)
		{			
			SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_GeneralEntity.FindComponent(SlotManagerComponent));
			
			if (slotManager)
			{
				EntitySlotInfo entityInSlot = slotManager.GetSlotByName(m_sSlotName);
				
				if(entityInSlot)
				{
					entityInSlot.AttachEntity(m_EntityToAttach);
					vector matrix[4];
					Math3D.AnglesToMatrix(m_Rotation, matrix);
					matrix[3] = m_Position;
					Math3D.MatrixScale(matrix,m_fScale);
					entityInSlot.SetAdditiveTransformLS(matrix);
					m_GeneralEntity.Update();
					m_EntityToAttach.Update();
				}
			}
		}
	}
}
