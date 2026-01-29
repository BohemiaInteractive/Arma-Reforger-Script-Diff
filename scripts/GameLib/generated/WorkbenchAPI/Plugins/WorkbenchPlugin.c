/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup WorkbenchAPI_Plugins
\{
*/

class WorkbenchPlugin: Managed
{
	private void WorkbenchPlugin();
	private void ~WorkbenchPlugin();

	event void Run();
	event void RunCommandline();
	event void Configure();
	event void OnResourceContextMenu(notnull array<ResourceName> resources);
}

/*!
\}
*/

#endif // WORKBENCH
