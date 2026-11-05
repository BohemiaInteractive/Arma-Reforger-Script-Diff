[ComponentEditorProps(category: "GameScripted/Sound", description: "")]
class SCR_CrowdSoundComponentClass : SoundComponentClass
{
}

class SCR_CrowdSoundComponent : SoundComponent
{	
	[Attribute()]
	protected vector m_vDimensions;
	
	[Attribute()]
	protected float m_fCrowdSize;
	
	protected AudioHandle m_audioHandle = AudioHandle.Invalid;
		
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		if (IsFinishedPlaying(m_audioHandle))
		{
			if (m_fCrowdSize < 5.0)
			{
				m_audioHandle = SoundEvent("SOUND_RALLY_CROWD_SMALL_LP");
			}
			else if (m_fCrowdSize < 20.0)
			{
				m_audioHandle = SoundEvent("SOUND_RALLY_CROWD_MEDIUM_LP");
			}
			else
			{
				m_audioHandle = SoundEvent("SOUND_RALLY_CROWD_HUGE_LP");
			}
			
			const SCR_CrowdSoundComponentClass componentData = SCR_CrowdSoundComponentClass.Cast(GetComponentData(owner));
			if (!componentData)
			{
				return;
			}
			
			//vector dimensions = componentData.GetDimensions();
			vector dimensions = m_vDimensions;
			AudioSystem.SetBoundingVolumeParams(m_audioHandle, 2, dimensions[0], dimensions[1], dimensions[2]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CrowdSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);
	}
}