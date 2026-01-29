/**
@defgroup Editor Editor
\image html editor_logo.png

Editor is a tool for run-time world observation and manipulation.

It's designed to provide a platform for many specialized features.

## Elements
### Managers
- It's created for each player by **editor core** (SCR_EditorManagerCore).
- Locally it's controlled by **editor manager** (SCR_EditorManagerEntity).
- Various **modes** handle specialized functionality (SCR_EditorModeEntity).
- **Editor components** are responsible for individual features (SCR_BaseEditorComponent).

### Editable Entities
- Entities like vehicles or props can be registered for use in the editor.
- **Core of editable entities** (SCR_EditableEntityCore) holds a list of all entities.
- Individual entities are identified by a **component** (SCR_EditableEntityComponent) or its inherited versions.

## Technical Implementation
- A lot of communication is done through script invokers.
 + e.g., editor components listen to events on editor mode or manager entity, instead of the entities iterating through their components and calling functions there.
 + Outside systems can hook into these systems too, as they already exist for editor purposes.
 + Disadvantage is that from where an event is invoked, it's not clear who everyone is listening.
- **Awareness** between various parts of editor is **always top-down**, never the other way around. Exmaples:
 + Editor mode is aware of the editor manager it belongs to and can access its functions, but the editor manager does not really touch the editor mode (apart from initialization).
 + Editor component knows what editor mode is attached to, but the mode does not care what components it carries.
 + Widget component on editor GUI reads and writes to editor component, but the component is unaware of the GUI.
- GUI functionality is **strictly separated** from system functionality.
 + e.g., SCR_ToolbarActionsEditorComponent holds the list of toolbar actions, but does not control GUI; instead, it provides API to which SCR_ActionsToolbarEditorUIComponent connects.
*/