/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

/*!
Set of parameters used by TryUseItemOverrideParams
*/
sealed class ItemUseParameters
{
	// Item which will be used.
	proto external IEntity GetEntity();
	proto external void SetEntity(IEntity entity);
	// Should character be allowed to use movement input while action is being performed. Bool in script has size of enf::uint, not bool.
	proto external bool ShouldAllowMovementDuringAction();
	proto external void SetAllowMovementDuringAction(bool allowed);
	// Should we hide the item after the action is successfully performed? If action is cancelled, item is always left in hand (unless it's cancelled by a different action that hides the item).
	proto external bool ShouldKeepInHandAfterSuccess();
	proto external void SetKeepInHandAfterSuccess(bool keep);
	// Should we keep the gadget in hand, while the animation is playing, even if the gadget is not the item being primarily used?
	proto external bool ShouldKeepGadgetVisible();
	proto external void SetKeepGadgetVisible(bool keepVisible);
	// There can be only one main user of an item, with whom there is proper animation synchronization (incl. timing correction, etc.). If it is possible to attach multiple users to the item, all short-term users of an item should use m_bIsMainUserOfTheItem = false.
	proto external bool IsMainUserOfTheItem();
	proto external void SetIsMainUserOfTheItem(bool isMainUser);
	// ID of the command that should be called, can be found using BindCommand() from the item's animation component.
	proto external int GetCommandID();
	proto external void SetCommandID(int cmdID);
	// The first int argument of the called command.
	proto external int GetCommandIntArg();
	proto external void SetCommandIntArg(int cmdIntArg);
	// The float argument of the called command.
	proto external float GetCommandFloatArg();
	proto external void SetCommandFloatArg(float cmdFloatArg);
	// Maximum length of item use after which we will call m_CmdId with int argument = -1 - this is used to stop looping animations by starting the Out animation. If animation is not supposed to be looping, use larger time frame than expected length of the animation.
	proto external float GetMaxAnimLength();
	proto external void SetMaxAnimLength(float maxAnimLength);
	// Currently sets the BodyPart variable for healing items.
	proto external int GetIntParam();
	proto external void SetIntParam(int intParam);
	// Alignment point to which RHandProp bone on character will be aligned during the animation.
	proto external PointInfo GetAlignmentPoint();
	proto external void SetAlignmentPoint(PointInfo pointWS);
	// Node in the item graph through which it will be bound to the character's graph.
	proto external int GetItemGraphEntryPoint();
	// Only call SetItemGraphEntryPoint after the entity has already been set, so that the graph entry point can be determined.
	proto external void SetItemGraphEntryPoint(string itemGraphEntryPoint);
	// Binding name in the character graph to which the Item Entry Node will be attached.
	proto external string GetCharGraphBindingName();
	proto external void SetCharGraphBindingName(string charGraphBindingName);
	// Clean up the params.
	proto external void Reset();
}

/*!
\}
*/
