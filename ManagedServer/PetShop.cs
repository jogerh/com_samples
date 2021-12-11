using System;
using System.Runtime.InteropServices;
using InterfacesInterop;

namespace ManagedServer
{
    [Guid("5011c315-994d-49b4-b737-03a846f590a0")]
    [ProgId("ManagedServer.PetShop.1")]
    [ComVisible(true)]
    public class PetShop : IPetShop
    {
        public PetShop()
        {

        }
        public IDog BuyDog()
        {
            Type comServerType = Type.GetTypeFromCLSID(new Guid("d162d2f7-cdf4-44bc-8018-6058420bcfdc"));
            return Activator.CreateInstance(comServerType ?? throw new COMException()) as IDog;
        }

        public Address GetAddress()
        {
            var address = new Address();
            address.Street = "Suhms gate";
            address.PostalCode = "0363";
            address.City = "Oslo";
            return address;
        }
    }
}
