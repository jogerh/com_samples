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

        }

        [Test]
        public void RequireThat_CluckObserverIsCalledOnStaThread_WhenCluckAsyncIsCalledFromStaThread()
        {
            // Arrange
            var cluckObserver = new Mock<IAsyncCluckObserver>();

            int calledOnThreadId = 0;
            cluckObserver.Setup(mock => mock.OnCluck()).Callback(() =>
            {
                calledOnThreadId = Thread.CurrentThread.ManagedThreadId;
            });

            // Act
            m_hen.CluckAsync(cluckObserver.Object);

            // Assert
            cluckObserver.Verify(mock => mock.OnCluck(), Times.AtLeastOnce);
            Assert.AreEqual(calledOnThreadId, Thread.CurrentThread.ManagedThreadId);
        }

        private IHen m_hen;
    }
}