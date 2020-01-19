using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Collections.Specialized;
using System.Collections;

namespace CSharpExample
{
    class SampleGeneric
    {
        struct struct_ListItem
        {
            public int item1_;
            public int item2_;
            public string name;

            public struct_ListItem(int item1, int item2)
            {
                item1_ = item1;
                item2_ = item2;
                name = "";
            }
        }

        public void TestList()
        {
            Console.WriteLine("TestList Begin");
            //IList,ICollection,IEnumerable
            List<int> list = new List<int>(16) { 56, 40, 19 };
            list.Capacity = 32;

            list.Add(1024);
            list.AddRange(new int[] { 666,777,888 });

            list.RemoveAt(1);
            list.Remove(777);
            //list.RemoveRange(0,6);

            list.ForEach(x=>Console.WriteLine(x.ToString()));
            Console.WriteLine("\n");

            List<struct_ListItem> listItems = new List<struct_ListItem>();
            for (int i = 0; i < list.Count; ++i)
                listItems.Add(new struct_ListItem(list[i], list[i] + 2));

            listItems = list.ConvertAll<struct_ListItem>(x => new struct_ListItem(x, x + 2));

            int pos0 = list.IndexOf(888);
            int pos1 = listItems.FindIndex(x => x.item2_ == 890);

            listItems.Sort((l, r) => { return l.item1_ - r.item1_;  });
            listItems.Reverse();

            IReadOnlyCollection<struct_ListItem> listRd = listItems.AsReadOnly();
            foreach (var x in listRd)
                Console.WriteLine(x.item2_.ToString());

            Console.WriteLine("TestList End\n");
        }
        public void TestQueue()
        {
            Console.WriteLine("TestQueue Begin");

            Queue<int> que = new Queue<int>(16);
            foreach (int x in new int[] { 2, 4, 6, 8, 13 })
                que.Enqueue(x);
            que.Dequeue();
            Console.WriteLine(que.Peek());

            Console.WriteLine("TestQueue End\n");
        }
        public void TestStack()
        {
            Console.WriteLine("TestStack Begin");

            Stack<int> stk = new Stack<int>(16);
            foreach (int x in new int[] { 2, 4, 6, 8, 13 })
                stk.Push(x);
            stk.Pop();

            bool contain2 = stk.Contains(2);
            Console.WriteLine(stk.Peek());

            Console.WriteLine("TestStack End\n");
        }
        public void TestLinkedList()
        {
            LinkedList<int> llist = new LinkedList<int>();
            foreach (int x in new int[] { 2, 4, 6, 8, 13 })
                llist.AddFirst(x);
            llist.AddLast(33);
            llist.RemoveFirst();
            foreach (var x in llist)
                Console.WriteLine(x.ToString());
        }
        public void TestSortedList()
        {
            SortedList<string, string> dic = new SortedList<string, string>();
            dic["name"] = "eric cartman";
            dic["pname"] = "eric cartman";
            dic["qname"] = "eric cartman";
            dic["sentense"] = "screw you guys";

            string name;
            dic.TryGetValue("name", out name);

            if (dic.ContainsKey("name"))
                foreach(KeyValuePair<string, string> iter in dic)
                    Console.WriteLine(iter.Key + iter.Value);

            foreach(var key in dic.Keys)
                Console.WriteLine(key);

            foreach (var value in dic.Values)
                Console.WriteLine(value);

            var lookUp = dic.ToLookup(x => x.Value);
            foreach(KeyValuePair<string, string> iter in lookUp["eric cartman"])
                Console.WriteLine(iter.Key);
        }
    }

    public class MyAsyncAPM
    {
        void TestAPM_AsyncWaitHandle()
        {
            IAsyncResult result = Dns.BeginGetHostEntry("www.baidu.com", null, null);
            // Wait until the operation completes.
            result.AsyncWaitHandle.WaitOne();
            // The operation completed. Process the results.
            try
            {
                IPHostEntry host = Dns.EndGetHostEntry(result);
                string[] aliases = host.Aliases;
                IPAddress[] addresses = host.AddressList;
            }
            catch (SocketException e)
            {
                Console.WriteLine("Exception occurred while processing the request: {0}", e.Message);
            }
        }

