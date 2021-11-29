using System;
using System.Diagnostics;
using NUnit.Framework;
using Moq;
using InterfacesInterop;
namespace InteropTests
{
    public class InteropTestsWithHens
    {
        [SetUp]
        public void Setup()
        {
            Type comServerType = Type.GetTypeFromProgID("AtlHenLib.AtlHen.1");
            Assert.NotNull(comServerType);
            m_hen = Activator.CreateInstance(comServerType) as IHen;
        }

        [Test]
        public void RequireThat_CluckObserverIsCalled_WhenPassingAsMockToAtlHen()
        {
            var cluckObserver = new Mock<IAsyncCluckObserver>();

            m_hen.CluckAsync(cluckObserver.Object); // TODO: Verify that Cluck is called on the right thread

            cluckObserver.Verify(mock => mock.OnCluck(), Times.AtLeastOnce);
        }

        private IHen m_hen;
    }
}