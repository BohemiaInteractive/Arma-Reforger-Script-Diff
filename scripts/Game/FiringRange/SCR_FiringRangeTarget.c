[EntityEditorProps(category: "GameScripted/FiringRange", description: "Handeles behaviour of target. Evaluates a hit.", color: "0 0 255 255")]
class SCR_FiringRangeTargetClass: BaseBuildingClass
{
	
};

enum ETargetState
{
	TARGET_UP = 0,
	TARGET_DOWN = 1,
};

enum ETargetMode
{
	TRAINING = 1,
	COMPETITION = 2,
};

class SCR_FiringRangeTarget : BaseBuilding
{

	//! Current state at any given time
	protected int m_iDesiredState;
	
	//! Delay for how long target stays down.
	[Attribute("3", UIWidgets.Slider, "Target in stays in folded position for... seconds", "1 100 3")] 
	protected float m_fTimeInFoldedPosition;

	//! Distance of the target
	[Attribute("1", UIWidgets.Slider, "How far is the target from shooting line.", "1 9999 50")] 
	protected int m_iTargetDistance;
	
	//! Time, for how long target stays in erected position.
	[Attribute("3", UIWidgets.Slider, "Target stays in erected position for... seconds", "0.5 10 0.5")] 
	protected float m_fTimeInErectedPosition;
	
