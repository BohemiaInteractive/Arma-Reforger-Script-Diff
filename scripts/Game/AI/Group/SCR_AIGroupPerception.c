void SCR_AIOnTargetClusterStateDeleted(SCR_AITargetClusterState c);
typedef func SCR_AIOnTargetClusterStateDeleted;

void SCR_AIGroupPerceptionOnNoEnemy(SCR_AIGroup group);
typedef func SCR_AIGroupPerceptionOnNoEnemy;

void SCR_AIGroupPerceptionOnEnemyDetected(SCR_AIGroup group, SCR_AITargetInfo target);
typedef func SCR_AIGroupPerceptionOnEnemyDetected;

void SCR_AIGroupPerceptionOnEnemyDetectedFiltered(SCR_AIGroup group, SCR_AITargetInfo target, AIAgent reporter);
typedef func SCR_AIGroupPerceptionOnEnemyDetectedFiltered;

class SCR_AIGroupPerception : Managed
{
	// Target is considered lost if noone has seen it for this time.
	// This value should be consistent with actual duration of AI combat behavior (see TARGET_MAX_LAST_SEEN)
	const float TARGET_LOST_THRESHOLD_S = 10.0;
	
	// Time till target is totally forgotten and is removed from memory.
	// This doesn't need to be much longer than MAX_CLUSTER_AGE_S
	const float TARGET_FORGET_THRESHOLD_S = 150.0;
	
	protected SCR_AIGroup m_Group;
	protected SCR_AIGroupUtilityComponent m_Utility;
	
	protected ref ScriptInvokerBase<SCR_AIOnTargetClusterStateDeleted> Event_OnTargetClusterStateDeleted = new ScriptInvokerBase<SCR_AIOnTargetClusterStateDeleted>();
	protected ref ScriptInvokerBase<SCR_AIGroupPerceptionOnEnemyDetected> Event_OnEnemyDetected;
	protected ref ScriptInvokerBase<SCR_AIGroupPerceptionOnEnemyDetectedFiltered> Event_OnEnemyDetectedFiltered;
	protected ref ScriptInvokerBase<SCR_AIGroupPerceptionOnNoEnemy> Event_OnNoEnemy;
	
	ref array<IEntity> m_aTargetEntities = new array<IEntity>;						// Read only! Don't dare to modify it!
	ref array<ref SCR_AITargetInfo> m_aTargets = new array<ref SCR_AITargetInfo>;	// Read only!
	ref array<ref SCR_AIGroupTargetCluster> m_aTargetClusters = {};					// Read only!
	SCR_AIGroupTargetCluster m_MostDangerousCluster;								// Read only!
	
	//---------------------------------------------------------------------------------------------------
	void SCR_AIGroupPerception(SCR_AIGroupUtilityComponent utility, SCR_AIGroup group)
	{
		m_Group = group;
		m_Utility = utility;
	}
	
	//---------------------------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIOnTargetClusterStateDeleted> GetOnTargetClusterDeleted()
	{
		return Event_OnTargetClusterStateDeleted;
	}
	
	//---------------------------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIGroupPerceptionOnEnemyDetected> GetOnEnemyDetected()
	{
		if (!Event_OnEnemyDetected)
			Event_OnEnemyDetected = new ScriptInvokerBase<SCR_AIGroupPerceptionOnEnemyDetected>();
		
		return Event_OnEnemyDetected;
	}
	
	//---------------------------------------------------------------------------------------------------
	//! This is similar to OnEnemyDetected event, but it will be invoked only once per group perception update,
	//! You should use this if you don't need every detected target, but need to know when anything has been detected
	ScriptInvokerBase<SCR_AIGroupPerceptionOnEnemyDetectedFiltered> GetOnEnemyDetectedFiltered()
	{
		if (!Event_OnEnemyDetectedFiltered)
			Event_OnEnemyDetectedFiltered = new ScriptInvokerBase<SCR_AIGroupPerceptionOnEnemyDetectedFiltered>();
		
		return Event_OnEnemyDetectedFiltered;
	}
	
