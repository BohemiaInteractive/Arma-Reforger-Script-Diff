/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup WorldSystems
\{
*/

enum WorldSystemPoint
{
	//! Update point at the beginning of the frame update loop.
	FrameStartBegin,
	//! Update point before EntityEvent.FRAME.
	FrameStart,
	//! Update point after EntityEvent.FRAME.
	Frame,
	//! Update point before EntityEvent.FIXEDFRAME.
	BeforeFixedFrame,
	//! Update point after EntityEvent.FIXEDFRAME.
	FixedFrame,
	//! Update point after all fixed-frame steps.
	AfterSimulation,
	//! Update point before physics substeps.
	BeforePhysics,
	//! Update point before EntityEvent.SIMULATE.
	SimulatePhysics,
	//! Update point before EntityEvent.POSTSIMULATE.
	PostSimulatePhysics,
	//! Update point after physics substeps.
	AfterPhysics,
	//! Update point after EntityEvent.POSTFRAME.
	PostFrame,
	//! Update point before EntityEvent.POSTFIXEDFRAME.
	BeforePostFixedFrame,
	//! Update point after EntityEvent.POSTFIXEDFRAME.
	PostFixedFrame,
	//! Update point after all post-fixed-frame steps.
	AfterPostSimulation,
	//! One-time point during world loading before entities are created.
	BeforeEntitiesCreated,
	//! One-time point during world loading before entities are initialized.
	BeforeEntitiesInitialized,
	//! One-time point during world loading after entities are initialized.
	EntitiesInitialized,
	//! One-time point during world loading after replication runtime has started.
	RuntimeStarted,
}

/*!
\}
*/
