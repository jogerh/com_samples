using System;
using System.Threading;
using NUnit.Framework;
using Moq;
using InterfacesInterop;
namespace InteropTests
{
    [TestFixture, Apartment(ApartmentState.STA)]
    public class InteropTestsWithSingleThreadedApartment
    {
        [SetUp]
        public void Setup()
        {
            Type comServerType = Type.GetTypeFromProgID("AtlHenLib.AtlHen.1");
            Assert.NotNull(comServerType);
            m_hen = Activator.CreateInstance(comServerType) as IHen;
            m_apartmentThreadId = Thread.CurrentThread.ManagedThreadId;

        }

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

            m_hen.CluckAsync(cluckObserver.Object);

            cluckObserver.Verify(mock => mock.OnCluck(), Times.AtLeastOnce);

            // As expected, the CluckObserver OnCluck method was called on the same
            // thread as we called IHen CluckAsync on
            Assert.AreEqual(calledOnThreadId, m_apartmentThreadId);
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

            // Now, call CluckAsync from a different thread that is its own single threaded apartment
            var thread = new Thread(() =>
            {
                m_hen.CluckAsync(cluckObserver.Object);
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
            Assert.AreEqual(calledOnThreadId, m_apartmentThreadId);
        }

        private IHen m_hen;
        private int m_apartmentThreadId;
    }
}