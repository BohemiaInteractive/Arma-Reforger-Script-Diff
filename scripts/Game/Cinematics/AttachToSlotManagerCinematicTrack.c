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
		FindEntity(world);
		m_EntityToAttach = GenericEntity.Cast(world.FindEntityByName(m_sEntityToAttach));		
		globalWorld = world;
	}
	
	void FindEntity(World world)
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
			FindEntity(globalWorld);
			
			if (m_sEntityToAttach == "player" && GetGame().GetPlayerController())
				m_EntityToAttach = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
			else
				m_EntityToAttach = GenericEntity.Cast(globalWorld.FindEntityByName(m_sEntityToAttach));	
		}
		
		//Attaching entity to slot
		if (m_GeneralEntity && m_EntityToAttach)
		{
			ChimeraCharacter char = ChimeraCharacter.Cast(m_EntityToAttach);
			SCR_BaseCompartmentManagerComponent compartmentManagerComp = SCR_BaseCompartmentManagerComponent.Cast(m_GeneralEntity.FindComponent(SCR_BaseCompartmentManagerComponent));

			if (char && compartmentManagerComp)
			{
				CompartmentAccessComponent compartmentAccessComponent = char.GetCompartmentAccessComponent();
				BaseCompartmentSlot compartmentSlot = compartmentManagerComp.FindCompartmentByName(m_sSlotName);
				if (!compartmentSlot)
				{
					Debug.ErrorFormat("AttachToSlotManagerCinematicTrack is misconfigured, cannot find slot by name of \"%1\"", m_sSlotName);
					return;
				}

				compartmentAccessComponent.GetInVehicle(m_GeneralEntity, compartmentSlot, true, 0, ECloseDoorAfterActions.CLOSE_DOOR, false);
				return;
			}

			SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_GeneralEntity.FindComponent(SlotManagerComponent));
			if (!slotManager)
				return;

			EntitySlotInfo entityInSlot = slotManager.GetSlotByName(m_sSlotName);	
			if(!entityInSlot)
				return;

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
