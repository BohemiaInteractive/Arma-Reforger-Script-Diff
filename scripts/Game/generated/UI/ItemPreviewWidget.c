/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class ItemPreviewWidget: RenderTargetWidget
{
	//! For currently rendered item - fetches provided nodeId (-1 for root) with provided offset and returns it's position in widget space
	proto external bool TryGetItemNodePositionInWidgetSpace(TNodeId nodeID, inout vector offsetLS[4], out vector posInWidget);
}

/*!
\}
*/
