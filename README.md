# quantum-hosky-unreal

Quantum Hosky Unreal project (2026), built on Unreal 4D https://github.com/godot-dimensions/unreal-4d

## Engine and checkout layout

The engine needs both the Unreal 4D engine patches and Hazelight's AngelScript integration, including the Quantum Hosky binding changes.

The default checkout layout is:

```text
Unreal4D/
├── UnrealEngine5/
├── unreal-4d/
└── quantum-hosky-unreal/
    └── QuantumHosky/
```

The project references the shared Unreal 4D plugin through `../../unreal-4d/Plugins`. The engine defaults to `../../UnrealEngine5/` relative to the project directory. Pass `-EngineRoot` on Windows, `--engine-root` on macOS/Linux, or set `UNREAL_ENGINE_ROOT` to use another engine checkout.

## Build and run on Windows

From `QuantumHosky/`, run `Build-Windows.bat` to generate `QuantumHosky.sln`, VS Code metadata, and build the editor, or `Run-Windows.bat` to build and launch it. Generated VS Code configuration is also available at the repository root, with the shared Unreal4D plugin included in the workspace.

The PowerShell scripts can also be called directly:

```powershell
.\Build-Windows.ps1 -GenerateOnly
.\Build-Windows.ps1 -SkipProjectFiles
.\Build-Windows.ps1 -Target Game -Configuration Shipping
.\Run-Windows.ps1 -SkipBuild
```

## Build and run on macOS and Linux

From `QuantumHosky/`, use `Build-macOS.command` / `Run-macOS.command` on macOS or `Build-Linux.sh` / `Run-Linux.sh` on Linux. These launchers share `Build-Unix.sh` and `Run-Unix.sh`, generate VS Code metadata, and use the same engine and plugin layout as Windows.

The macOS paths also use the shared setup in the neighboring Unreal4D plugin’s `Tools` directory. Project generation, builds, plugin packaging, and ShaderCompileWorker receive matching Apple Clang warning exceptions, preventing warning-setting changes when switching between Quantum Hosky and the Unreal4D example. Other compiler settings, such as configuration, architecture, and Xcode selection, must also match to reuse engine build products. Source-engine editor builds ensure ShaderCompileWorker is built.

The setup discovers the installed Metal compiler and companion tools, creates an ignored override under the plugin’s `BuildArtifacts/MacToolchains`, and passes it to the editor and shader workers. Missing tools are reported before compilation starts. Install Xcode’s Metal Toolchain component, or select a nonstandard installation with `UNREAL4D_METAL_TOOLCHAIN`. Generated macOS VS Code tasks use the same setup; regenerate metadata after changing the selected Xcode installation. No machine-specific toolchain paths are committed.

If the launcher files are not executable after checkout, run:

```sh
chmod +x Build-*.sh Run-*.sh Build-macOS.command Run-macOS.command
```

The shell scripts can also be invoked:

```sh
./Build-Linux.sh --generate-only
./Build-Linux.sh --skip-project-files
./Build-Linux.sh --target Game --configuration Shipping
./Run-Linux.sh --skip-build
./Run-macOS.command --configuration DebugGame
```

Build defaults are the `QHEditor` target and `Development` configuration. `--target Game` builds `QHGame`; it does not package the game. Run scripts launch the editor and add `-debug` for `DebugGame`. `--generate-only` (or `-GenerateOnly` on Windows) generates IDE metadata without building or launching the editor.

The macOS and Linux scripts have been checked on Windows; native builds on those platforms remain untested.
