//------------------------------------------------------------------------------------------------
//! Who decides when an SCR_AIGroup transitions between Dormant (members despawned) and Active
//! (members materialised)?
//!
//!   Manual           External code (scenario scripts, spawner components, GM tools) drives the
//!                    group by calling SpawnMembers / DespawnMembers directly. Default.
//!   ProximityDriven  The group monitors ObserversSystem on a coarse timer and auto-materialises
//!                    when an observer is within m_fSpawnDistance, despawns when all observers
//!                    are beyond m_fDespawnDistance. Spawners that own the group set this policy
//!                    once on creation and then walk away.
enum SCR_EAIGroupLifecyclePolicy
{
	Manual          = 0,
	ProximityDriven = 1
};
