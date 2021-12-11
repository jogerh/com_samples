using System;
using System.Runtime.InteropServices;
using System.Threading;
using NUnit.Framework;
using Moq;
using ComUtilityManaged;
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

        // This test demonstrates that .Net Hen implementation does not marshal calls to thread where Hen created.
        [Test]
        public void RequireThat_HenIsCalledOnWorkerStaThread_WhenDotNetHenIsCreatedOnTestStaThread()
        {
            // Here we create "default" .Net Hen implementation.
            var hen = new Mock<IHen>();

            int calledOnThreadId = 0;
            hen.Setup(mock => mock.Cluck()).Callback(() =>
            {
                // Store in callback thread id where Cluck is called.
                calledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            // Now, call Cluck from a different thread that .Net Hen was created.
            var thread = new Thread(() =>
            {
                hen.Object.Cluck();
            });
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();

            // Here, note that Join is very important because it pumps messages until the thread exits.
            thread.Join();

            hen.Verify(mock => mock.Cluck(), Times.AtLeastOnce);

            // Here we see that even .Net hen was created on test STA thread, Cluck call was happened on worker.
            Assert.AreEqual(calledOnThreadId, thread.ManagedThreadId);
        }

        // This test is same as above, but it uses Hen inherited from StandardOleMarshalObject.
        // It demonstrates that even Hen inherited from StandardOleMarshalObject calls are still
        // happened on the worker thread.
        [Test]
        public void RequireThat_HenIsCalledOnWorkerStaThread_WhenDotNetHenIsStandardOleMarshalObject()
        {
            // Here we create Hen inherited from StandardOleMarshalObject on a single threaded apartment
            var hen = new Mock<Hen>();

            int calledOnThreadId = 0;
            hen.Setup(mock => mock.Cluck()).Callback(() =>
            {
                // Store in callback thread id where call happened.
                calledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            // Now, call Cluck from a different thread that .Net Hen was created
            var thread = new Thread(() =>
            {
                hen.Object.Cluck();
            });
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();

            // Here, note that Join is very important because it pumps messages until the thread exits.
            thread.Join();

            hen.Verify(mock => mock.Cluck(), Times.AtLeastOnce);

            // Here we see that even .Net Hen inherited from StandardOleMarshalObject
            // and created on test STA thread, Cluck is called on worker.
            Assert.AreEqual(calledOnThreadId, thread.ManagedThreadId);
        }

        // This test demonstrates that it is not enough to call RoGetAgileReference to get marshaling with .Net Hen.
        [Test]
        public void RequireThat_CluckIsCalledOnWorkerStaThread_WhenDefaultDotNetHenIsMarshaledWithAgileReferenceWrapper()
        {
            // Here we create "default" .Net Hen implementation.
            var hen = new Mock<IHen>();
            // We create AgileReferenceWrapper what uses RoGetAgileReference to get thread safe object.
            var henAgile = new AgileReferenceWrapper<IHen>(hen.Object);

            int calledOnThreadId = 0;
            hen.Setup(mock => mock.Cluck()).Callback(() =>
            {
                // Store in callback thread id where Cluck is called.
                calledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            // Now, call Cluck from a different thread that .Net Hen was created.
            var thread = new Thread(() =>
            {
                // We call resolve on AgileReferenceWrapper to get object which calls will be marshaled
                // to the thread AgileReference was created on.
                var h = henAgile.Resolve();
                // Cluck should happen on worker STA thread.
                h.Cluck();
            });
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();

            // Here, note that Join is very important because it pumps messages until the thread exits.
            thread.Join();

            hen.Verify(mock => mock.Cluck(), Times.AtLeastOnce);

            // Here we see that even .Net hen is marshaled with AgileReferenceWrapper Cluck is called on worker STA thread.
            Assert.AreEqual(calledOnThreadId, thread.ManagedThreadId);
        }

        // This test is same as above, but it uses Hen inherited from StandardOleMarshalObject.
        // It demonstrates that StandardOleMarshalObject and RoGetAgileReference are both mandatory
        // conditions to get marshaling with .Net implementation.
        [Test]
        public void RequireThat_CluckIsCalledOnStaThread_WhenDotNetHenInheritedFromStandardOleMarshalObjectIsMarshaledWithAgileReferenceWrapper()
        {
            // Here we create .Net Hen implementation inherited from StandardOleMarshalObject.
            var hen = new Mock<Hen>();
            // We create AgileReferenceWrapper what uses RoGetAgileReference to get thread safe object.
            var henAgile = new AgileReferenceWrapper<IHen>(hen.Object);

            int calledOnThreadId = 0;
            hen.Setup(mock => mock.Cluck()).Callback(() =>
            {
                // Store in callback thread id where Cluck is called.
                calledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            // Now, call Cluck from a different thread that .Net Hen was created.
            var thread = new Thread(() =>
            {
                // We call resolve on AgileReferenceWrapper to get object which calls will be marshaled
                // to the thread AgileReference was created on.
                var h = henAgile.Resolve();
                // Cluck should happen on test STA thread.
                h.Cluck();
            });
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();

            // Here, note that Join is very important because it pumps messages until the thread exits.
            thread.Join();

            hen.Verify(mock => mock.Cluck(), Times.AtLeastOnce);

            // Here we see that .Net hen inherited from StandardOleMarshalObject and marshaled
            // with AgileReferenceWrapper Cluck is called on test STA thread.
            Assert.AreEqual(calledOnThreadId, Thread.CurrentThread.ManagedThreadId);
        }

        // This test demonstrates that marshaling did not happen if both classes is implemented
        // In .Net and inherited from StandardOleMarshalObject.
        [Test]
        public void RequireThat_CluckObserverAndHenAreCalledOnWorkerStaThread_WhenCluckAsyncIsCalledOnNonMarshaledHenFromWorkerThread()
        {
            // Here we create the cluck observer inherited from StandardOleMarshalObject on a single threaded apartment
            var cluckObserver = new Mock<AsyncCluckObserver>();

            int cluckObserverCalledOnThreadId = 0;
            cluckObserver.Setup(mock => mock.OnCluck()).Callback(() =>
            {
                // Allow us to observe the thread that the observer was actually called on.
                cluckObserverCalledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            var hen = new Mock<Hen>();
            int henCalledOnThreadId = 0;

            hen.Setup(mock => mock.CluckAsync(It.IsAny<IAsyncCluckObserver>()))
                .Callback<IAsyncCluckObserver>(observer =>
                    {
                        // Allow us to observe the thread that the hen was actually called on.
                        henCalledOnThreadId = Thread.CurrentThread.ManagedThreadId;
                        observer.OnCluck();
                    });

            // Now, call CluckAsync from a different thread that is its own single threaded apartment
            var thread = new Thread(() =>
            {
                // We pass AsyncCluckObserver on the worker thread, so calls to it should happen here.
                hen.Object.CluckAsync(cluckObserver.Object);
            });
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();

            // Here, note that Join is very important because it pumps messages until the thread exits.
            thread.Join();

            cluckObserver.Verify(mock => mock.OnCluck(), Times.AtLeastOnce);

            // Reader must pay attention on code below, we use It.Is<IAsyncCluckObserver>(observer => ReferenceEquals(observer, cluckObserver.Object))
            // expression to show that observers references are equal, this indirectly proves that marshaling did not happen.
            hen.Verify(mock => mock.CluckAsync(It.Is<IAsyncCluckObserver>(observer => ReferenceEquals(observer, cluckObserver.Object))), Times.AtLeastOnce);

            // Marshaling did not happen above, so CluckAsync and OnCluck calls should happen on same thread,
            // and that thread is the worker.
            Assert.AreEqual(henCalledOnThreadId, cluckObserverCalledOnThreadId);
            Assert.AreEqual(cluckObserverCalledOnThreadId, thread.ManagedThreadId);
        }

        // This test shows, how to get behavior RequireThat_CluckObserverIsCalledOnStaThread_WhenCluckAsyncIsCalledFromWorkerThread
        // with both interfaces implemented in .Net using AgileReferenceWrapper.
        [Test]
        public void RequireThat_HenIsCalledOnTestStaThread_WhenCluckAsyncIsCalledOnMarshaledHenFromWorkerThread()
        {
            // Here we create the cluck observer inherited from StandardOleMarshalObject on a single threaded apartment
            var cluckObserver = new Mock<AsyncCluckObserver>();

            int cluckObserverCalledOnThreadId = 0;
            cluckObserver.Setup(mock => mock.OnCluck()).Callback(() =>
            {
                // Allow us to observe the thread that the observer was actually called on.
                cluckObserverCalledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            var hen = new Mock<Hen>();
            int henCalledOnThreadId = 0;
            hen.Setup(mock => mock.CluckAsync(It.IsAny<IAsyncCluckObserver>()))
                .Callback<IAsyncCluckObserver>(observer =>
                {
                    // Allow us to observe the thread that the hen was actually called on.
                    henCalledOnThreadId = Thread.CurrentThread.ManagedThreadId;
                    observer.OnCluck();
                });

            // Create agile reference to hen, this should make behavior same as in:
            // RequireThat_CluckObserverIsCalledOnStaThread_WhenCluckAsyncIsCalledFromWorkerThread
            var henAgile = new AgileReferenceWrapper<IHen>(hen.Object);

            // Now, call CluckAsync from a different thread that is its own single threaded apartment
            var thread = new Thread(() =>
            {
                // Calls to resolved hen should be on test STA thread.
                var h = henAgile.Resolve();

                // We pass AsyncCluckObserver on the worker thread to resolved hen,
                // so calls to cluckObserver should happen on this thread.
                h.CluckAsync(cluckObserver.Object);
            });
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();

            // Here, note that Join is very important because it pumps messages until the thread exits.
            thread.Join();

            cluckObserver.Verify(mock => mock.OnCluck(), Times.AtLeastOnce);

            // Reader must pay attention on code below, we use It.Is<IAsyncCluckObserver>(observer => !ReferenceEquals(observer, cluckObserver.Object))
            // expression to show that observers references are NOT equal, this indirectly proves that marshaling is happened.
            hen.Verify(mock => mock.CluckAsync(It.Is<IAsyncCluckObserver>(observer => !ReferenceEquals(observer, cluckObserver.Object))), Times.AtLeastOnce);

            // Because AgileReferenceWrapper is created on test STA thread, CluckAsync call must be
            // on test STA thread, on the other hand OnCluck should be marshaled to worker thread.
            Assert.AreEqual(henCalledOnThreadId, Thread.CurrentThread.ManagedThreadId);
            Assert.AreEqual(cluckObserverCalledOnThreadId, thread.ManagedThreadId);
        }

        static IHen CreateAtlHen()
        {
            Type comServerType = Type.GetTypeFromProgID("AtlServer.AtlHen.1");
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

    public abstract class Hen : StandardOleMarshalObject, IHen
    {
        /// <summary> Cluck is abstract to enable mocking. </summary>
        public abstract void Cluck();

        /// <summary> CluckAsync is abstract to enable mocking. </summary>
        public abstract void CluckAsync(IAsyncCluckObserver cluckObserver);
    }
}