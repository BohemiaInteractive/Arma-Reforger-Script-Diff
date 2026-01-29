/** @ingroup Editable_Entities
*/ 

/*!
State of editable entity (SCR_EditableEntityComponent).

Entities of each state are managed by SCR_EntitiesManagerEditorComponent.

There is a strict order in which entities of various states are nested.
For example, ACTIVE entity is always VISIBLE and UNLOCKED.

States on the same level can overlap, e.g., ACTIVE entity can also be RENDERED.

\dot
graph G {
	node [shape=box, fontname=Helvetica, fontsize=10];
	fontname=Helvetica;
	fontsize=10;

	subgraph clusterA {label = "UNLOCKED"; margin=16;
		subgraph clusterB {label = "VISIBLE";
			PLAYER;
			subgraph clusterC {label = "ACTIVE";
				subgraph clusterD {label = "INTERACTIVE"; style=solid;
					RENDERED;
				}
				SELECTED;
				subgraph clusterE {label = "FOCUSED"; style=solid;
					HOVER;
				}
			}
		}
	}
}
\enddot
*/
enum EEditableEntityState
{
	UNLOCKED				= 1 << 0, ///< Entity keys are matching editor keys
	VISIBLE					= 1 << 1, ///< Entity is marked as visible (SCR_EditableEntityComponent.SetVisible())
	RENDERED				= 1 << 2, ///< Entity is within its rendering distance or is SELECTED.
	ACTIVE					= 1 << 3, ///< Entity is in the current layer or one if its children
	INTERACTIVE				= 1 << 4, ///< Entity is directly in the current layer
	SELECTED				= 1 << 5, ///< Entity is selected
	FOCUSED					= 1 << 6, ///< Entity is focused (as opposed to HOVER, multiple entities can be focused, e.g., when hovering over a composition)
	HOVER					= 1 << 7, ///< Entity is under cursor (only one entity can be hovered on at once)
	PLAYER					= 1 << 8, ///< Entity is a player
	COMPATIBLE_SLOT			= 1 << 9, ///< Entity is a slot compatible with currently transformed/placed entity
	CURRENT_LAYER			= 1 << 10, ///< Current layer entity (only one entity can be current at once)
	PREVIEW					= 1 << 11, ///< Local preview
	EDITED					= 1 << 12, ///< Currently edited entities
	PINGED					= 1 << 13, ///< Entity pinged by players
	EXTENDABLE				= 1 << 14, ///< Entity can be extended by another entity
	DESTROYED				= 1 << 15, ///< Entity is destroyed / dead
	COMMANDED				= 1 << 16, ///< Entities ready to be commanded
	HIGHLIGHTED				= 1 << 17, ///< Entity requires player's attention
	CURRENT_LAYER_CHILDREN	= 1 << 18, ///< Only direct children of current layer
	AUTONOMOUS				= 1 << 19, ///< AI is behaving autonomously and may ignore orders
	BASE_BUILDING			= 1 << 20, ///< Entities part of current base building mode
};