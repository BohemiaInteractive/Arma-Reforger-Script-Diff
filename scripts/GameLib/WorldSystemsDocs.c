
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
