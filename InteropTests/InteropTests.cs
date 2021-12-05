using System;
using System.Runtime.InteropServices;
using System.Threading;
using NUnit.Framework;
using Moq;
using InterfacesInterop;

namespace InteropTests
{
    [TestFixture, Apartment(ApartmentState.STA)]
    public class InteropTestsWithSingleThreadedApartment
    {
        // This test verifies that in a single threaded case, callbacks from COM are executed
        // on the caller thread. Nothing fancy going on here.
        [Test]
        public void RequireThat_CluckObserverIsCalledOnStaThread_WhenCluckAsyncIsCalledFromStaThread()
        {
            var cluckObserver = new Mock<IAsyncCluckObserver>();

            int calledOnThreadId = 0;
            cluckObserver.Setup(mock => mock.OnCluck()).Callback(() =>
            {
                calledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            var hen = CreateAtlHen();
            hen.CluckAsync(cluckObserver.Object);

            cluckObserver.Verify(mock => mock.OnCluck(), Times.AtLeastOnce);

            // As expected, the CluckObserver OnCluck method was called on the same
            // thread as we called IHen CluckAsync on
            Assert.AreEqual(calledOnThreadId, Thread.CurrentThread.ManagedThreadId);
        }


        // This test demonstrates a more interesting case. We create a IAsyncCluckObserver
        // on a single threaded apartment, but then we call CluckAsync from a worker thread.
        // What will happen now? According to COM, objects created on a single threaded
        // apartment should never receive calls from other threads. This test shows that
        // the .NET runtime and COM runtime handles this correctly. 
        [Test]
        public void RequireThat_CluckObserverIsCalledOnStaThread_WhenCluckAsyncIsCalledFromWorkerThread()
        {
            // Here we create the cluck observer on a single threaded apartment
            var cluckObserver = new Mock<IAsyncCluckObserver>();

            int calledOnThreadId = 0;
            cluckObserver.Setup(mock => mock.OnCluck()).Callback(() =>
            {
                // Allow us to observe the thread that the observer was actually called on.
                // Feel free to put a breakpoint here, and also in the AtlHen implementation
                calledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            var hen = CreateAtlHen();

            // Now, call CluckAsync from a different thread that is its own single threaded apartment
            var thread = new Thread(() =>
            {
                hen.CluckAsync(cluckObserver.Object);
            });
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();

            // Here, note that Join is very important because it pumps messages until the thread exits.
            thread.Join();

            cluckObserver.Verify(mock => mock.OnCluck(), Times.AtLeastOnce);

            // Here we see that even if we call CluckAsync on a worker thread, the
            // OnCluck function is called on the correct single threaded apartment thread.
            // Of course, this happens because the IHen::CluckAsync is being marshaled over
            // to the single threaded apartment, and therefore OnCluck also is called on
            // the same thread.
            Assert.AreEqual(calledOnThreadId, Thread.CurrentThread.ManagedThreadId);
        }

        // This test demonstrates how marshaling works with StandardOleMarshalObject.
        // We created AsyncCluckObserver on the test STA thread, then we call CluckAsync on the worker thread.
        // According to the Microsoft documentation, call should happen on the thread,
        // where we pass StandardOleMarshalObject to another STA object, it is worker for our case
        [Test]
        public void RequireThat_CluckObserverIsCalledOnWorkerStaThread_WhenCluckObserverInheritsStandardOleMarshalObject()
        {
            // Here we create the cluck observer inherited from StandardOleMarshalObject on a single threaded apartment
            var cluckObserver = new Mock<AsyncCluckObserver>();

            int calledOnThreadId = 0;
            cluckObserver.Setup(mock => mock.OnCluck()).Callback(() =>
            {
                // Allow us to observe the thread that the observer was actually called on.
                // Feel free to put a breakpoint here, and also in the AtlHen implementation
                calledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            var hen = CreateAtlHen();

            // Now, call CluckAsync from a different thread that is its own single threaded apartment
            var thread = new Thread(() =>
            {
                // We pass AsyncCluckObserver on the worker thread, so calls to it should happen here.
                hen.CluckAsync(cluckObserver.Object);
            });
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();

            // Here, note that Join is very important because it pumps messages until the thread exits.
            thread.Join();

            cluckObserver.Verify(mock => mock.OnCluck(), Times.AtLeastOnce);

            // Here we see that when we call CluckAsync on a worker thread, the
            // OnCluck function is called on the correct single threaded apartment thread (worker thread),
            // while CluckAsync calls was happened on the test thread.
            Assert.AreEqual(calledOnThreadId, thread.ManagedThreadId);
        }

        static IHen CreateAtlHen()
        {
            Type comServerType = Type.GetTypeFromProgID("AtlHenLib.AtlHen.1");
            Assert.NotNull(comServerType);
            return Activator.CreateInstance(comServerType) as IHen;
        }
    }

    /// <summary>
    /// StandardOleMarshalObject will make marshaling available for us.
    /// See tests where it used.
    /// </summary>
    public abstract class AsyncCluckObserver : StandardOleMarshalObject, IAsyncCluckObserver
    {
        /// <summary> OnCluck is abstract to enable mocking. </summary>
        public abstract void OnCluck();
    }
}