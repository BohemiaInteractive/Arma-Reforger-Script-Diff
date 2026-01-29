/**
@defgroup Editor_Entities Entities
Editor entities manage functionality of the editor.

# Structure
For the editor to function propertly, it must be configured in the following structure.
All entities must be saved into a prefabs and linked in attributes mentioned below.
\dot
digraph Editor_Entities
{
	rankdir="LR";
	compound=true;
	fontname="sans-serif bold";
	fontsize=12;
	splines=ortho;
	style=filled;
	node [shape=box, style=filled, fontname="sans-serif", fontsize=10, color=khaki1];
	edge [fontname="sans-serif", fontsize=9, arrowhead="box", color=skyblue1, fontcolor=skyblue4];

	subgraph cluster_Manager
	{
		label = "SCR_EditorManagerEntity";
		URL="\ref SCR_EditorManagerEntity";
		color=skyblue1;

		Manager_Component_1 [label="SCR_BaseEditorComponent 1", URL="\ref SCR_BaseEditorComponent"];
		Manager_Component_2 [label="SCR_BaseEditorComponent 2", URL="\ref SCR_BaseEditorComponent"];
		Manager_Component_n [label="SCR_BaseEditorComponent n", URL="\ref SCR_BaseEditorComponent"];
	}
	subgraph cluster_Mode_n
	{
		label = "SCR_EditorModeEntity n";
		URL="\ref SCR_EditorModeEntity";
		color=skyblue1;

		Mode_n_Component_1 [label="SCR_BaseEditorComponent 1", URL="\ref SCR_BaseEditorComponent"];
		Mode_n_Component_2 [label="SCR_BaseEditorComponent 2", URL="\ref SCR_BaseEditorComponent"];
		Mode_n_Component_n [label="SCR_BaseEditorComponent n", URL="\ref SCR_BaseEditorComponent"];
	}
	subgraph cluster_Mode_1
	{
		label = "SCR_EditorModeEntity 1";
		URL="\ref SCR_EditorModeEntity";
		color=skyblue1;

		Mode_1_Component_1 [label="SCR_BaseEditorComponent 1", URL="\ref SCR_BaseEditorComponent"];
		Mode_1_Component_2 [label="SCR_BaseEditorComponent 2", URL="\ref SCR_BaseEditorComponent"];
		Mode_1_Component_n [label="SCR_BaseEditorComponent n", URL="\ref SCR_BaseEditorComponent"];
	}

	Core [label="SCR_EditorManagerCore\nConfig", URL="\ref SCR_EditorManagerCore", color=gray85];

	Core -> Manager_Component_2 [lhead=cluster_Manager, label="m_EditorManagerPrefab\nAttribute"];
	Manager_Component_2 -> Mode_1_Component_n [ltail=cluster_Manager, lhead=cluster_Mode_1, label="m_ModePrefabs\nAttribute"];
	Manager_Component_2 -> Mode_n_Component_1 [ltail=cluster_Manager, lhead=cluster_Mode_n, label="m_ModePrefabs\nAttribute"];
}
\enddot

# Execution Order
Functions in relevant classes are called in the following order.

\dot
digraph Editor_Entities_Order
{
	compound=true;
	newrank=true;
	fontname="sans-serif";
	fontsize=12;
	splines=ortho;
	style=filled;
	node [shape=box, fontname="sans-serif", height=0.25, fontsize=10, style=filled, fillcolor=white];
	edge [fontname="sans-serif", fontsize=9];

	{
		node [shape=plaintext];
		label_1[label = "Player joins,\ncreate editor manager"];
		label_2[label = "Call custom event\n(used by the game mode\nor other systems)"];
		label_3[label = "Create editor modes\n(either specific ones in the Event,\nor default ones afterwards)"];
		label_4[label = "Set current mode\nand auto-open editor\n(when enabled)"];
	}

	{
		color=gray85;
		subgraph cluster_SCR_BaseGameMode
		{
			label = "SCR_BaseGameMode";
			URL="\ref SCR_BaseGameMode";
	
			Event_OnPlayerConnected;
		}
		subgraph cluster_SCR_EditorManagerCore
		{
			label = "SCR_EditorManagerCore";
			URL="\ref SCR_EditorManagerCore";
	
			CreateEditorManager;
			Event_OnEditorManagerCreatedServer;
		}
		subgraph cluster_SCR_EditorManagerEntity
		{
			label = "SCR_EditorManagerEntity";
			URL="\ref SCR_EditorManagerEntity";
	
			Manager_InitServer[label="InitServer"];
			Manager_InitOwner[label="InitOwner", fillcolor=plum];
			Manager_PostInitServer[label="PostInitServer"];
			CreateEditorMode;
		}
		subgraph cluster_SCR_EditorModeEntity
		{
			label = "SCR_EditorModeEntity";
			URL="\ref SCR_EditorModeEntity";
	
			Mode_InitServer[label="InitServer"];
			Mode_InitOwner[label="InitOwner", fillcolor=plum];
		}
	}

	label_1 -> label_2 -> label_3 -> label_4;

	Event_OnPlayerConnected -> CreateEditorManager;
	CreateEditorManager -> Manager_InitServer;
	Manager_InitServer -> Manager_InitOwner;
	Manager_InitServer -> Event_OnEditorManagerCreatedServer;
	Event_OnEditorManagerCreatedServer -> CreateEditorMode;

	CreateEditorMode -> Mode_InitServer;
	Mode_InitServer -> Mode_InitOwner;
	Mode_InitServer -> Manager_PostInitServer;

	subgraph cluster_Legend
	{
		label = "Legend";
		fillcolor=white;
		color=gray50;

		Legend_Client[label="Client function", fillcolor=plum];
		Legend_Server[label="Server function"];
	}

	{rank=same; label_1; Event_OnPlayerConnected; CreateEditorManager; Manager_InitServer; Manager_InitOwner;}
	{rank=same; label_2; Event_OnEditorManagerCreatedServer;}
	{rank=same; label_3; CreateEditorMode; Mode_InitServer; Mode_InitOwner;}
	{rank=same; label_4; Manager_PostInitServer;}
}
\enddot

@ingroup Editor

@defgroup Editor_Components Editor Components
Editor manager components
@ingroup Editor_Entities

@defgroup Editor_Components_Attributes Attributes
Global editor attributes
@ingroup Editor_Components
*/