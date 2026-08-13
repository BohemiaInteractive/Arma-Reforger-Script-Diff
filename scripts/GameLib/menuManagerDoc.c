/*!
\defgroup Menu Menu System
\addtogroup Menu
\{
\see \ref Page_Menu
\}
*/

/*!
\page Page_Menu Menu System

\tableofcontents

\section MenuSystem_Description Description
MenuManager handles creation, opening, closing and updating of game menus and dialogs. It connects UI layouts (the visual representation), script classes (the functionality) and action contexts (for input handling) into a single system. It provides a stack-based system where menus can overlay each other and dialogs are prioritized.

Each menu is an instance of \ref MenuBase or its subclass. A menu preset connects three things together: the UI layout that defines how the menu looks, the script class that implements its behavior, and the action context that handles its input.

\dot
digraph MenuSystem {
	rankdir=LR;
	node [shape=box, style=filled, fillcolor="#E8E8E8", fontname="sans-serif", fontsize=10];
	edge [fontname="sans-serif", fontsize=9];

	subgraph cluster_script {
		label="Script"; labeljust=l; style=dashed; fontname="sans-serif"; fontsize=11;
		"Script Class";
		Member;
	}

	subgraph cluster_menu {
		label="Menu"; labeljust=l; style=dashed; fontname="sans-serif"; fontsize=11;
		MenuBase [fillcolor="#B0C4DE"];
		MenuItem [fillcolor="#B0C4DE"];
	}

	subgraph cluster_ui {
		label="UI"; labeljust=l; style=dashed; fontname="sans-serif"; fontsize=11;
		"UI Layout";
		Widget;
	}

	subgraph cluster_input {
		label="Input"; labeljust=l; style=dashed; fontname="sans-serif"; fontsize=11;
		ActionContext;
		InputAction;
	}

	MenuBase -> MenuItem [arrowhead=odiamond, label="*"];
	MenuBase -> "UI Layout" [style=dashed, label="Layout"];
	MenuBase -> "Script Class" [style=dashed, label="Class"];
	MenuBase -> ActionContext [style=dashed, label="ActionContext"];
	MenuItem -> Widget [dir=both, style=dashed, label="WidgetName"];
	MenuItem -> InputAction [dir=both, style=dashed, label="Action"];
	MenuItem -> Member [dir=both, style=dashed, label="MenuBindAttribute"];
	Widget -> "UI Layout" [style=dashed, arrowhead=none, label="from"];
	InputAction -> ActionContext [style=dashed, arrowhead=none, label="from"];
	Member -> "Script Class" [style=dashed, arrowhead=none, label="from"];
}
\enddot

Key classes:
- \ref MenuManager - manages the lifecycle of all menus and dialogs.
- \ref MenuBase - base class for all menus, provides widget access, callbacks and menu item handling.

\section MenuSystem_Configuration Configuration
The list of available menus is described in a menu configuration file which is referenced in the project settings under **MenuManagerSettings** section.

Each MenuPreset has the following properties:
- **%Layout** - menu UI layout resource path.
- **%Class** - script class implementing the menu.
- **%ActionContext** - **\ref InputSystem_ActionContexts "Context"** name active while the menu is open.
- **%MenuItems** - array of MenuItem objects binding widgets to actions.
- **%PreloadCount** - number of pre-loaded instances. Pre-loaded menus are created in advance to avoid runtime allocation.
- **%Persistent** - when enabled, the menu is not destroyed after close and can be reused.

Example configuration:
\verbatim
MenuManager {
 MenuPresets {
  MenuPreset MAIN_MENU {
   Layout "UI/Layouts/Menus/MyMenu.layout"
   Class "MyMenu"
   MenuItems {
    MenuItem confirm {
     WidgetName "ConfirmButton"
     Action "MenuSelect"
    }
    MenuItem cancel {
     WidgetName "CancelButton"
     Action "MenuBack"
    }
   }
  }
 }
}
\endverbatim

\section MenuSystem_Presets Menu Presets
Menus are identified by the **ScriptMenuPresetEnum** enum. The base enum is empty and is meant to be inherited and extended by each game project.

\remark Enum entry names must match the preset identifier names in the configuration file, as they are linked by name. For example, **%MyGameMenus.MAIN_MENU** corresponds to the **MenuPreset MAIN_MENU** in the configuration above.

To register a custom enum, override the \ref Game.GetMenuPreset method in your game class and return the inherited variant.

Example:
\snippet this Menu System Presets Example

\section MenuSystem_Lifecycle Menu Lifecycle
MenuBase provides a set of callbacks that are invoked in a specific order during the lifetime of a menu.

Opening sequence:
-# OnMenuInit - called during MenuManager config load.
-# OnMenuOpen - called when the menu is being opened.
-# OnMenuShow - called when the menu becomes visible.
-# OnMenuFocusGained - called when the menu gains focus.
-# OnMenuOpened - called after the menu is fully opened and ready.

While open:
- OnMenuUpdate - called each frame while the menu is open.
- OnMenuItem - called on menu item interaction.

Closing sequence:
-# OnMenuFocusLost - called when the menu loses focus or another menu overlaps it.
-# OnMenuHide - called when the menu becomes hidden.
-# OnMenuClose - called when the menu is closed.

\section MenuSystem_Dialogs Dialogs
Dialogs are special menus opened via \ref MenuManager.OpenDialog. They are stacked by priority using DialogPriority:
- INFORMATIVE - low priority informational dialogs.
- WARNING - warning dialogs.
- CRITICAL - critical dialogs that demand immediate attention.

Dialog results are represented by DialogResult.

\section MenuSystem_MenuItems Menu Items
Menu items connect widgets to callbacks and input actions. They can be configured in the menu preset or added at runtime using \ref MenuBase.AddMenuItem.

Use \ref MenuBase.BindItem to register a callback for a menu item. The \ref MenuBindAttribute attribute can be used to bind a method to a menu item declaratively.

Example:
\snippet this Menu System Basic Example
*/

#ifdef DOXYGEN

//! [Menu System Presets Example]
enum MyGameMenus : ScriptMenuPresetEnum
{
	MAIN_MENU,
}

class MyGame : Game
{
	override typename GetMenuPreset()
	{
		return MyGameMenus;
	}
}
//! [Menu System Presets Example]

//! [Menu System Basic Example]
class MyMenu: MenuBase
{
	[MenuBindAttribute("confirm")]
	void OnConfirm()
	{
		Print("Confirmed!");
		Close();
	}

	[MenuBindAttribute("cancel")]
	void OnCancel()
	{
		Close();
	}

	override void OnMenuOpen()
	{
		SetLabel("title", "My Menu");
	}
}

void OpenMyMenu()
{
	MenuManager manager = g_Game.GetMenuManager();
	MenuBase menu = manager.OpenMenu(MyGameMenus.MAIN_MENU);
}
//! [Menu System Basic Example]

#endif
