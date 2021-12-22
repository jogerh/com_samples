import unittest
import comtypes

from comtypes.client import GetModule, CreateObject
# To be able to use declared interfaces,
# we have to generate wrapper code from .tlb file, or from .dll
GetModule("..\Build\Output\Interfaces.tlb")

# Simple tests to demonstrate how COM classes can be used from python.
class Test_Examples(unittest.TestCase):
    def test_example_how_to_use_managed_server_in_python(self):
        # From prog id create IPetShop, this interface is declared in Interfaces\IPetShop.idl
        # The implementation is in ManagedServer\PetShop.cs
        shop = CreateObject("ManagedServer.PetShop.1", interface = comtypes.gen.Interfaces.IPetShop)

        # Like in .Net, all COM related code is wrapped, so all 
        # arguments marked [out] or [out, retval] in the IDL are returned from a successful method call
        # See https://pythonhosted.org/comtypes/#creating-and-accessing-com-objects for details
        address = shop.GetAddress()

        # Add asserts to ensure that return address is correct according to implementation
        assert address.Street == "Suhms gate"
        assert address.PostalCode == "0363"
        assert address.City == "Oslo";

    def test_example_how_to_use_atl_server_in_python(self):
        # From class id create IHen, this interface is declared in Interfaces\IHen.idl
        # The implementation is in AtlServer\AtlHen.cpp
        hen = CreateObject("{9eedb943-b267-4f0c-b8b6-59fe3851f239}", interface = comtypes.gen.Interfaces.IHen)
        hen.Cluck()

if __name__ == '__main__':
    unittest.main()
