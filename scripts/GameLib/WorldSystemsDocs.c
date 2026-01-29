/*!
\defgroup WorldSystems World Systems
\addtogroup WorldSystems
\{
1. \ref Page_WorldSystems
\}
*/

/*!

\page Page_WorldSystems %World Systems

\tableofcontents

\section WorldSystemsDocs_Introduction Introduction
%World systems are means to perform some operations at various stages of world
update loop. They are meant to complement entities and their components. Usually
world system implements some feature using entities and components in the world,
similar to how a puppeteer would use puppets to perform a stage play.

%World systems differ from entities and components in a few ways:
- Only one instance of particular world system type can be created for
  particular world. On the other hand, there can be many instances of single
  entity type in a world, and each entity instance may have possibly multiple
  components of same type on it.
- Entities can be organized into hierarchies and have position and orientation
  in the world. %World systems do not - while they are tied to the world, they
  do not exist on specific location inside of it.
- Entities are updated one after another, and order in which their updates
  happen, is not defined. %World systems can affect their order of updates
  through dependencies between them.
- %World file holds all entities and components which will be created in the
  world when it is loading (though more entities may be created after gameplay
  starts). What world systems will be instantiated is specified using a config
  file that is independent of the world.

To demonstrate how world systems and entities with components would be used to
build gameplay, let's use a simple FPS game where players spawn on spawn points
placed in map. %Game supports both deathmatch and team-deathmatch modes. Spawn
points are represented as entities in the world. They are also assigned to
specific teams either through property on entity itself, or on a component
attached to entity that is dedicated to holding team affiliation information. We
would then implement logic for score counting and player spawning for our two
game modes using world systems. By switching configuration of these systems, we
can run the map in deathmatch or team-deathmatch mode. If we are running in
deathmatch mode, we can use all spawn points when determining where a player
should spawn. If we are running in team-deathmatch mode, our respawn logic will
be also taking into account team affiliation of each spawn point, ensuring
players do not respawn in the middle of enemy territory.

Previous description has been intentionally very simple and vague. Goal was to
give a rough idea of how things fit together without going into too many
details. In a real game, there may be many more systems driving a world (tens to
even hundreds), many of which may be shared across all game modes, while others
would be specific to particular game mode. Mods may also alter behavior of
systems, implement their own and organize them to create new game modes.

\section WorldSystemDocs_CreatingAndUsingWorldSystem Creating and using world system
Let us take a look at how a world system is implemented and integrated in the
game.

There are a few things necessary for every world system to work:
1. Your world system class must be derived from WorldSystem or one of its
   subclasses.
2. You need to override WorldSystem.InitInfo().
3. In your implementation of `InitInfo()` function, you set information about
   your world system so that engine knows how to handle it.

\remarks Arma Reforger previously used a legacy version of world systems. This
document will refer to these as "legacy systems". Differences between current and
legacy systems will be described as they come up. To make transition easier, there
is support for migrating systems gradually from legacy to new version. A world
system will follow legacy rules if it does not override `InitInfo()`. Once `InitInfo()`
is provided, all information will be taken from WorldSystemInfo and legacy sources
will only be used for mismatch warnings. Eventually, this support for legacy sources
will be dropped altogether, so it is best to migrate as soon as possible.
\remarks In legacy version, your world system had to be subclass of `BaseSystem`
instead of WorldSystem.

Following code is an example of very simple world system, with each of the above
points marked.
\snippet this Hello world system

After we recompile and reload scripts in workbench, we are done defining our
world system in script. But we won't be able to see it in game just yet. For
that, we need to run the game with config that references our world system, so
we either have to create new config, or add it to an existing one. In practice,
you will most likely be adding your world system to existing config, but to
demonstrate all steps of the process, we will be creating new config from
scratch.

To create new world system config:
1. In "Resource Browser", select directory where you want to place your config,
   right-click in an empty space and choose "Config file" in "Create Resource"
   section of the context menu.
2. Type name of your config file.
3. Choose "SystemSettings" as config class.

\image{inline} html WorldSystems/create-config-file.png
\image{inline} html WorldSystems/create-config-file-name.png
\image{inline} html WorldSystems/create-config-file-class.png
\image{inline} html WorldSystems/create-config-file-done.png

You should now see your new config file in "Resource Browser". From here on, the
process is the same same as adding to an existing config:
1. Double-click on the config file in "Resource Browser" to open it.
2. Add new entry to "Systems" array using "+" button.
3. Search for your new world system type, then select it by clicking on it.

\image{inline} html WorldSystems/add-world-system-open-config.png
\image{inline} html WorldSystems/add-world-system-search.png
\image{inline} html WorldSystems/add-world-system-done.png

You should now have your world system added to the end of the "Systems" array.
When this config is used to run a world, world systems listed in the "Systems"
array will be created.

Steps for running world with particular config are specific to each game so we
won't go into that here. However, for testing, "World Editor" supports selecting
one of predefined configs to use in "Play Mode". For our config to appear in
this list, we need to add it there first:
1. In main workbench window menu at the top, go to "Workbench", then "Options".
2. In "Game Project" tab, in bottom part scroll down to "Modules" section,
   looking for "System Module Settings".
3. Add your config to the "Configs" array.

\image{inline} html WorldSystems/testing-options.png
\image{inline} html WorldSystems/testing-module-settings.png
\image{inline} html WorldSystems/testing-play-mode-menu.png

If you now open some world in %World Editor, you can go to "Play Mode" button
dropdown menu, "World Systems Config" submenu, and see your config there.
Clicking it will select it for use when entering "Play Mode". With your config
selected, if you now enter play mode, you should see the message from
HelloWorldSystem constructor appear in log. This shows us that our world system
has been created and everything works as expected.

%World systems can be queried at runtime as well. To try it out, you can paste
following code into Remote Console of Script Editor while you are in play mode with
HelloWorldSystem created (note that this script relies on `GetGame()` function,
which might not be available in all projects).
\code
	// Make sure you are using correct world, as there may be multiple worlds in
	// some situations and main game world is not always the one you want.
	World world = GetGame().GetWorld();

	// Once you have the world, you query it for particular system.
	WorldSystem foundSystem = world.FindSystem(HelloWorldSystem);
	Print(foundSystem);
\endcode

\section WorldSystemDocs_Info World system information
Let us now go back to the HelloWorldSystem.InitInfo() function we implemented
in our example. This function has only one purpose: to fill WorldSystemInfo
instance with information about our system. Each world system has some
meta-information associated with it, which is stored in WorldSystemInfo. Engine
uses this information for various purposes: ordering of updates, dependencies
between world systems, determining whether instance of world system should be
created in some situation, and so on. We will be going over various
WorldSystemInfo properties in related sections.

Main reason for this approach is to allow derived classes and mods to inherit as
much of the information from the original system, making changes to only those
properties they need to. Here is an example of how someone might decide to mod
our HelloWorldSystem in order to introduce per-frame updates to it:
\snippet this Hello world system - modded

Important bits are:
1. Calling InitInfo() implementation of base class (or class being modded),
   letting it do its thing.
2. Making changes necessary for our implementation.

Default values of WorldSystemInfo properties are well defined, but you might not
always be able to rely on them. Specifically, all world systems implemented in
native code get to modify these default values in WorldSystemInfo before
`InitInfo()` is called. The only system that guarantees to leave default values
unchanged is WorldSystem. If it is necessary to ensure that WorldSystemInfo has
everything set to default, one can use WorldSystemInfo.RestoreDefaults() to
achieve it.

\warning Using WorldSystemInfo.RestoreDefaults() should be considered last
resort. In almost all cases, it is better to use more fine-grained operations,
otherwise you risk maintenance issues further down the line.

With the exception of WorldSystemInfo.RestoreDefaults(), all WorldSystemInfo
functions follow same naming convention, which puts together operation and
property on which this operation happens. Based on operations available, there
are two kinds of properties:
1. Single-value properties, which only provide `Set*()` operation.
	- WorldSystemInfo.SetAbstract()
2. Set-like properties, which provide `Add*()`, `Remove*()` and `Clear*()`
   operations.
	- WorldSystemInfo.AddPoint()
	- WorldSystemInfo.RemovePoint()
	- WorldSystemInfo.ClearPoints()

\warning While `InitInfo()` function allows one to write anything they want, in
most cases the implementation should be very simple. For this reason, there is
no support for querying property values. Highly suspicious things include (but
are not limited to):
- any kind of control flow (branches, loops)
- function calls other than `super.InitInfo()` or WorldSystemInfo member
  functions
- side-effects other than changes in WorldSystemInfo
- reliance on any kind of state of classes or variables (including global or
  static variables)

\section WorldSystemDocs_SystemPoints World system points
Every world system can register for notifications at particular points during world
lifetime. We have already seen how this is done in previous example, so let's look
at it again:
\snippet{trimleft} this Hello world system - modded InitInfo

\remarks Legacy system specifies system points using config property "System Points"
which can be modified in world system config.

When we initialize WorldSystemInfo, we call WorldSystemInfo.AddPoint(), passing
WorldSystemPoint.Frame as argument (marked by `2.` in the example). This tells
engine that our world system wants to be notified at this system point.

\remarks WorldSystemPoint enum was previously called `ESystemPoint`. Old name is
still provided as an alias, but it is deprecated and will be removed in the future.

There are several system points that world systems can register to. For a full
list you can refer to WorldSystemPoint. In short, there are two kinds of system
points: update points and one-time points. Update points occur during world update
loop periodically. On the other hand, one-time points occur only once during lifetime
of particular world and its associated world systems. Many of them are related to
entity events (see EntityEvent) and thus follow similar rules. For example, every
iteration of world update loop is guaranteed to hit WorldSystemPoint.Frame (same as
EntityEvent.FRAME) exactly once. On the other hand, WorldSystemPoint.FixedFrame
(same as EntityEvent.FIXEDFRAME) may be hit zero or more times, depending on how
much time one iteration of world update loop took and what is the fixed timestep
logic used by particular game.

If a world system is registered for notification on at least one system point of
particular kind, corresponding notification function will be called whenever that
point is hit. For update points, notification function is `OnUpdatePoint()`, while
for one-time points it is `OnOneTimePoint()`.

\remarks Legacy version only supports update points, for which it calls notification function `OnUpdate()`.

It is also possibly to affect the relative ordering of world systems that
registered to same system point. To do so, WorldSystemInfo has `ExecuteBefore`
and `ExecuteAfter` properties that can be used to specify ordering constraints.

\warning Unless two systems have execution constraint specified between them
(either through `ExecuteBefore` on first, or through `ExecuteAfter` on second),
their ordering is undefined. In practice, these two systems without execution
constraint will execute in some order, and that order may appear to stay the
same for some time, but there is no guarantee that it will stay the same
forever.

Here is an example of a system that uses multiple system points and specifies
execution constraint on our HelloWorldSystem from previous examples:
\snippet this Multi-point system

\remarks Legacy systems can provide virtual function
```
bool DependsOn(WorldSystemPoint point, WorldSystem system)
```
to describe dependencies. This is similar to `ExecuteAfter` in that when this
function returns `true` for particular combination of point and system,
it will be as if that combination of point and system was added to `ExecuteAfter`
set. There is no `ExecuteBefore` equivalent available for legacy systems.

This system registers to three system points (`1.`). It also adds one execution
constraint using WorldSystemInfo.AddExecuteBefore() (`2.`). This constraint says
that in WorldSystemPoint.Frame, MultiPointSystem.OnUpdatePoint() must execute before
HelloWorldSystem.OnUpdatePoint().

Rest of the code demonstrates different behavior of various system points. It
counts how many times was particular system point hit and prints messages with
system point and its hit index. After enough hits have been printed, system
disables itself, preventing further calls to `OnUpdatePoint()`.

Now add MultiPointSystem to our previously created config that already contains
HelloWorldSystem and run a world using this config. Based on order in which log
messages from each system appear, you can see that HelloWorldSystem and
MultiPointSystem execute WorldSystemPoint.Frame in order we specified. You can try
to change constraint from `ExecuteBefore` to `ExecuteAfter` to see messages
appear in the opposite order. Or you can remove execution constraint altogether
and see in what order they execute afterwards (though don't rely on it to stay
the same).

\note Instead of `ExecuteBefore` on MultiPointSystem, we could have set
`ExecuteAfter` on HelloWorldSystem which would have the same effect (of course,
changing what system is passed to it as well). In most cases it doesn't matter
where constraint is specified, but good practice is to prefer specifying it in
system that you are maintaining, instead of adding or modding it into someone
else' system.

Very likely you will also notice that number of hits at the end is different for
each system point we registered. This is because of different system points
having different relation to world update loop. WorldSystemPoint.SimulatePhysics
might not appear at all if there are no objects that require simulation of
physics. However, order in which these system points are hit (assuming they are
hit at all) is guaranteed. Describing world update loop in detail is outside
the scope of world systems, though.

\section WorldSystemDocs_SystemConfiguration World system configuration
%World systems can declare variables that will be exposed in world system
config. This is done by decorating variable declarations with Attribute. These
variables will then be initialized by engine and world system can use them at
runtime.

Here is an example of exposing variables in world system config:
\snippet this Configurable system

1. We declare member variable `m_ConfigInt` which will be initialized from
   config.
2. We decorate it using Attribute to let engine know that `m_ConfigInt` should
   be exposed through config, as well as how to expose it (in this case, what
   should be its default value).
3. We print value of `m_ConfigInt` we obtained from config.

Running world with this world system will produce a message in log, showing that
value of `m_ConfigInt` will match whatever you set in the config (2 by default).

\note In general, a world system can also change its behavior based on entities
present in the world. Whether some configuration option should come from world
system config, or from an entity in the world, is sometimes not clear. Ideally,
configuration in world system config should be independent from worlds
themselves so that one config can be used with different worlds. Things which
are specific to particular world should be stored in that world. However, there
are also ambiguous situations which are hard to categorize. In those cases, one
should consider pros and cons of either approach and decide accordingly.
Sometimes, there just isn't a simple answer.

\section WorldSystemDocs_ClassHierarchySupport Support for class hierarchies
Let's say we want to model multiple game modes by having one common base class
for them (derived from WorldSystem) which defines common interface of all game
modes. For two game modes (using deathmatch and team-deathmatch example from
\ref WorldSystemsDocs_Introduction) we would like to have following inheritance
relationship:

\dot
digraph GameModes {
	rankdir="BT";
	WorldSystem;
	GameMode -> WorldSystem;
	DeathmatchMode -> GameMode;
	TeamDeathmatchMode -> GameMode;
}
\enddot

We would also like to enforce a few rules:
1. Only one game mode can be created at a time.
2. It shouldn't be possible to create GameMode itself as it is just defining
   interface without implementation, only full implementations can be created.
3. Others can search for GameMode type using World.FindSystem() and obtain
   created game mode.

Enforcing rule 2 is the easiest and we have already seen it before in previous
examples: `Abstract` property of WorldSystemInfo. Basically, abstract systems
cannot be instantiated and they will not appear as option when editing world
system config. All world systems derived from WorldSystem are abstract by
default. In all of our previous examples, we were marking world systems as not
abstract so we could add them to world system config. This time, we will do the
opposite and mark GameMode as abstract.

\remarks Legacy systems implemented in script are not abstract and they can
always be instantiated. It is not possible to change this.

Rules 1 and 3 may seem unrelated at first, but we can solve them together. If
you look at World.FindSystem() function, you will notice that it can return at
most one world system instance for particular type. Behavior of
`FindSystem(type)` can be summarized as follows: If `type` is *unique* (or
derived from unique type), then it returns the one instance that can be safely
cast to this type (or null if there is no such world system instance). For all
other types, it only returns instances that match the type exactly, ignoring
inheritance relationships. This brings us to `Unique` property of
WorldSystemInfo. When a world system type is marked as unique using this
property, only one world system that is derived from this unique type can be
created at runtime. In other words, if we make GameMode type unique, we will
enforce that only one implementation of game mode will be created at runtime and
also make World.FindSystem() allow searches for GameMode type to return this
game mode implementation.

Here is what it would all look like in the code.
\snippet this Class hierarchy example

1. We set GameMode WorldSystemInfo properties `Abstract` and `Unique` to `true`,
   preventing creation of GameMode itself, and also requiring that at most one
   type derived from GameMode can be created. Actual game mode implementations
   have `Abstract` set to `false`.
2. We use `OnInit()` to see which game mode is created by printing messages
   specific to each game mode.
3. We also declare a GameMode.Notify() function that can be invoked by rest of
   the game without knowing which game mode is actually running.

If you now run world with either DeathmatchMode or TeamDeathmatchMode, you will
see corresponding message from `OnInit()` in log. You can also test in Remote
Console of Script Editor that `Notify()` can be called without knowing which
game mode exactly is created:
\code
	World world = GetGame().GetWorld();
	GameMode gameMode = GameMode.Cast(world.FindSystem(GameMode));
	gameMode.Notify();
\endcode

\remarks Legacy systems are marked as *unique* automatically based on their base
class. If their direct base class is either WorldSystem or `GameSystem`, they are
marked as unique. It is not possible to change this.

\section WorldSystemDocs_Replication Replication of world systems
\note This section requires some understanding of replication and
replication-related terminology. If you can't understand something, feel free to
ignore it. Things will make more sense once you get familiar with replication as
well.

All world systems support replication and can work in multiplayer. It is up to
author of the system to decide whether that is necessary or not. In some cases,
a world system naturally only exists on client (eg. sound system) or only on
server (eg. hit registration that does lag compensation). In other cases,
a world system needs to be replicated between server and clients.

For a world system to be replicated, it must be registered in replication. This
is done automatically when all following conditions are met:
1. It must exist on both server and clients (we'll get to that in
   \ref WorldSystemDocs_SystemLocation section).
2. It must have some replicated state, RPC, or replication callback. In other
   words, its replication layout must not be empty.

All replicated world systems are owned by server. This means that replicated
world systems can broadcast information to all clients, but they cannot
communicate with specific client (neither sending, nor receiving). We'll talk
about communication between specific client and server in section
\ref WorldSystemDocs_WorldController.

\subsection WorldSystemDocs_SystemLocation World system location
To limit world system creation to either server or client, we use `Location`
property of WorldSystemInfo. WorldSystemInfo.SetLocation() takes
WorldSystemLocation, which has three possible values: `Both`, `Client` and
`Server`. However, game can be running in various multiplayer modes, and mapping
might not be obvious. Following table summarizes which multiplayer modes match
which locations (yes - system is created, no - system is not created):

| Multiplayer mode             | Both | Client | Server |
|------------------------------|------|--------|--------|
| Single-player                | yes  | yes    | yes    |
| Multiplayer client           | yes  | yes    | no     |
| Multiplayer listen server    | yes  | yes    | yes    |
| Multiplayer dedicated server | yes  | no     | yes    |

This table may look strange at first. After all, why is mode with "server" in
its name sometimes creating systems with WorldSystemLocation.Client? You might want
to think of WorldSystemLocation.Client as "place where player interacts with the
game" (providing inputs, getting audio-visual output). On the other hand,
WorldSystemLocation.Server is "place where full game simulation happens" (ie. the
only place that knows about everything). In that sense, because there is no way
to interact with dedicated server, it makes no sense for world system that
should only appear on WorldSystemLocation.Client to be there. On the other hand,
a multiplayer client knows only what server told it and doesn't have full
picture required for running simulation, so world system that only appears on
WorldSystemLocation.Server wouldn't be able to function there. Single-player and
listen server (ie. player-hosted server) are basically the same in this regard -
they allow player interaction and also know about everything. In fact,
single-player is just listen server that doesn't allow clients to connect.

\remarks Legacy system specifies location using config property "System Location"
which can be modified in world system config.
\remarks WorldSystemLocation enum was previous called `ESystemLocation`. Old name
is still provided as an alias, but it is deprecated and will be removed in the
futured.

\subsection WorldSystemDocs_WorldController World controllers
As mentioned before, world systems are not able to communicate with specific
clients. To achieve that, each world system can specify world controllers that
should be created along with this system for each player.

\remarks Legacy world systems do not support world controllers.

While owner of replicated world system is always server, owner of world
controller is always one of the players. This allows owner player to communicate
with server through the world controller and it also allows server to send
information only to particular player via their controller.

Let's now take a look at an example of a world controller.

\snippet this World controller example

1. We declare our world system called PlayerNameInputSystem, similar to other
   world systems we've seen before. We also declare PlayerNameInputController
   which is derived from WorldController. This is how world controllers are
   declared.
2. We set up WorldSystemInfo. There are two interesting bits here:
	1. We set `Location` to WorldSystemLocation.Client. This system just provides
	   means for player to provide us with some input (player name), so we don't
	   need it on server.
	2. We add PlayerNameInputController to WorldSystemInfo property
	   `Controllers`. This tells the engine which controllers to instantiate for
	   a particular system.
3. We set up WorldControllerInfo for our controller. This is similar concept as
   WorldSystemInfo - it gives the engine information about particular
   controller.
4. When "Apply" button is pressed, we look up PlayerNameInputController instance
   belonging to player controlling the game and we request change of name
   through this controller. This request is sent to server where it can be
   handled.

%Step 3. brings up an aspect of controller which wasn't mentioned yet. We said
that each controller is owned by particular player. As such, controllers are
always streamed to their owners and so each client owning a controller has
a proxy instance of it which it can search for (eg. using
WorldController.FindMyController()). However, what about controllers belonging
to other players? This is what `Public` property on WorldControllerInfo affects.
A public controller is one that appears on all clients, not just its owner. A
private controller (ie. one that is not public) will only be present on client
who owns it. However, server knows about all controllers (both public and
private) and it doesn't matter if it is listen server (which owns one of these
controllers) or dedicated server (which doesn't own any controller). So in our
example, we take advantage of the fact that controller is public and we use that
to communicate name of particular player to all other players.

You might also notice how in step 4. we search for controller using
WorldController.FindMyController() function. This is similar to
World.FindSystem() which we've already seen before. To make inheritance work
with this kind of API, WorldControllerInfo also has `Unique` property, similar
to WorldSystemInfo. Behavior of unique controllers is similar to that of unique
systems - only one world controller derived from unique world controller type
can be instantiated.

It may be tempting to use controller constructor to react to controller creation
(as a "player connected" event of sorts), but you should keep in mind that not
everything might be fully set up at the time of constructor invocation. For
this purpose, there are other functions on controller which may be better suited
for this:

\snippet this World controller example - modding

WorldController.OnAuthorityReady() is part of world controller framework and it
is always invoked once authority instance fully prepared. On the other hand,
`RplGiven()` is more general replication callback which is related to ownership
transfer in replication, but in this case it can be used as signal that proxy
instance on owning client is fully prepared.

You might have been wondering why PlayerNameInputController is created both on
client and server, even though PlayerNameInputSystem has its `Location` set to
WorldSystemLocation.Client. This is is because world controllers exist purely to
allow client-server communication within replication infrastructure. It makes no
sense to have client-only or server-only controller. Therefore, when gathering
set of world controllers to create based on world systems in config, all world
systems which are created either on client or on server have their controller
created as well.

*/