        void TestAPM_Block()
        {
            IAsyncResult result = Dns.BeginGetHostEntry("www.baidu.com", null, null);
            // Do any additional work that can be done here.
            try
            {
                // EndGetHostByName blocks until the process completes.
                IPHostEntry host = Dns.EndGetHostEntry(result);
                string[] aliases = host.Aliases;
                IPAddress[] addresses = host.AddressList;
            }
            catch (SocketException e)
            {
                Console.WriteLine("An exception occurred while processing the request: {0}", e.Message);
            }
        }

        void TestAPM_Poll()
        {
            IAsyncResult result = Dns.BeginGetHostEntry("www.baidu.com", null, null);
            // Poll for completion information.
            while (result.IsCompleted != true)
            {
                //UpdateUserInterface();
            }
            // The operation is complete. Process the results.
            try
            {
                IPHostEntry host = Dns.EndGetHostEntry(result);
                string[] aliases = host.Aliases;
                IPAddress[] addresses = host.AddressList;
            }
            catch (SocketException e)
            {
                Console.WriteLine("An exception occurred while processing the request: {0}", e.Message);
            }
        }

        static int requestCounter;
        static ArrayList hostData = new ArrayList();
        static StringCollection hostNames = new StringCollection();
        void TestAPM_AsyncCallback()
        {
            AsyncCallback callBack = new AsyncCallback(ProcessDnsInformation);
            string host;
            do
            {
                Console.Write(" Enter the name of a host computer or <enter> to finish: ");
                host = Console.ReadLine();
                if (host.Length > 0)
                {
                    // Increment the request counter in a thread safe manner.
                    Interlocked.Increment(ref requestCounter);
                    // Start the asynchronous request for DNS information.
                    Dns.BeginGetHostEntry(host, callBack, host);
                }
            } while (host.Length > 0);

            // The user has entered all of the host names for lookup.
            // Now wait until the threads complete.
            while (requestCounter > 0)
            {
                //UpdateUserInterface();
            }
            
            // Display the results.
            for (int i = 0; i < hostNames.Count; i++)
            {
                if (hostData[i] as string != null)
                {   
                    // A SocketException was thrown.
                    Console.WriteLine("Request for {0} returned message: {1}", hostNames[i], hostData[i] as string);
                }
                else
                {
                    // Get the results.
                    IPHostEntry h = (IPHostEntry)hostData[i];
                    string[] aliases = h.Aliases;
                    IPAddress[] addresses = h.AddressList;
                }
            }
        }
        static void ProcessDnsInformation(IAsyncResult result)
        {
            string hostName = (string)result.AsyncState;
            hostNames.Add(hostName);
            try
            {
                // Get the results.
                IPHostEntry host = Dns.EndGetHostEntry(result);
                hostData.Add(host);
            }
            // Store the exception message.
            catch (SocketException e)
            {
                hostData.Add(e.Message);
            }
            finally
            {
                // Decrement the request counter in a thread-safe manner.
                Interlocked.Decrement(ref requestCounter);
            }
        }


        public class AsyncDemo
        {
            // The method to be executed asynchronously.
            public string TestMethod(int callDuration, out int outVal, out int threadId)
            {
                Console.WriteLine("Test method begins.");
                Thread.Sleep(callDuration);
                threadId = Thread.CurrentThread.ManagedThreadId;
                outVal = 0;
                return String.Format("My call time was {0}.", callDuration.ToString());
            }
        }
        // The delegate must have the same signature as the method
        // it will call asynchronously.
        public delegate string AsyncMethodCaller(int callDuration, out int outVal, out int threadId);

        public void TestAPM_AsyncDemo()
        {
            AsyncDemo demo = new AsyncDemo();
            var caller = new AsyncMethodCaller(demo.TestMethod);

            int threadId;
            int outValue;
            IAsyncResult result = caller.BeginInvoke(3000, out outValue, out threadId, null, null);

            {
                Thread.Sleep(0);
                Console.WriteLine("Main thread {0} does some work.", Thread.CurrentThread.ManagedThreadId);
            }
            //@1 result.AsyncWaitHandle.WaitOne();
            /*@2 while (result.IsCompleted == false)
            {
                Thread.Sleep(250);
                Console.Write(".");
            }*/

            string returnValue = caller.EndInvoke(out outValue, out threadId, result);
            Console.WriteLine("The call executed on thread {0}, with return value \"{1}\".", threadId, returnValue);
        }
    };

    public class MyAsyncEAP
    {

    }

    public class MyTestThread
    {
        public class ThreadObject
        {
            public object SyncLock = new object();
            public long SyncInterLock = 0;
            public Mutex SyncMutex = new Mutex(false, "ThreadObject Mutex");
            public SemaphoreSlim SynSem = new SemaphoreSlim(1);

