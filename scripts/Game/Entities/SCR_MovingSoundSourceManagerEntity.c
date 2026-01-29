[EntityEditorProps(category: "GameScripted/Sound", description: "Spawns moving sounds around camera")]
class SCR_MovingSoundSourceEntityClass : GenericEntityClass
{
};

class SCR_MovingSoundSourceEntity : GenericEntity
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Sound project name (acp)", "acp")]
	protected ResourceName m_sSoundProjectName;
	
	[Attribute("", UIWidgets.Auto, desc: "Sound event name that will be triggered",)]
	protected string m_SoundEventName;
	
	[Attribute("0", UIWidgets.Slider, "[s]", "0 1000 1")]
	protected int m_iTriggerInterval;
	
	[Attribute("0", UIWidgets.Slider, "[m]", "0 10000 1")]
	protected int m_iTrajectoryHeight;
	
	[Attribute("0", UIWidgets.Slider, "[m]", "0 10000 1")]
	protected int m_iTrajectoryDistance;
	
	[Attribute("0", UIWidgets.Slider, "[m]", "0 10000 1")]
	protected int m_iTrajectoryLenght;
	
	[Attribute("0", UIWidgets.Slider, "[m/s]", "0 1000 1")]
	protected int m_iMovementSpeed;
		
	private float m_fTime = 30;
	
	private ref array<ref SCR_MoveData>  m_aMoveData = new array<ref SCR_MoveData>;
		
	//------------------------------------------------------------------------------------------------
	void PlayMoveSound(float timeSlice)
	{
		m_fTime += timeSlice;
		
		if (m_fTime <= m_iTriggerInterval)
			return;
		
		m_fTime = 0;
		
		SCR_MoveData moveData = GetMoveData();
				
		vector mat[4];	
		Math3D.MatrixIdentity4(mat);		
		mat[3] = moveData.m_vPosition;
		
		moveData.m_AudioHandle = AudioSystem.PlayEvent(m_sSoundProjectName, m_SoundEventName, mat);
		
		if (moveData.m_AudioHandle == AudioHandle.Invalid)
			return;
										
		m_aMoveData.Insert(moveData);
	}
	
	
	//------------------------------------------------------------------------------------------------
	SCR_MoveData GetMoveData()
	{
		// Get camera position
		vector mat[4];
		this.GetWorld().GetCurrentCamera(mat);	
		vector cameraPosition = mat[3];
		
		SCR_MoveData moveData = new SCR_MoveData;
		
		// Get start and end point
		int side = Math.RandomIntInclusive(0, 1);
		if( side == 0)
			side = -1;
		
		const int trajectoryDistance = cameraPosition[0] - m_iTrajectoryDistance * side;
		
		vector A, B;
		
		A[0] = trajectoryDistance;
		B[0] = trajectoryDistance;
		
		const float trajectoryLenght = m_iTrajectoryLenght * 0.5 * side;
		
		A[2] = cameraPosition[2] + trajectoryLenght;
		B[2] = cameraPosition[2] - trajectoryLenght;
		
		// Get height
		A[1] = m_iTrajectoryHeight * Math.RandomFloat(0.8, 1.2);
		B[1] = m_iTrajectoryHeight * Math.RandomFloat(0.8, 1.2);
		
		// Get time
		moveData.m_fOveralMoveTime =  m_iTrajectoryLenght /  m_iMovementSpeed;
		
		// Get velocity		
		vector velocity = B - A;
		velocity /= moveData.m_fOveralMoveTime;
		
		moveData.m_vPosition = A;
		moveData.m_vVelocity = velocity;
				
		return moveData;	
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateFlySoundPosition(float timeSlice)
	{
		int count = m_aMoveData.Count();
		
		if (count == 0)
			return;
		
		count--;
		
		for (int i = count; i >= 0; i--)
		{
			SCR_MoveData moveData = m_aMoveData[i];
			
			// Sound finished playing
			if (AudioSystem.IsSoundPlayed(moveData.m_AudioHandle))
			{
				m_aMoveData.Remove(i);
				return;
			}

			moveData.m_fMoveTime += timeSlice;
			
			// Remove when movement is over
			if(moveData.m_fMoveTime >= moveData.m_fOveralMoveTime)
			{
				AudioSystem.TerminateSound(moveData.m_AudioHandle);
				m_aMoveData.Remove(i);
				return;
			}
						
			moveData.m_vPosition += moveData.m_vVelocity * timeSlice;
			
			vector mat[4];
			Math3D.MatrixIdentity4(mat);
			mat[3] = moveData.m_vPosition;
						
			AudioSystem.SetSoundTransformation(moveData.m_AudioHandle, mat);
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	void Terminate()
	{		
		foreach(SCR_MoveData moveData : m_aMoveData)
		{
			AudioSystem.TerminateSoundFadeOut(moveData.m_AudioHandle, false, 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{				
		PlayMoveSound(timeSlice);		
		UpdateFlySoundPosition(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MovingSoundSourceEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.POSTFRAME | EntityEvent.INIT);
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MovingSoundSourceEntity()
	{
		Terminate();
	}
		
}