#ifdef DOXYGEN

//! [Hello world system]
class HelloWorldSystem : WorldSystem // 1.
{
	override static void InitInfo(WorldSystemInfo outInfo) // 2.
	{
		outInfo.SetAbstract(false); // 3.
	}

	void HelloWorldSystem()
	{
		Print("Hello world (systems)");
	}
}
//! [Hello world system]

//! [Hello world system - modded]
modded class HelloWorldSystem
{
	//! [Hello world system - modded InitInfo]
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo); // 1.
		outInfo.AddPoint(WorldSystemPoint.Frame); // 2.
	}
	//! [Hello world system - modded InitInfo]

	private int m_FrameCount = 0;

	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (args.GetPoint() == WorldSystemPoint.Frame)
		{
			int frameIndex = m_FrameCount;
			PrintFormat("Hello world (systems) frame %1", frameIndex);

			m_FrameCount += 1;

			// Stop when we've done 10 frame updates (printed frame numbers from 0 to 9).
			if (m_FrameCount == 10)
				this.Enable(false);
		}
	}
}
//! [Hello world system - modded]

//! [Multi-point system]
class MultiPointSystem : WorldSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			// 1.
			.AddPoint(WorldSystemPoint.Frame)
			.AddPoint(WorldSystemPoint.FixedFrame)
			.AddPoint(WorldSystemPoint.SimulatePhysics)
			// 2.
			.AddExecuteBefore(HelloWorldSystem, WorldSystemPoint.Frame);
	}

	private int m_FrameCount = 0;
	private int m_FixedFrameCount = 0;
	private int m_SimulatePhysicsCount = 0;

	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		switch (args.GetPoint())
		{
		case WorldSystemPoint.Frame:
			PrintFormat("MultiPointSystem Frame %1", m_FrameCount);
			m_FrameCount += 1;
			break;
		case WorldSystemPoint.FixedFrame:
			PrintFormat("MultiPointSystem FixedFrame %1", m_FixedFrameCount);
			m_FixedFrameCount += 1;
			break;
		case WorldSystemPoint.SimulatePhysics:
			PrintFormat("MultiPointSystem SimulatePhysics %1", m_SimulatePhysicsCount);
			m_SimulatePhysicsCount += 1;
			break;
		}

		int total = m_FrameCount + m_FixedFrameCount + m_SimulatePhysicsCount;
		if (total >= 100)
			this.Enable(false);
	}
}
//! [Multi-point system]