	//! Set if the target is firing line indicator or regular target	
	[Attribute( defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Set if the target has to be used as hit indicator." )]
	protected bool m_bIsIndicator;
	
	// If true, target is reset to standing position after set time
	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Set if the target should be reset to erected position." ), RplProp()]
	private bool m_bAutoResetTarget;
	
	//Internal value for keeping target down for a while
	[RplProp()]
	protected bool m_bWaitInPosition = false;
	
	// Signal manager to pass signals into proc anim
	private SignalsManagerComponent m_SignalManager;
	private int m_iTargetHitSignal;
	
	// Firing range controller
	private SCR_FiringRangeController m_LineController;
	private SCR_FiringRangeManager m_FiringRangeManager;
	
	// Proc anim component
	private ProcAnimComponent m_ProcAnimComponent;
		
	// RPL component
	private RplComponent m_RplComponent;
	
	// time for how long the target should stay folded
	protected float m_fFallenTargetTimer;
	
	// Variable preventing multiple hit evaluation
	protected bool m_bTargetHit = false;
		
	// Owner of the target (parent entity)
	private IEntity m_OwnerEntity = null;
	
	// Target owenr (player)
	private int m_iTargetOwner = -1;
	
	//--- Target modes
	private int m_iTargetMode = -1;
	
	// value of hit (score)
	private int m_iHitValue; 
	
	// state of the target for JIP 
	private int m_iTargetState;
	
	private IEntity m_FiringLineController;
	
	ref ScriptInvoker Event_TargetChangeState = new ScriptInvoker;
	
	// target scoring points 
	const float SCORE_CENTER   = 10;
	const float SCORE_CIRCLE_1 =  5;
	const float SCORE_CIRCLE_2 =  3;
	const float SCORE_CIRCLE_3 =  1;
	
	// definition of areas on target for scoring
	// 10 points area
	const float AREA_10_W_LEFT = 	0.03;	
	const float AREA_10_W_RIGHT =  -0.03;
	const float AREA_10_H_UP = 		1.15;
	const float AREA_10_H_DOWN = 	1.03;
		
	// 5 points area
	const float AREA_5_W_LEFT = 	0.08;	
	const float AREA_5_W_RIGHT =   -0.08;
	const float AREA_5_H_UP = 		1.2;
	const float AREA_5_H_DOWN = 	0.98;
	
	// 3 points area
	const float AREA_3_W_LEFT = 	0.13;	
	const float AREA_3_W_RIGHT =   -0.13;
	const float AREA_3_H_UP = 		1.32;
	const float AREA_3_H_DOWN = 	0.84;
	
	// 1 point area
	const float AREA_1_W_LEFT = 	0.28;	
	const float AREA_1_W_RIGHT =   -0.28;
	const float AREA_1_H_UP = 		1.77;
	const float AREA_1_H_DOWN = 	0.5;
	
	const float AREA_BOTTOM_VALID = 0.40;
	
	//------------------------------------------------------------------------------------------------
	override void OnDamage(float damage,
				  EDamageType type,
				  IEntity pHitEntity, 
				  inout vector outMat[3], // Hit position/direction/normal
				  IEntity damageSource, 
				  notnull Instigator instigator, 
				  int colliderID, 
				  float speed)
	{

		if (IsProxy())
			return;
		
		// don't continue if projectile hit the stand of the target. 
		if (!IsHitValid(CoordToLocal(outMat[0])))
			return;
		
		super.OnDamage(damage,type,pHitEntity,outMat,damageSource,instigator,colliderID,speed);
		
		// check if target is in erected state
		if (GetState() != ETargetState.TARGET_UP)
 			return;

		// target was already hit. Multiple execution check
		if (m_bTargetHit)
			return;
		m_bTargetHit = true;

		SetState(ETargetState.TARGET_DOWN);
		
		// activate entity EOnFrame
		SetEventMask(EntityEvent.FRAME);
		
		// get lobby
		// get ID of the player who hits the target	
		int playerID = instigator.GetInstigatorPlayerID();
		
		// check if the player is also the owner of the target or the target is in training mode
		if (m_iTargetOwner == playerID || m_iTargetMode == ETargetMode.TRAINING)
		{		
			// Check if the Firing range manager exists
			if (!m_FiringRangeManager)
				return;
			
			if (GetValidityOfHit(CoordToLocal(outMat[0]))) 
			m_FiringRangeManager.AddIndicator(CoordToLocal(outMat[0]),VectorToLocal(outMat[2] * -0.1), pHitEntity);
		}
		
		// only if owner of the target is the player, count score
		if (m_iTargetOwner == playerID && m_iTargetMode == ETargetMode.COMPETITION)
		{
			GetHitValue(CoordToLocal(outMat[0]),playerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets desired animation state
	void SetState(int state)
	{
		Rpc(SetStateMP, state);
		SetStateMP(state);
		Replication.BumpMe();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void SetStateMP(int state)
	{
		m_iDesiredState = state;
		Event_TargetChangeState.Invoke(state, this);
		if (m_iDesiredState == ETargetState.TARGET_DOWN)
		{
			// Reset time for how long the target should lie down.
			m_fFallenTargetTimer = 0;
			// Start timer which will erect target.
			m_bWaitInPosition = true;
			// Animate target
			if (m_SignalManager)
				m_SignalManager.SetSignalValue(m_iTargetHitSignal, ETargetState.TARGET_DOWN);
			// Play sound
			SCR_SoundManagerModule.CreateAndPlayAudioSource(this, SCR_SoundEvent.SOUND_TARGET_DOWN); 		
			// Count the targets 
			if (m_LineController)
				m_LineController.CountPopUpTargets();	
			Replication.BumpMe();
		}
		else
		{
			m_fFallenTargetTimer = 0;
			m_bTargetHit = false;
			m_bWaitInPosition = false;
			// Animate target
			if (m_SignalManager)
				m_SignalManager.SetSignalValue(m_iTargetHitSignal, ETargetState.TARGET_UP);
			// Play sound
			SCR_SoundManagerModule.CreateAndPlayAudioSource(this, SCR_SoundEvent.SOUND_TARGET_UP); 
		
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return if is proxy or not
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return states of target <0, 1> standing or lies
	int GetState()
	{
		if (!m_SignalManager)
			return 0;
		return m_SignalManager.GetSignalValue(m_SignalManager.FindSignal("target_hit"));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return set distance of target from firing line
	int GetSetDistance()
	{
		return m_iTargetDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return current owner of the target
	int GetTargetOwner()
	{
		return m_iTargetOwner;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set current owner of the target
	void SetTargetOwner(int playerID)
	{
		m_iTargetOwner = playerID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Seter of target mode
	void SetTargetMode(int targetMode)
	{
		m_iTargetMode = targetMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return time, for how long the target should be in erected state
	int GetErectedDuration()
	{
		//recalculate seconds to miliseconds
		return m_fTimeInErectedPosition * 1000;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true, if the hit is valid (player hit the area of the target, where we actually can calculate any score).
	bool GetValidityOfHit(vector coordOfHit)
	{
		float HitY = coordOfHit[1];
		if (HitY > AREA_1_H_DOWN)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns state of autoreset
	void SetAutoResetTarget(bool state)
	{
		m_bAutoResetTarget = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check what part of the target projectile hit.
	bool IsHitValid(vector coordOfHit)
	{
		float HitX = coordOfHit[0];
		float HitY = coordOfHit[1];
		
		if (HitX < AREA_1_W_LEFT && HitX > AREA_1_W_RIGHT && HitY < AREA_1_H_UP && HitY > AREA_BOTTOM_VALID)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return true when the target is used as a hit indicator. Flase, if not.
	bool IsIndicator()
	{
		return m_bIsIndicator;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Evaluate the hit coordinates and add a propper score
	void GetHitValue(vector coordOfHit, int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return;
				
		float HitX = coordOfHit[0];
		float HitY = coordOfHit[1];
				
		if (HitX < AREA_10_W_LEFT && HitX > AREA_10_W_RIGHT && HitY < AREA_10_H_UP && HitY > AREA_10_H_DOWN)
			m_iHitValue = SCORE_CENTER;
		
		else if (HitX < AREA_5_W_LEFT && HitX > AREA_5_W_RIGHT && HitY < AREA_5_H_UP && HitY > AREA_5_H_DOWN)
			m_iHitValue = SCORE_CIRCLE_1;
		
		else if (HitX < AREA_3_W_LEFT && HitX > AREA_3_W_RIGHT && HitY < AREA_3_H_UP && HitY > AREA_3_H_DOWN)
			m_iHitValue = SCORE_CIRCLE_2;
		
		else if (HitX < AREA_1_W_LEFT && HitX > AREA_1_W_RIGHT && HitY < AREA_1_H_UP && HitY > AREA_1_H_DOWN)
			m_iHitValue = SCORE_CIRCLE_3;
		
		m_FiringRangeManager.CountPlayerScore(m_iTargetOwner, m_iHitValue);
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// This counter runs, if the target was animated down after hit and at the same time m_bAutoResetTarget is true (that means target is in default mode, not in competition one)
		if (m_bWaitInPosition && m_bAutoResetTarget)
		{
			if (m_fFallenTargetTimer < m_fTimeInFoldedPosition)
			{
				m_fFallenTargetTimer+= timeSlice;
			}
			else
			{
				// Flip target back to standing position	
				SetState(ETargetState.TARGET_UP);				
				// deactivate entity EOnFrame
				ClearEventMask(EntityEvent.FRAME);
			}	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
    {
        // Save targets state.
		m_iTargetState = GetState();
        writer.Write(m_iTargetState, 1);
        return true;
    }
     
	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		// Load targets state.
        if (!reader.Read(m_iTargetState, 1)) return false;
		SetState(m_iTargetState);
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_SignalManager = SignalsManagerComponent.Cast(FindComponent(SignalsManagerComponent));
		m_ProcAnimComponent = ProcAnimComponent.Cast(FindComponent(ProcAnimComponent));
				
		if (!m_SignalManager)
			return;
		
		// Get the signal index
		m_iTargetHitSignal = m_SignalManager.FindSignal("target_hit");
		SetTargetMode(ETargetMode.TRAINING);
		
		m_OwnerEntity = owner.GetParent();
		if (!m_OwnerEntity)
			return;
		
		m_LineController = SCR_FiringRangeController.Cast(m_OwnerEntity);
		if (!m_LineController)
			return;

		if (m_bIsIndicator)
			m_LineController.SetIndicator(owner);
				
		m_FiringRangeManager = SCR_FiringRangeManager.Cast(m_OwnerEntity.GetParent());
		if (!m_FiringRangeManager)
			return;
		
		m_RplComponent = RplComponent.Cast(m_FiringRangeManager.FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_FiringRangeTarget(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		m_FiringLineController = parent;		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_FiringRangeTarget()
	{
	}
};