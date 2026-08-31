# Photoshop.NET production-readiness test

This project is a dependency-free managed smoke test for `Photoshop.NET`. It
calls the public wrapper through the real `DllImport` declarations, creates a
document with RGB and RGBA images, a group, a mask, and a supplied merged
preview, then verifies byte-exact read-back. The default run covers both PSD and
PSB output with Raw, RLE, ZIP, and ZIP-prediction compression. It also verifies
16- and 32-bit document metadata detection.
It also loads the copy from `runtimes/win-x64/native` by absolute path, so a
missing native dependency fails before the wrapper test starts.

## Native DLL requirements

The static Visual Studio build was configured with:

```text
cmake -S . -B vs2022 -G "Visual Studio 17 2022" -A x64 --preset x64-static-release
```

For `vs2022/PhotoshopAPI.C/Release/PhotoshopAPI.C.dll`,
`dumpbin /dependents` reports only:

- `bcrypt.dll`
- `KERNEL32.dll`
- `ADVAPI32.dll`

Both are Windows system DLLs. Therefore the minimum application-owned native
deployment for .NET `DllImport("PhotoshopAPI.C")` is:

- `PhotoshopAPI.C.dll`

The static build does not require `deflate.dll`, `MSVCP140.dll`,
`VCRUNTIME140.dll`, or any other MSVC runtime DLL. The .NET wrapper itself is
managed code and does not add another native dependency. Windows supplies the
system DLLs and resolves `DllImport("PhotoshopAPI.C")` to
`PhotoshopAPI.C.dll`.

For comparison, a build using the dynamic `x64-windows` vcpkg triplet requires
the project-owned `deflate.dll` and the MSVC runtime DLLs in addition to
`PhotoshopAPI.C.dll`. A NuGet package should place the required native files
under `runtimes/win-x64/native`.

Build and run the test with:

```powershell
dotnet run --project Photoshop.NET.Tests/Photoshop.NET.Tests.csproj -c Release
```

Additional PSD/PSB paths may be supplied after `--`. Each is opened through
the wrapper and probed for metadata and root-layer traversal. The built-in
16/32-bit cases copy the repository's layered single-image fixtures and
validate managed read/write/read metadata preservation. Supplied samples
provide deeper coverage of additional real documents.

If the native build is in a different directory, pass
`PhotoshopApiNativeBuildDirectory` to MSBuild. For a dynamic `x64-windows`
build, also pass `PhotoshopApiNativeDependencyDirectory` so the test can copy
`deflate.dll`.
