[CinematicTrackAttribute(name:"Sound Track", description:"Track used for triggering sound")]
class SCR_SoundCinematicTrack : SCR_CinematicTrackBase
{
	[Attribute("")]
	protected string m_sResourceName;
	
	[Attribute("")]
	protected string m_sSoundEvent;
	
	[Attribute("")]
	protected string m_sEntityName;
	
	[Attribute("")]
	protected string m_sBoneName;
	
	[Attribute("")]
	protected vector m_vOffset;
	
	[Attribute("")]
	protected string m_sSignalName1;
	
	[Attribute("")]
	protected float m_fSignalValue1;
	
	[Attribute("")]
	protected string m_sSignalName2;
	
	[Attribute("")]
	protected float m_fSignalValue2;
	
	[Attribute("")]
	protected string m_sSignalName3;
	
	[Attribute("")]
	protected float m_fSignalValue3;
	
	[Attribute("")]
	protected string m_sSignalName4;
	
	[Attribute("")]
	protected float m_fSignalValue4;
	
	[Attribute()]
	protected bool m_bUpdatePosition;
	
	[Attribute(desc:"Passes GRoomSize variable to RoomSize signal")]
	protected bool m_bSetRoomSize;
	
	[Attribute(desc:"Passes GInterior variable to Interior signal")]
	protected bool m_bSetInterior;
	
	AudioHandle m_AudioHandle = AudioHandle.Invalid;
	
	override void OnApply(float time)
	{
		if (m_AudioHandle == AudioHandle.Invalid)
		{
			return;
		}
		
		if (AudioSystem.IsSoundPlayed(m_AudioHandle))
		{
			m_AudioHandle = AudioHandle.Invalid;
		}
				
		if (m_bUpdatePosition)
		{
			vector mat[4];
			GetTransformation(mat);
			
			AudioSystem.SetSoundTransformation(m_AudioHandle, mat);
		}
	}
			
	[CinematicEventAttribute()]
	void PlayEvent()
	{
		if (m_sResourceName.IsEmpty() || m_sSoundEvent.IsEmpty())
		{
			return;
		}
		
		array<string> signalNames = {};
		array<float> signalValues = {};
		
				
		// Handle signals
		if (!m_sSignalName1.IsEmpty())
		{
			signalNames.Insert(m_sSignalName1);
			signalValues.Insert(m_fSignalValue1);
		}
		
		if (!m_sSignalName2.IsEmpty())
		{
			signalNames.Insert(m_sSignalName2);
			signalValues.Insert(m_fSignalValue2);
		}
			
		if (!m_sSignalName3.IsEmpty())
		{
			signalNames.Insert(m_sSignalName3);
			signalValues.Insert(m_fSignalValue3);
		}
				
		if (!m_sSignalName4.IsEmpty())
		{
			signalNames.Insert(m_sSignalName4);
			signalValues.Insert(m_fSignalValue4);
		}
		
		if (m_bSetRoomSize)
		{
			const float gRoomSize = AudioSystem.GetVariableValue("GRoomSize", "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf");	
			signalNames.Insert("RoomSize");
			signalValues.Insert(gRoomSize);
		}
		
		if (m_bSetInterior)
		{
			const float gInterior = AudioSystem.GetVariableValue("GInterior", "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf");	
			signalNames.Insert("Interior");
			signalValues.Insert(gInterior);
		}
		
		// Handle offset
		vector mat[4];
		GetTransformation(mat);
		
		// Play sound
		m_AudioHandle = AudioSystem.PlayEvent(m_sResourceName, m_sSoundEvent, mat, signalNames, signalValues);
	}
	
	[CinematicEventAttribute()]
	void TerminateEvent()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
	}
	
	void GetTransformation(out vector mat[4])
	{
		IEntity owner;
		
		if (m_sEntityName.IsEmpty())
		{
			Math3D.MatrixIdentity4(mat);
			mat[3] = m_vOffset;
		}
		else
		{
			owner = FindOwner(m_sEntityName);
			if (owner)
			{
				if (!m_sBoneName.IsEmpty())
				{
					Animation animation = owner.GetAnimation();
					if (!animation)
					{
						owner.GetTransform(mat);
						return;
					}
					
					TNodeId	boneIndex = animation.GetBoneIndex(m_sBoneName);	
					animation.GetBoneMatrix(boneIndex, mat);
					mat[3] = owner.CoordToParent(mat[3]);
				}
				else 
				{
					owner.GetTransform(mat);
					
					if (m_vOffset != vector.Zero)
					{
						mat[3] = owner.CoordToParent(m_vOffset);
					}
				}
			}
			else
			{
				Math3D.MatrixIdentity4(mat);
				mat[3] = m_vOffset;
			}
		}
	}
}
