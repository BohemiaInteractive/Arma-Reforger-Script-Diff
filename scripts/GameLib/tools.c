/**
 \brief ScriptCallQueue Class provide "lazy" calls - when we don't want to execute function immediately but later during frame update (used mainly in UI)
 \n usage:
 @code
	class MyGame extends Game
	{
		ref ScriptCallQueue m_CallQueue = new ScriptCallQueue();
	
		ScriptCallQueue GetCallqueue() { 
			return m_CallQueue;
		}
	
		override void OnUpdate(float timeslice)
		{
			m_CallQueue.Tick(timeslice);
			...
		}
		...
	}

	class MyObject
	{
		int m_cnt = 0;
	
		void Hello(int p1, string p2)
		{
			Print("Hello( " + p1 + " , " + p2 + ")");
		}
	
		void Test()
		{
			Print(m_cnt);
			m_cnt++;
			
			if (m_cnt > 10)
			{
				ScriptCallQueue queue = g_Game.GetCallqueue();
				queue.Remove(Test);
			}
		}
	}

	void Test(MyObject obj)
	{
		ScriptCallQueue queue = g_Game.GetCallqueue(); 
		queue.CallLater(obj.Hello, 5000, false, 65, "world"); // adds call 'obj.Hello(65, "world")' into queue, and it will be executed once after 5s
		queue.CallLater(obj.Test, 3000, true); // adds call 'obj.Test()' into queue, and it will be executed each 3s
		queue.Call(obj.Hello, 72, "world 2"); // adds call 'obj.Hello(72, "world 2")' into queue, and it will be executed next frame (on next call of ScriptCallQueue.Tick)
	}
 @endcode
 */
class ScriptCallQueue
{
	//! executes calls on queue if their time is already elapsed, if 'repeat = false' call is removed from queue
	proto native void Tick(float timeslice);
	//! adds call into the queue with given parameters and arguments (arguments are holded in memory until the call is executed/removed or ScriptCallQueue is destroyed)
	proto void Call(func fn, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	//! adds call into the queue with given parameters and arguments (arguments are holded in memory until the call is executed/removed or ScriptCallQueue is destroyed)
	proto void CallByName(Managed obj, string fnName , void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	//! adds call into the queue with given parameters and arguments (arguments are holded in memory until the call is executed/removed or ScriptCallQueue is destroyed)
	proto void CallLater(func fn, int delay = 0, bool repeat = false, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);	
	//! adds call into the queue with given parameters and arguments (arguments are holded in memory until the call is executed/removed or ScriptCallQueue is destroyed)
	proto void CallLaterByName(Managed obj, string fnName, int delay = 0, bool repeat = false, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);	
	//! remove specific call from queue	
	proto void Remove(func fn);
	//! return Remaining time to the call execution (in miliseconds)
	proto int GetRemainingTime(func fn);
	//! remove specific call from queue	
	proto void RemoveByName(Managed obj, string fnName);
	//! return Remaining time to the call execution (in miliseconds)
	proto int GetRemainingTimeByName(Managed obj, string fnName);
	//! remove all calls from queue
	proto native void Clear();
	//! dump all callbacks into log
	proto native void Dump();
}

/**
 \brief ScriptInvoker Class provide list of callbacks
 \n usage:
 @code
	class Player
	{
		ref ScriptInvoker m_DeathInvoker	= new ScriptInvoker();
	
		void OnKilled()
		{
			m_DeathInvoker.Invoke(this);		
		}
	}

	void LogPlayerDeath(p)
	{
		Print("RIP " + p);
	}		

	class Game
	{
		void RemovePlayer(Player p)
		{
		}

		void GameOver()
		{
		}
	}

	void OnPlayerSpaned(Player p)
	{
		Game game = g_Game;
		p.m_DeathInvoker.Insert(LogPlayerDeath);
		p.m_DeathInvoker.Insert(game.RemovePlayer);
		p.m_DeathInvoker.Insert(game.GameOver);
	}
 @endcode
 */
class ScriptInvokerBase<Class T>: Managed
{
	//! invoke call on all inserted methods with given arguments
	proto void Invoke(void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	//! insert method to list	
	proto void Insert(T fn);	
	//! remove specific call from list
	proto void Remove(T fn);
	//! remove all calls from list
	proto native void Clear();
	//! dump all callbacks into log
	proto native void Dump();
}

typedef ScriptInvokerBase<func> ScriptInvoker;

class AutotestBase
{
    static proto native void UpdateLastProcessingTime(float seconds);
	static proto native bool IsLastProcessingTime_Signal();
	static proto native void ResetLastProcessingTime_Signal();
	static proto native void SetLastProcessingTime_Treshold(float seconds);
}