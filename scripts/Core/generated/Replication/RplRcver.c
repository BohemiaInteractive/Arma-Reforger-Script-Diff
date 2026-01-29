/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Replication
\{
*/

/*!
Target of the RPC call. RPC delivery depends on multiple factors: replication mode
of caller, ownership and RPC receiver value. An RPC may be dropped, executed directly
on the caller, or routed to remote instances. Which remote instances will RPC be
routed to depends on RplRcver value and evaluation of RplCondition.

Full routing table for RPCs:

| caller mode | caller is owner | receiver  | action                            |
|-------------|-----------------|-----------|-----------------------------------|
| Server      | yes             | Server    | direct call                       |
| ^           | ^               | Owner     | direct call                       |
| ^           | ^               | Broadcast | routed call (to relevant clients) |
| ^           | no              | Server    | direct call                       |
| ^           | ^               | Owner     | routed call (to owner client)     |
| ^           | ^               | Broadcast | routed call (to relevant clients) |
| Client      | yes             | Server    | routed call (to server)           |
| ^           | ^               | Owner     | direct call                       |
| ^           | ^               | Broadcast | direct call                       |
| ^           | no              | Server    | dropped                           |
| ^           | ^               | Owner     | dropped                           |
| ^           | ^               | Broadcast | direct call                       |

Note that evaluating RplCondition can drop an RPC, even if according to routing
table the call would be executed directly or routed.

Simplified version of the above table that may be easier to think about:

Mode = SERVER
|              | Server | Owner  | Broadcast   |
|--------------|--------|--------|-------------|
| Owner        | self   | self   | all clients |
| Not Owner    | self   | client | all clients |

Mode = CLIENT
|              | Server | Owner  | Broadcast   |
|--------------|--------|--------|-------------|
| Owner        | server | self   | self        |
| Not Owner    | drop   | drop   | self        |

When using broadcast, relevant clients are those who have the item through which
broadcast is sent streamed in. A broadcast sent before an item is streamed to
a client will not arrive on given client. When using broadcasts, you should always
consider what will happen in a situation when item is streamed to the client after
the broadcast has been sent.
*/
enum RplRcver
{
	//! Server side.
	Server,
	//! %Instance owner.
	Owner,
	//! Instances with proxy role.
	Broadcast,
}

/*!
\}
*/
