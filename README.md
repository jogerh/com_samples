# COM examples

COM Examples contains sample implementations that use Microsoft COM technology. The intention is to provide minimal working samples that explains how to implement and use COM objects.

## Content

* [Interfaces](Interfaces#interfaces): COM interfaces written in interface definition language (IDL)
* [AtlServer](AtlServer#atlhenlib): An ATL implementation of a COM server that provides hens
* [AtlFreeServer](AtlFreeServer#atlfreeserver): An COM server implemented without ATL. It provides dogs
* [ManagedServer](ManagedServer/): An COM server implemented in .NET. Here you can buy dogs.
* [PyComServer](PyComServer/): An Com server implemented in python. It provides snakes.
* [WinrtServer](WinrtServer/): An COM server implemented in winrt as an Universal Windows component. It provides programmers.
* [ComUtility](ComUtility/): COM related utilities used across the projects
* [TutorialsAndTests](TutorialsAndTests/): Tutorials and tests used to demonstrate how COM objects are used from C++
* [InteropTests](InteropTests/): Unit tests used to demonstrate how COM objects are used from .NET
* [PyComTests](PyComTests/): Unit tests used to demonstrate how COM objects are used from python

## Building

Dependencies: 
* Visual Studio 2019 with Universal Windows Platform development workload and Python development tools
* [C++/WinRT templates and visualizer for VS2019 (Wsix)](https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/intro-to-using-cpp-with-winrt#visual-studio-support-for-cwinrt-xaml-the-vsix-extension-and-the-nuget-package)
* Python 3.9 
