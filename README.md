# macOS App Creator

![macOS](https://img.shields.io/badge/os-macOS-black?logo=apple)
![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)
![License](https://img.shields.io/badge/License-MIT-green.svg)

**macOS App Creator** is a lightning-fast, native C++ command-line tool designed to completely automate the tedious process of turning raw CMake C++ projects into distributable, fully-featured macOS `.app` bundles. 

Whether you prefer lightning-fast terminal prompts or native macOS graphical dialogs, this tool compiles your code, structures the app bundle, generates the XML `Info.plist`, attaches icons, and **automatically pulls and bundles all external dependencies** (`.dylib` files) so your app works on *any* Mac.

---

## Install with Homebrew
```bash
brew tap Nexus-Titan/tab https://github.com/Nexus-Titan/homebrew-tap.git
brew update
brew install tiwut-mac-app-creator
```
run with:
```bash
mac-app-creator

# Samples:
mac-app-creator -create -build -path /MyProject
# or -> for gui, run:
mac-app-creator -gui -create -build -path /MyProject
```

## Features

- **Automated CMake Builds:** Automatically generates build files and compiles your Release binaries.
- **Instant App Packaging:** Creates the standard `MyApp.app/Contents/{MacOS,Resources}` structure instantly.
- **Smart Dependency Bundling:** Automatically resolves and bundles external `.dylib` libraries (using `macdylibbundler`) so your app doesn't crash on other users' machines.
- **Dynamic Plist Generation:** Asks for App Name, Bundle ID, Version, Minimum OS, and Categories, generating a perfect `Info.plist`.
- **GUI & CLI Modes:** Use `-gui` to trigger native macOS graphical popups (powered by AppleScript), or stick to the terminal for CI/CD pipelines.
- **Edit Mode:** Easily modify properties (like version numbers) of existing `.app` bundles without rebuilding the code.
- **Zero Heavy Dependencies:** Written in raw C++17. No massive GUI frameworks (like Qt) required for the graphical prompts.

---

## Prerequisites

Before using or compiling this tool, ensure your macOS environment is set up:

1. **Xcode Command Line Tools:** (For the C++ compiler)
   ```bash
   xcode-select --install
   ```
2. **CMake:** (For building your target projects)
   ```bash
   brew install cmake
   ```
3. **Mac Dylib Bundler:** (Crucial for bundling dependencies into your `.app`)
   ```bash
   brew install dylibbundler
   ```

---

## Compilation / Installation

Compile the tool directly using macOS's default `clang++` compiler. No Makefiles needed:

```bash
clang++ -std=c++17 -O3 mac-app-creator.cpp -o mac-app-creator
```

*(Optional: Move it to your `/usr/local/bin` to use it from anywhere!)*
```bash
sudo mv mac-app-creator /usr/local/bin/
```

---

## Usage Guide

The tool operates using a modular flag system. 

### 1. Create and Build an App (Terminal Mode)
Navigate to a folder containing a `CMakeLists.txt` project and run:

```bash
./mac-app-creator -create -build -path /Users/tiwut/Downloads/MyProject
```
**What this does:**
- Prompts you in the terminal for your App Name, Version, Bundle ID, and Icon path.
- Runs `cmake` and builds the executable.
- Packages everything into `/Users/tiwut/Downloads/MyProject/YourApp.app`.
- Pulls in all external `.dylib` dependencies automatically.

### 2. Create and Build an App (GUI Mode)
If you prefer user-friendly graphical popup boxes instead of terminal text, simply add the `-gui` flag!

```bash
./mac-app-creator -gui -create -build -path /Users/tiwut/Downloads/MyProject
```
**What this does:**
- Identical to the command above, but uses macOS native `osascript` windows to ask you for your app's metadata. 

### 3. Edit an Existing App Bundle
If you already have a compiled `.app` and just need to update a property (like bumping the `CFBundleVersion` or changing the App Name) without rebuilding the C++ code:

```bash
./mac-app-creator -edit -path /Users/tiwut/Downloads/Sample.app
```
*(You can also append `-gui` to this command to use graphical prompts).*

---

## Command Line Arguments

| Flag | Description |
| :--- | :--- |
| `-path <dir>` | **(Required)** The target directory containing your CMake project, or the path to a `.app` file if using `-edit`. |
| `-create` | Tells the tool to create a new `.app` bundle and generate an `Info.plist`. |
| `-build` | Instructs the tool to execute CMake generation and build commands before packaging. |
| `-edit` | Opens an existing `.app` bundle to safely modify its internal `Info.plist`. |
| `-gui` | Switches all user prompts from standard terminal text to native macOS graphical dialog boxes. |

---

## Under the Hood

How does it achieve all this without external C++ libraries?
- **Native GUI:** Instead of embedding Qt or ImGui, it executes standard macOS `osascript` (AppleScript) commands to trigger native `display dialog` boxes.
- **Plist Editing:** Safely utilizes Apple's built-in `/usr/libexec/PlistBuddy` to mutate XML files without corrupting them.
- **Filesystem:** Utilizes modern `<filesystem>` from C++17 for cross-platform, safe directory creation and file copying.
- **Portability:** Uses `dylibbundler` to rewrite the `@executable_path` and `@rpath` inside your Mach-O binaries.

---

## 🤝 Contributing

Pull requests are welcome! If you'd like to add features (like automatic code-signing via `codesign` or `.dmg` generation via `hdiutil`), feel free to fork the repository and submit a PR.

## License

This project is licensed under the [MIT License](LICENSE) - see the LICENSE file for details. Do whatever you want with it!
