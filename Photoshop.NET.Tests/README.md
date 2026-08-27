# Photoshop.NET native load test

This project is a dependency-free managed smoke test for `Photoshop.NET`. It
calls the public wrapper through the real `DllImport` declarations, creates a
document with an image, group, and mask, and writes a temporary PSD.
It also loads the copy from `runtimes/win-x64/native` by absolute path, so a
missing native dependency fails before the wrapper test starts.

For the current x64 MSVC Release build, `dumpbin /dependents` reports:

- `PhotoshopAPI.C.dll`
  - `deflate.dll`
  - `MSVCP140.dll`
  - `MSVCP140_ATOMIC_WAIT.dll`
  - `VCRUNTIME140.dll`
  - `VCRUNTIME140_1.dll`
  - Windows system DLLs and CRT API-set DLLs
- `deflate.dll`
  - `VCRUNTIME140.dll`
  - Windows system DLLs and CRT API-set DLLs

Therefore the project copies these project-owned native files into
`runtimes/win-x64/native`:

- `PhotoshopAPI.C.dll`
- `deflate.dll`

For a directly executed test application, the same two files are also copied
to the application directory because an ordinary project output does not
automatically add a manually created `runtimes` directory to its native DLL
search path. A NuGet package uses the `runtimes/win-x64/native` layout.

The MSVC runtime must be installed through the Visual C++ Redistributable, or
the four MSVC runtime DLLs listed above must also be bundled by the consuming
application. The Windows system and CRT API-set DLLs are supplied by Windows.

Build and run the test with:

```powershell
dotnet run --project Photoshop.NET.Tests/Photoshop.NET.Tests.csproj -c Release
```

If the native build is in a different directory, pass
`PhotoshopApiNativeBuildDirectory` and/or
`PhotoshopApiNativeDependencyDirectory` to MSBuild.