//! [Configurable system]
class ConfigurableSystem : WorldSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false);
	}

	[Attribute(defvalue: "2")] // 2.
	private int m_ConfigInt; // 1.

	void ConfigurableSystem()
	{
		PrintFormat("ConfigurableSystem.ctor m_ConfigInt = %1", m_ConfigInt); // 3.
	}
}
//! [Configurable system]

//! [Class hierarchy example]
class GameMode : WorldSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		// 1.
		outInfo
			.SetAbstract(true)
			.SetUnique(true);
	}

	// 2.
	override protected void OnInit()
	{
		Print("GameMode.OnInit()");
	}

	// 3.
	void Notify();
}

class DeathmatchMode : GameMode
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		// 1.
		outInfo.SetAbstract(false);
	}

	// 2.
	override protected void OnInit()
	{
		Print("DeathmatchMode.OnInit()");
	}

	// 3.
	override void Notify()
	{
		Print("DeathmatchMode.Notify()");
	}
}

class TeamDeathmatchMode : GameMode
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		// 1.
		outInfo.SetAbstract(false);
	}

	// 2.
	override protected void OnInit()
	{
		Print("TeamDeathmatchMode.OnInit()");
	}

	// 3.
	override void Notify()
	{
		Print("TeamDeathmatchMode.Notify()");
	}
}
//! [Class hierarchy example]


