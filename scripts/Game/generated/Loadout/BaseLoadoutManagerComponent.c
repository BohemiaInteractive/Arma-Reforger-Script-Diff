/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Loadout
\{
*/

class BaseLoadoutManagerComponentClass: GameComponentClass
{
}

class BaseLoadoutManagerComponent: GameComponent
{
	ref ScriptInvoker<BaseLoadoutManagerComponent> m_OnLoadoutChangedInvoker = new ScriptInvoker<BaseLoadoutManagerComponent>();

	//! Returns the summed forward (chest-depth) offset contributed by all worn clothing. Used to push chest-mounted gadgets forward.
	proto external float GetClothesOffsetFront();

	// callbacks

	event private void OnLoadoutChanged(BaseLoadoutManagerComponent sender) { if (m_OnLoadoutChangedInvoker) m_OnLoadoutChangedInvoker.Invoke(sender); };
}

/*!
\}
*/