            public AutoResetEvent EventProduce = new AutoResetEvent(true), EventComsumer = new AutoResetEvent(false);
            public ManualResetEventSlim EventProduceMan = new ManualResetEventSlim(true), EventComsumerMan = new ManualResetEventSlim(false);
            public CountdownEvent EventCD = new CountdownEvent(2);

            public void Producer(object param)
            {
                Console.WriteLine("Producer: Start");

                Queue<int> que = (Queue<int>)param;
                int startValue = 6;
                try
                {
                    while (startValue-- > 0)
                    {
#if USE_LOCK
                        lock (SyncLock)
                        {
                            if (que.Count == 0)
                            {
                                que.Enqueue(startValue);
                                Console.WriteLine("Producer: Enqueue {0:D}", startValue);
                            }
                        }
#elif USE_INTER_LOCK
                        while (Interlocked.CompareExchange(ref SyncInterLock, 1, 0) != 0)
                            ;
                        que.Enqueue(startValue);
                        Console.WriteLine("Producer: Enqueue {0:D}", startValue);
                        Interlocked.Decrement(ref SyncInterLock);
#elif USE_MUTEX
                        if (SyncMutex.WaitOne())
                        {
                            que.Enqueue(startValue);
                            Console.WriteLine("Producer: Enqueue {0:D}", startValue);
                            SyncMutex.ReleaseMutex();
                        }        
#elif USE_EVENT_SEMSLIM
                        //EventProduce.WaitOne();
                        EventProduceMan.Wait();
                        EventProduceMan.Reset();

                        SynSem.Wait();
                        if (que.Count == 0)
                        {
                            que.Enqueue(startValue);
                            Console.WriteLine("Producer: Enqueue {0:D}", startValue);
                            EventComsumer.Set();
                            EventComsumerMan.Set();
                        }
                        SynSem.Release();
#endif
                        que.Enqueue(startValue);
                        Console.WriteLine("Producer: Enqueue {0:D}", startValue);
                        EventCD.Signal();

                        Thread.Sleep(100);
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine("ThreadFunc1 Exception handled: {0:D}", e.Message);
                }

                Console.WriteLine("Producer: Finish");
            }

            public void Comsumer(object param)
            {
                Console.WriteLine("Comsumer: Start");

                Queue<int> que = (Queue<int>)param;
                while (true)
                {
#if USE_LOCK
                    lock (SyncLock)
                    {
                        if (que.Count > 0)
                        {
                            int startValue = que.Dequeue();
                            Console.WriteLine("Comsumer: Dequeue {0:D}", startValue);
                        }
                    }
#elif USE_INTER_LOCK
                    while (Interlocked.CompareExchange(ref SyncInterLock, 1, 0) != 0)
                        ;
                    if (que.Count > 0)
                    {
                        int startValue = que.Dequeue();
                        Console.WriteLine("Comsumer: Dequeue {0:D}", startValue);
                    }
                    Interlocked.Decrement(ref SyncInterLock);
#elif USE_MUTEX
                    if (SyncMutex.WaitOne())
                    {
                        if (que.Count > 0)
                        {
                            int startValue = que.Dequeue();
                            Console.WriteLine("Comsumer: Dequeue {0:D}", startValue);
                        }
                        SyncMutex.ReleaseMutex();
                    }
#elif USE_EVENT_SEMSLIM
                    //EventComsumer.WaitOne();
                    EventComsumerMan.Wait();
                    EventComsumerMan.Reset();

                    SynSem.Wait();
                    if (que.Count > 0)
                    {
                        int startValue = que.Dequeue();
                        Console.WriteLine("Comsumer: Dequeue {0:D}", startValue);
                        if (que.Count == 0)
                            //EventProduce.Set();
                            EventProduceMan.Set();
                    }
                    SynSem.Release();
#endif
                    EventCD.Wait();
                    while(que.Count > 0)
                    {
                        int startValue = que.Dequeue();
                        Console.WriteLine("Comsumer: Dequeue {0:D}", startValue);
                    }
                    EventCD.Reset();
                }

                Console.WriteLine("Comsumer: Finish");
            }
        }

        void Case10()
        {
            Queue<int> que = new Queue<int>();

            ThreadObject tobj = new ThreadObject();

            Thread tComsumer = new Thread(tobj.Comsumer);

            Thread tProducer = new Thread(tobj.Producer);
            tProducer.IsBackground = true;

            tComsumer.Start(que);
            tProducer.Start(que);
        }