//! [World controller example]
class PlayerNameInputSystem : WorldSystem // 1.
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		// 2.
		outInfo
			.SetAbstract(false)
			.SetLocation(WorldSystemLocation.Client)
			.AddPoint(WorldSystemPoint.Frame)
			.AddController(PlayerNameInputController);
	}

	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		string playerName;
		bool apply = false;

		// Display DbgUI window which allows player to change their name.
		DbgUI.Begin("Player name input");
		DbgUI.InputText("name", playerName);
		apply = DbgUI.Button("Apply");
		DbgUI.End();

		if (apply)
		{
			// 4.
			auto controller = PlayerNameInputController.Cast(
				GetSystems().FindMyController(PlayerNameInputController)
			);
			controller.RequestNameChange(playerName);
		}
	}
}

class PlayerNameInputController : WorldController // 1.
{
	override static void InitInfo(WorldControllerInfo outInfo)
	{
		// 3.
		outInfo.SetPublic(true);
	}

	[RplProp(onRplName: "OnPlayerNameChanged")]
	string m_PlayerName;

	void RequestNameChange(string newPlayerName)
	{
		Rpc(Rpc_NameChange_S, newPlayerName);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_NameChange_S(string newPlayerName)
	{
		if (m_PlayerName == newPlayerName)
			return;

		m_PlayerName = newPlayerName;
		Replication.BumpMe();
		// We invoke callback explicitly on server. On clients, it will be invoked
		// automatically when replication changes m_PlayerName value.
		this.OnPlayerNameChanged();
	}

	private void OnPlayerNameChanged()
	{
		PlayerId ownerPlayerId = this.GetOwnerPlayerId();
		// This message will appear on server and all clients.
		PrintFormat("Player %1 name '%2'", ownerPlayerId, m_PlayerName);
	}
}

//! [World controller example]

//! [World controller example - modding]
modded class PlayerNameInputController
{
	override protected void OnAuthorityReady()
	{
		Print("PlayerNameInputController.OnAuthorityReady()");
	}

	private bool RplGiven(ScriptBitReader reader)
	{
		Print("PlayerNameInputController.RplGiven()");
		return true;
	}
}
//! [World controller example - modding]


#endif
