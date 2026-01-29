[CinematicTrackAttribute(name:"Bone Animation Track", description:"Track used for animating bones of entity slot")]
class SlotBoneAnimationCinematicTrack : CinematicTrackBase
{
	[Attribute("")]
	string m_sSlotName;
	
	[Attribute("")]
	string m_sBoneName;
	
	[Attribute("0 0 0")]
	vector m_YawPitchRoll;
	
	[Attribute("0 0 0")]
	vector m_Position;
	
	private GenericEntity m_GeneralEntity;
	private IEntity m_SlotEntity;
	private World globalWorld;
	
	override void OnInit(World world)
	{
		// Find particle entity by using name of track
		findEntity(world);
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
		}
		
		if (m_GeneralEntity)
		{			
				
			//Animating bone on object
			if (m_sSlotName == "")
			{
				Animation anim = m_GeneralEntity.GetAnimation();
				int boneId = anim.GetBoneIndex(m_sBoneName);			
				anim.SetBone(m_GeneralEntity, boneId, m_YawPitchRoll, m_Position, 1.0);							
				m_GeneralEntity.Update();
			}
			else //Animating bone on object inside slot
			{
				SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_GeneralEntity.FindComponent(SlotManagerComponent));
				
				if (slotManager)
				{
					EntitySlotInfo entityInSlot = slotManager.GetSlotByName(m_sSlotName);
					
					if(entityInSlot)
					{
						m_SlotEntity = entityInSlot.GetAttachedEntity();
					
						if(m_SlotEntity)
						{
							Animation anim = m_SlotEntity.GetAnimation();
							
							if (anim)
							{
								int boneId = anim.GetBoneIndex(m_sBoneName);
								anim.SetBone(m_SlotEntity, boneId, m_YawPitchRoll, m_Position, 1.0);
							}
							
							m_GeneralEntity.Update();
						}
					}
				}
			}
		}
	}
}