        void Case1()
        {
            Barrier bar = new Barrier(2, b => Console.WriteLine("End of phase {0}", b.CurrentPhaseNumber + 1));
            ThreadStart func = () =>
            {
                Console.WriteLine("----------------------------------------------");
                bar.SignalAndWait();
            };
            new Thread(func).Start();
            new Thread(func).Start();
        }

        void Case2()
        {
            ReaderWriterLockSlim rwLock = new ReaderWriterLockSlim();
            Dictionary<string, string> dic = new Dictionary<string, string>();

            ThreadStart reader = () =>
            {
                bool readFalg = false;
                while (!readFalg)
                {
                    rwLock.EnterReadLock();
                    if (dic.ContainsKey("name"))
                    {
                        Console.WriteLine("read name={0}", dic["name"]);
                        readFalg = true;
                    }
                    rwLock.ExitReadLock();
                    Thread.Sleep(100);
                }

            };
            new Thread(reader).Start();
            new Thread(reader).Start();

            new Thread(()=> {
                try
                {
                    rwLock.EnterUpgradeableReadLock();
                    if (!dic.ContainsKey("name"))
                    {
                        try
                        {
                            rwLock.EnterWriteLock();
                            dic["name"] = "guts";
                        }
                        finally
                        {
                            rwLock.ExitWriteLock();
                        }
                        Console.WriteLine("write name={0}", dic["name"]);
                    }
                }
                finally
                {
                    rwLock.ExitUpgradeableReadLock();
                }
            }).Start();
        }

        static volatile int synInt = 0;

        void Case3()
        {
            new Thread(()=> {
                var w = new SpinWait();
                while (synInt < 20)
                {
                    w.SpinOnce();
                }
            }).Start();

            new Thread(()=> {
                while (++synInt < 20)
                    Thread.Sleep(1);
            }).Start();
        }

        void Case4()
        {
            using (var cts = new CancellationTokenSource())
            {
                ThreadPool.QueueUserWorkItem((object token_) =>
                {
                    CancellationToken token = (CancellationToken)token_;
                    while (!token.IsCancellationRequested)
                    {
                        Console.WriteLine("wait cancel");
                        Thread.Sleep(TimeSpan.FromSeconds(0.1));
                    }
                }, cts.Token);
                Thread.Sleep(TimeSpan.FromSeconds(0.5));
                cts.Cancel();
            }

            using (var evt = new ManualResetEvent(false))
            using (var cts = new CancellationTokenSource())
            {
                var worker = ThreadPool.RegisterWaitForSingleObject(evt,
                    (param, isTimedOut) => {//wait
                        if (isTimedOut)
                            cts.Cancel();
                        else
                            Console.WriteLine("Worker operation success param={0}", param);
                    }, 
                    1024, TimeSpan.FromSeconds(0.1), true);

                ThreadPool.QueueUserWorkItem(_ => {//work
                    for (int i = 0; i < 6; i++)
                    {
                        if (cts.Token.IsCancellationRequested)
                            return;
                        Thread.Sleep(TimeSpan.FromSeconds(1));
                    }
                    evt.Set();
                });

                Thread.Sleep(TimeSpan.FromSeconds(0.1).Add(TimeSpan.FromSeconds(2)));
                worker.Unregister(evt);
            }
        }

        void Case5()
        {
            Action<string> TaskMethod = (string name) =>
            {
                Console.WriteLine("Task {0} is running on a thread id {1}. Is thread pool thread: {2}", name, Thread.CurrentThread.ManagedThreadId, Thread.CurrentThread.IsThreadPoolThread);
            };

            var t1 = new Task(() => TaskMethod("Task 1"));
            var t2 = new Task(() => TaskMethod("Task 2"));
            t2.Start();
            t1.Start();

            Task.Run(() => TaskMethod("Task 3"));

            Task.Factory.StartNew(() => TaskMethod("Task 4"));
            Task.Factory.StartNew(() => TaskMethod("Task 5"), TaskCreationOptions.LongRunning);

            Thread.Sleep(TimeSpan.FromSeconds(1));
        }

        public void TestAll()
        {
            Case1();
            Case2();
            Case3();
            Case4();
            Case10();
        }
    }

    public class MyTestIO
    {
        public void TestConsole()
        {
            int kvalue = 50;
            Console.WriteLine($"kvalue={kvalue}");
            Console.WriteLine("kvalue={%d}", kvalue);
        }
    }
}