	//---------------------------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIGroupPerceptionOnNoEnemy> GetOnNoEnemy()
	{
		if (!Event_OnNoEnemy)
			Event_OnNoEnemy = new ScriptInvokerBase<SCR_AIGroupPerceptionOnNoEnemy>();
		
		return Event_OnNoEnemy;
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void RemoveTarget(IEntity enemy)
	{
		if (!enemy)
			return;
		
		int index = m_aTargetEntities.Find(enemy);
		
		if (index > -1)
		{
			m_aTargetEntities.Remove(index);
			m_aTargets.Remove(index);
			if (m_aTargetEntities.IsEmpty() && Event_OnNoEnemy)
				Event_OnNoEnemy.Invoke(m_Group);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void RemoveTarget(int id)
	{
		m_aTargetEntities.Remove(id);
		m_aTargets.Remove(id);
		
		if (m_aTargetEntities.IsEmpty() && Event_OnNoEnemy)
			Event_OnNoEnemy.Invoke(m_Group);
	}
	
	//---------------------------------------------------------------------------------------------------
	// Adds or updates target from BaseTarget
	SCR_AITargetInfo AddOrUpdateTarget(notnull BaseTarget target, out bool outNewTarget)
	{
		IEntity enemy = target.GetTargetEntity();
		
		if (!enemy)
		{
			outNewTarget = false;
			return null;
		}
			
		int id = m_aTargetEntities.Find(enemy);
		if (id > -1)
		{
			SCR_AITargetInfo oldTargetInfo = m_aTargets[id];
			EAITargetInfoCategory oldCategory = m_aTargets[id].m_eCategory;
			
			// Ignore if destroyed or disarmed
			if (oldCategory == EAITargetInfoCategory.DESTROYED || oldCategory == EAITargetInfoCategory.DISARMED)
			{
				outNewTarget = false;
				return oldTargetInfo;
			}
				
			// This target was already found
			// Which newTimestamp to use? Depends on target category
			float newTimestamp;
			ETargetCategory targetCategory = target.GetTargetCategory();
			if (targetCategory == ETargetCategory.DETECTED)
				newTimestamp = target.GetTimeLastDetected();
			else
				newTimestamp = target.GetTimeLastSeen();
			
			if (oldTargetInfo.m_fTimestamp < newTimestamp)
			{
				// New information is newer
				// Is new data more relevant?
				if ((targetCategory == ETargetCategory.ENEMY) || // If enemy, always update
					((targetCategory == ETargetCategory.DETECTED) && (oldCategory != EAITargetInfoCategory.IDENTIFIED)) )
				{
					oldTargetInfo.UpdateFromBaseTarget(target);
				}
			}
			
			outNewTarget = false;
			return oldTargetInfo;
		}		
		
		// new enemy found
		
		// Ignore if disarmed
		if (target.IsDisarmed())
		{
			outNewTarget = false;
			return null;
		}
			
		SCR_AITargetInfo targetInfo = new SCR_AITargetInfo();
		targetInfo.InitFromBaseTarget(target);
				
		m_aTargetEntities.Insert(enemy);
		m_aTargets.Insert(targetInfo);
		
		if (Event_OnEnemyDetected)
		{
			Event_OnEnemyDetected.Invoke(m_Group, targetInfo);
		}
		
		outNewTarget = true;
		return targetInfo;
	}
	
	//---------------------------------------------------------------------------------------------------
	void AddOrUpdateGunshot(notnull IEntity shooter, vector worldPos, Faction faction, float timestamp, bool endangering)
	{
		int id = m_aTargetEntities.Find(shooter);
		if (id > -1)
		{
			// Update data
			SCR_AITargetInfo oldTargetInfo = m_aTargets[id];
			if ((oldTargetInfo.m_eCategory != EAITargetInfoCategory.IDENTIFIED) &&	// Update only if it wasn't seen yet
				(timestamp > oldTargetInfo.m_fTimestamp))					// Update only if new data is newer
			{
				oldTargetInfo.UpdateFromGunshot(worldPos, timestamp, endangering);
			}
			
			// Update endangering flag
			oldTargetInfo.m_bEndangering |= endangering;
		}
		else
		{
			// Create new data
			PerceivableComponent perceivable = PerceivableComponent.Cast(shooter.FindComponent(PerceivableComponent));
			
			// Ignore aircrafts. Fix to prevent suppression of aircrafts,
			// but also there is no reason to track aircrafts in group perception.
			if (perceivable && perceivable.GetUnitType() == EAIUnitType.UnitType_Aircraft)
				return;
			
			SCR_AITargetInfo targetInfo = new SCR_AITargetInfo();
			targetInfo.InitFromGunshot(shooter, perceivable, worldPos, faction, timestamp, endangering);
			
			m_aTargets.Insert(targetInfo);
			m_aTargetEntities.Insert(shooter);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	void Update()
	{
		UpdateTargetsFromMembers();	// These calls ...
		MaintainTargets();			// ... must be ...		|	<- 	this one removed targets
		ClusterTargets();			// ... in this order.	|		thus clustering must happen after it
		
		SelectMostDangerousCluster();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void UpdateTargetsFromMembers()
	{
		IEntity leaderEntity = m_Group.GetLeaderEntity();
		
		// Makes no sense without a leader
		if (!leaderEntity)
			return;
		
		// Update all targets from our group members
		array<AIAgent> agents = {};
		m_Group.GetAgents(agents);
		
		array<BaseTarget> targets = {};
		
		bool targetIsNew;
		SCR_AITargetInfo targetInfo;
		bool invokedEvent = false;
		
		foreach (SCR_AIInfoComponent infoComp : m_Utility.m_aInfoComponents)
		{
			PerceptionComponent perception = infoComp.m_Perception;
			
			targets.Clear();
			perception.GetTargetsList(targets, ETargetCategory.DETECTED);
			foreach (BaseTarget baseTarget : targets)
			{
				PerceivableComponent perceivable = baseTarget.GetPerceivableComponent();
				if (!perceivable)
					continue;
				
				// Ignore if target is in vehicle, from group perspective we don't care about vehicle occupants
				if (perceivable.IsInCompartment())
					continue;
				
				// Ignore aircrafts. Fix to prevent suppression of aircrafts,
				// but also there is no reason to track aircrafts in group perception.
				if (perceivable.GetUnitType() == EAIUnitType.UnitType_Aircraft)
					continue;
				
				targetInfo = AddOrUpdateTarget(baseTarget, targetIsNew);
				
				if (targetIsNew && !invokedEvent)
				{
					if (Event_OnEnemyDetectedFiltered)
					{
						AIAgent reporter = AIAgent.Cast(infoComp.GetOwner());
						Event_OnEnemyDetectedFiltered.Invoke(m_Group, targetInfo, reporter);
					}
					invokedEvent = true;
				}
			}
			
			targets.Clear();
			perception.GetTargetsList(targets, ETargetCategory.ENEMY);
			foreach (BaseTarget baseTarget : targets)
			{
				PerceivableComponent perceivable = baseTarget.GetPerceivableComponent();
				if (!perceivable)
					continue;
				
				// Ignore aircrafts. Fix to prevent suppression of aircrafts,
				// but also there is no reason to track aircrafts in group perception.
				if (perceivable.GetUnitType() == EAIUnitType.UnitType_Aircraft)
					continue;
				
				// Ignore if target is in vehicle, from group perspective we don't care about vehicle occupants
				if (perceivable.IsInCompartment())
					continue;
				
				targetInfo = AddOrUpdateTarget(baseTarget, targetIsNew);
				
				if (targetIsNew && !invokedEvent)
				{
					if (Event_OnEnemyDetectedFiltered)
					{
						AIAgent reporter = AIAgent.Cast(infoComp.GetOwner());
						Event_OnEnemyDetectedFiltered.Invoke(m_Group, targetInfo, reporter);
					}
					invokedEvent = true;
				}
			}
		}
	}
	
	//---------------------------------------------------------------------------
	protected void MaintainTargets()
	{
		float timeNow;
		PerceptionManager pm = GetGame().GetPerceptionManager();
		if (pm)
			timeNow = pm.GetTime();
		
		for (int i = m_aTargets.Count()-1; i >= 0; i--)
		{
			SCR_AITargetInfo tgtInfo = m_aTargets[i];
			EAITargetInfoCategory category = tgtInfo.m_eCategory;
			if (timeNow - tgtInfo.m_fTimestamp > TARGET_FORGET_THRESHOLD_S)			// Time to forget?
			{
				RemoveTarget(i);
			}
			else if (category == EAITargetInfoCategory.DISARMED)					// Disarmed?
			{
				if (tgtInfo.m_DamageManager && tgtInfo.m_DamageManager.IsDestroyed())	// Destroyed?
					tgtInfo.m_eCategory = EAITargetInfoCategory.DESTROYED;
				if (tgtInfo.m_Perceivable && !tgtInfo.m_Perceivable.IsDisarmed())		// Not disarmed any more?
					tgtInfo.m_eCategory = EAITargetInfoCategory.DETECTED;				// Back to detected
			}
			else if (category != EAITargetInfoCategory.DESTROYED)					// Not destroyed?
			{					
				if (tgtInfo.m_DamageManager && tgtInfo.m_DamageManager.IsDestroyed())	// Destroyed?
					tgtInfo.m_eCategory = EAITargetInfoCategory.DESTROYED;
				else if (!tgtInfo.m_Entity)												// Deleted? Treat as destroyed.
					tgtInfo.m_eCategory = EAITargetInfoCategory.DESTROYED;
				else if (timeNow - tgtInfo.m_fTimestamp > TARGET_LOST_THRESHOLD_S)		// Lost?
					tgtInfo.m_eCategory = EAITargetInfoCategory.LOST;
				else if (tgtInfo.m_Perceivable && tgtInfo.m_Perceivable.IsDisarmed())	// Disarmed?
					tgtInfo.m_eCategory = EAITargetInfoCategory.DISARMED;
			}
		}
	}
	
	//---------------------------------------------------------------------------
	protected void ClusterTargets()
	{
		vector centerPos = m_Group.GetCenterOfMass();
		
		if (centerPos == vector.Zero) // It's zero when group is empty
			return;
		
		float minAngularDist = Math.DEG2RAD * 30.0;
		array<ref SCR_AIGroupTargetCluster> newClusters = {};
		GenerateClusters(m_aTargets, newClusters, centerPos, minAngularDist);
		
		// Calculate association of old clusters to new clusters
		array<ref array<int>> association = {};
		association.Resize(m_aTargetClusters.Count());
		for (int oldClusterId = 0; oldClusterId < m_aTargetClusters.Count(); oldClusterId++)
		{
			array<int> row = new array<int>();
			row.Resize(newClusters.Count());
			association[oldClusterId] = row;
			
			for (int newClusterId = 0; newClusterId < newClusters.Count(); newClusterId++)
			{
				int associationValue = GetClusterAssociation(m_aTargetClusters[oldClusterId], newClusters[newClusterId]);
				row[newClusterId] = associationValue;
			}
		}
		
		// Transfer previous orders from old clusters to new clusters
		
		array<int> newClusterStateIds = {};				// Index is ID of new cluster, value is ID of old cluster
		newClusterStateIds.Resize(newClusters.Count());	// From which we are transfering state
		for (int i = 0; i < newClusters.Count(); i++)
			newClusterStateIds[i] = -1;
		
		for (int oldClusterId = 0; oldClusterId < m_aTargetClusters.Count(); oldClusterId++)
		{
			// Find the new cluster with which this one associates most
			int maxAssociation = 0;
			int newClusterIdMaxAssociation = -1;
			
			array<int> row = association[oldClusterId];
			for (int i = 0; i < row.Count(); i++)
			{
				if (row[i] > maxAssociation)
				{
					maxAssociation = row[i];
					newClusterIdMaxAssociation = i;
				}
			}
			
			bool transferClusterState = false;
			
			if (newClusterIdMaxAssociation != -1)
			{
				// Old cluster associates with some new cluster
				
				
				int stateTransferedFrom = newClusterStateIds[newClusterIdMaxAssociation];
				if (stateTransferedFrom != -1)
				{
					// There is a conflict, we transfered state to this new cluster already
					// Decide which association value is greater
					
					if (association[stateTransferedFrom][newClusterIdMaxAssociation] < association[oldClusterId][newClusterIdMaxAssociation])
					{
						DeleteClusterState(newClusters[newClusterIdMaxAssociation]);
						transferClusterState = true;
					}
				}
				else
				{
					transferClusterState = true;
				}
			}
			
			if (transferClusterState)
			{
				TransferClusterState(m_aTargetClusters[oldClusterId], newClusters[newClusterIdMaxAssociation]);
				newClusterStateIds[newClusterIdMaxAssociation] = oldClusterId;
			}
			else
			{
				// Can't transfer state from old cluster to any new cluster, delete it
				DeleteClusterState(m_aTargetClusters[oldClusterId]);
			}
		}
		
		// Create state for all new clusters which did not inherit state
		// Process targets in all new clusters
		foreach (SCR_AIGroupTargetCluster c : newClusters)
		{
			if (!c.m_State)
				c.m_State = new SCR_AITargetClusterState(c);
			
			c.m_State.ProcessTargets();
		}
		
		m_aTargetClusters = newClusters;
		
		//DiagPrintClusterAssociation(association);
	}
	
	//---------------------------------------------------------------------------
	void GenerateClusters(array<ref SCR_AITargetInfo> targets, array<ref SCR_AIGroupTargetCluster> outClusters, vector centerPos, float minAngularDist)
	{
		outClusters.Clear();
		
		// Bail if no targets are provided
		if (targets.IsEmpty())
			return;
		
		int nTargets = targets.Count();
		
		//-------------------------------------------
		// Convert all positions to polar coordinates
		array<ref SCR_AITargetInfoPolar> targetsPolar = {};
		targetsPolar.Resize(nTargets);
		
		for (int i = 0; i < nTargets; i++)
		{
			SCR_AITargetInfo target = targets[i];
			vector vdir = target.m_vWorldPos - centerPos; // Not normalized!
			float angle = SCR_AIPolar.DirToAngle(vdir);
			float dist = vector.DistanceXZ(target.m_vWorldPos, centerPos);
			
			SCR_AITargetInfoPolar targetPolar = new SCR_AITargetInfoPolar();
			targetPolar.m_Target = target;
			targetPolar.m_fAngle = angle;
			targetPolar.m_fDistance = dist;
			
			targetsPolar[i] = targetPolar;
		}
		
		//-------------------------------------------
		// Iterate points and make clusters
		
		// First sort by angle
		targetsPolar.Sort(false);
		
		// Iterate targets sorted by angle
	    SCR_AIGroupTargetCluster currentCluster = new SCR_AIGroupTargetCluster();
		currentCluster.AddTarget(targetsPolar[0].m_Target, targetsPolar[0].m_fAngle, targetsPolar[0].m_fDistance);
		currentCluster.m_fAngleMin = targetsPolar[0].m_fAngle;
		currentCluster.m_fAngleMax = currentCluster.m_fAngleMin;
		
	    outClusters.Insert(currentCluster);
   
	    for (int i = 1; i < nTargets; i++)
		{
			SCR_AITargetInfoPolar target = targetsPolar[i];
			
	        float angle = target.m_fAngle;
	        
	        if (angle - currentCluster.m_fAngleMax < minAngularDist)
			{
	            // Add the target
				currentCluster.AddTarget(target.m_Target, angle, target.m_fDistance);
	            currentCluster.m_fAngleMax = angle; // Increase max angle
			}
	        else
			{
	            // Otherwise make a new cluster
	            currentCluster = new SCR_AIGroupTargetCluster();
				currentCluster.AddTarget(target.m_Target, target.m_fAngle, target.m_fDistance);
				currentCluster.m_fAngleMin = target.m_fAngle;
				currentCluster.m_fAngleMax = currentCluster.m_fAngleMin;
	            outClusters.Insert(currentCluster);
			}
		}
		
		// Merge last and first cluster if they are too close
		
		if (outClusters.Count() > 1)
		{
			SCR_AIGroupTargetCluster first = outClusters[0];
			SCR_AIGroupTargetCluster last = outClusters[outClusters.Count()-1];
			
			float distLastToFirst = Math.PI2 - last.m_fAngleMax + first.m_fAngleMin;
			if (distLastToFirst < minAngularDist)
			{
				// Merge last cluster into first one and remove it
				first.AddCluster(last);
				outClusters.RemoveOrdered(outClusters.Count()-1);
			}
		}
		
		// We are done!
	}
	
	//---------------------------------------------------------------------------
	// Returns integer, how cluster 'a' associates with cluster 'b'
	protected int GetClusterAssociation(SCR_AIGroupTargetCluster a, SCR_AIGroupTargetCluster b)
	{
		int intersection = 0;
		array<SCR_AITargetInfo> bTargets = b.m_aTargets;
		foreach (SCR_AITargetInfo aTarget : a.m_aTargets)
		{
			if (aTarget && // Don't associate null-targets
				bTargets.Find(aTarget) != -1)
				intersection++;
		}
		return intersection;
	}
	
	//---------------------------------------------------------------------------
	protected void SelectMostDangerousCluster()
	{
		float maxScore = 0;
		SCR_AIGroupTargetCluster maxScoreCluster;
		
		foreach (SCR_AIGroupTargetCluster c : m_aTargetClusters)
		{
			float score = CalculateClusterDangerScore(c);
			if (score > maxScore)
			{
				maxScore = score;
				maxScoreCluster = c;
			}
		}
		
		m_MostDangerousCluster = maxScoreCluster;
	}
	
	//---------------------------------------------------------------------------
	protected float CalculateClusterDangerScore(SCR_AIGroupTargetCluster c)
	{
		float score = 0;
		
		// distance
		float distClamped = Math.Min(c.m_State.m_fDistMin, 1000.0); 
		score += 1000.0 - distClamped;
		
		// If it's lost, it matters less, it's about to be forgotten
		if (c.m_State.m_eState != EAITargetClusterState.LOST)
			score += 600.0;
		
		// Identified or endangering targets
		score += 200.0 * c.m_State.m_iCountEndangering;
		score += 50.0 * c.m_State.m_iCountDetected;
		score += 100.0 * c.m_State.m_iCountIdentified;
		
		return score;
	}
	
	//---------------------------------------------------------------------------
	protected void DeleteClusterState(SCR_AIGroupTargetCluster c)
	{
		Event_OnTargetClusterStateDeleted.Invoke(c.m_State);
		c.m_State = null;
	}
	
	//---------------------------------------------------------------------------
	protected void TransferClusterState(SCR_AIGroupTargetCluster oldCluster, SCR_AIGroupTargetCluster newCluster)
	{
		newCluster.MoveStateFrom(oldCluster);
	}
	
	//---------------------------------------------------------------------------
	int GetTargetClusterStateId(SCR_AITargetClusterState s)
	{
		foreach (int i, SCR_AIGroupTargetCluster c : m_aTargetClusters)
		{
			if (c.m_State == s)
				return i;
		}
		return -1;
	}
	
	//---------------------------------------------------------------------------
	void DiagPrintClusterAssociation(array<ref array<int>> association)
	{
		Print("Cluster association:");
		for (int oldClusterId = 0; oldClusterId < association.Count(); oldClusterId++)
		{
			array<int> row = association[oldClusterId];
			
			string s = string.Format("%1: ", oldClusterId);
			for (int newClusterId = 0; newClusterId < row.Count(); newClusterId++)
				s = s + string.Format("%1\t", row[newClusterId].ToString());
			
			Print(s);
		} 
	}
	
	
	//---------------------------------------------------------------------------
	void DiagDrawClusters()
	{
		PerceptionManager pm = GetGame().GetPerceptionManager();
		float timeNow_s = pm.GetTime();
		
		foreach (int clusterId, SCR_AIGroupTargetCluster cluster : m_aTargetClusters)
		{
			foreach (SCR_AITargetInfo targetInfo : cluster.m_aTargets)
			{
				vector mrkPos = targetInfo.m_vWorldPos + Vector(0, 3, 0);
				
				string s = string.Empty;
				
				float timeSinceTimestamp_s = timeNow_s - targetInfo.m_fTimestamp;
				s = s + string.Format("C: %1, time-timestamp: %2\n", clusterId.ToString(), timeSinceTimestamp_s.ToString(5, 1));
				
				s = s + string.Format("Category: %1, dngr: %2", typename.EnumToString(EAITargetInfoCategory, targetInfo.m_eCategory), targetInfo.m_bEndangering);
				
				DebugTextWorldSpace.Create(GetGame().GetWorld(), s, DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
					mrkPos[0], mrkPos[1], mrkPos[2],
					color: 0xFFFFFFFF, bgColor: 0xFF000000,
					size: 10.0);
					//float size = 20.0, int , int , int priority = 1000);
			}
			
			vector clusterCenter = cluster.m_State.GetCenterPosition();
			
			// Mark for cluster
			string s = string.Empty;
			
			s = s + string.Format("Cluster: %1\n", clusterId);
			if (cluster.m_State)
			{
				s = s + string.Format("State: %1, Time since Info: %2, Max age: %3",
					typename.EnumToString(EAITargetClusterState, cluster.m_State.m_eState),
					cluster.m_State.GetTimeSinceLastNewInformation().ToString(5,1),
					cluster.m_State.m_fMaxAge_s.ToString(5, 1));
			}
			
			DebugTextWorldSpace.Create(GetGame().GetWorld(), s, DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
					clusterCenter[0], clusterCenter[1] + 10.0, clusterCenter[2],
					color: 0xFFFFFFFF, bgColor: 0xFF000000,
					size: 12.0);
		}
	}
}