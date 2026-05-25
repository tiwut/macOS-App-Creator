#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct AppInfo {
  std::string appName;
  std::string executableName;
  std::string bundleId;
  std::string version;
  std::string buildNumber;
  std::string iconPath;
  std::string minOS;
  std::string copyright;
  std::string category;
};

std::string execCommand(const char *cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe)
    throw std::runtime_error("popen() failed!");
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

std::string askUser(const std::string &prompt,
                    const std::string &defaultVal = "") {
  std::cout << prompt;
  if (!defaultVal.empty())
    std::cout << " [" << defaultVal << "]: ";
  else
    std::cout << ": ";

  std::string input;
  std::getline(std::cin, input);
  if (input.empty())
    return defaultVal;
  return input;
}

std::string askUserGUI(const std::string &prompt,
                       const std::string &defaultVal = "") {
  std::string script = "osascript -e 'text returned of (display dialog \"" +
                       prompt + "\" default answer \"" + defaultVal + "\")'";
  std::string result = execCommand(script.c_str());
  if (!result.empty() && result.back() == '\n')
    result.pop_back();
  return result.empty() ? defaultVal : result;
}

void createPlistBuddy(const fs::path &appPath, const AppInfo &info) {
  fs::path plistPath = appPath / "Contents" / "Info.plist";
  std::ofstream plist(plistPath);
  plist << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
           "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
        << "<plist version=\"1.0\">\n<dict>\n"
        << "\t<key>CFBundleExecutable</key>\n\t<string>" << info.executableName
        << "</string>\n"
        << "\t<key>CFBundleIdentifier</key>\n\t<string>" << info.bundleId
        << "</string>\n"
        << "\t<key>CFBundleName</key>\n\t<string>" << info.appName
        << "</string>\n"
        << "\t<key>CFBundleVersion</key>\n\t<string>" << info.buildNumber
        << "</string>\n"
        << "\t<key>CFBundleShortVersionString</key>\n\t<string>" << info.version
        << "</string>\n"
        << "\t<key>LSMinimumSystemVersion</key>\n\t<string>" << info.minOS
        << "</string>\n"
        << "\t<key>NSHumanReadableCopyright</key>\n\t<string>" << info.copyright
        << "</string>\n"
        << "\t<key>LSApplicationCategoryType</key>\n\t<string>" << info.category
        << "</string>\n"
        << "\t<key>NSHighResolutionCapable</key>\n\t<true/>\n";

  if (!info.iconPath.empty()) {
    plist << "\t<key>CFBundleIconFile</key>\n\t<string>AppIcon.icns</string>\n";
  }

  plist << "</dict>\n</plist>\n";
  plist.close();
}

void editPlist(const fs::path &plistPath, const std::string &key,
               const std::string &value) {
  std::string cmd = "/usr/libexec/PlistBuddy -c \"Set :" + key + " '" + value +
                    "'\" \"" + plistPath.string() + "\"";
  std::system(cmd.c_str());
}

int main(int argc, char *argv[]) {
  bool useGui = false;
  bool doCreate = false;
  bool doBuild = false;
  bool doEdit = false;
  std::string basePath = "";

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-gui")
      useGui = true;
    else if (arg == "-create")
      doCreate = true;
    else if (arg == "-build")
      doBuild = true;
    else if (arg == "-edit")
      doEdit = true;
    else if (arg == "-path" && i + 1 < argc) {
      basePath = argv[++i];
    }
  }

  if (basePath.empty()) {
    std::cerr << "Error: You must specify a path using -path\n";
    return 1;
  }

  fs::path workPath(basePath);

  if (doCreate) {
    AppInfo info;
    std::cout << "\n=== macOS App Creator ===\n";

    if (useGui) {
      info.appName = askUserGUI("App Name", "MyApp");
      info.executableName =
          askUserGUI("CMake Executable Target Name", info.appName);
      info.bundleId =
          askUserGUI("Bundle Identifier", "com.mycompany." + info.appName);
      info.version = askUserGUI("Version", "1.0.0");
      info.buildNumber = askUserGUI("Build Number", "1");
      info.iconPath =
          askUserGUI("Path to .icns icon (Leave empty if none)", "");
      info.minOS = askUserGUI("Minimum macOS Version", "10.15");
      info.copyright = askUserGUI("Copyright Info", "© 2024");
      info.category =
          askUserGUI("App Category (e.g., public.app-category.utilities)",
                     "public.app-category.utilities");
    } else {
      info.appName = askUser("App Name", "MyApp");
      info.executableName =
          askUser("CMake Executable Target Name", info.appName);
      info.bundleId =
          askUser("Bundle Identifier", "com.mycompany." + info.appName);
      info.version = askUser("Version", "1.0.0");
      info.buildNumber = askUser("Build Number", "1");
      info.iconPath = askUser("Path to .icns icon (Leave empty if none)", "");
      info.minOS = askUser("Minimum macOS Version", "10.15");
      info.copyright = askUser("Copyright Info", "© 2024");
      info.category = askUser("App Category", "public.app-category.utilities");
    }

    if (doBuild) {
      std::cout << "\n[*] Building project via CMake...\n";
      fs::current_path(workPath);
      std::system("cmake -B build -DCMAKE_BUILD_TYPE=Release");
      if (std::system("cmake --build build --config Release") != 0) {
        std::cerr << "Error: CMake build failed!\n";
        return 1;
      }
    }

    fs::path appBundle = workPath / (info.appName + ".app");
    std::cout << "\n[*] Creating App Bundle at: " << appBundle << "\n";
    fs::create_directories(appBundle / "Contents/MacOS");
    fs::create_directories(appBundle / "Contents/Resources");
    fs::path builtExe = workPath / "build" / info.executableName;
    if (fs::exists(builtExe)) {
      fs::copy_file(builtExe,
                    appBundle / "Contents/MacOS" / info.executableName,
                    fs::copy_options::overwrite_existing);
      std::system(
          ("chmod +x \"" +
           (appBundle / "Contents/MacOS" / info.executableName).string() + "\"")
              .c_str());
    } else {
      std::cerr << "[!] Warning: Built executable not found at " << builtExe
                << ". Did CMake build successfully?\n";
    }

    if (!info.iconPath.empty() && fs::exists(info.iconPath)) {
      fs::copy_file(info.iconPath,
                    appBundle / "Contents/Resources/AppIcon.icns",
                    fs::copy_options::overwrite_existing);
    }
    createPlistBuddy(appBundle, info);
    std::cout << "[*] Info.plist generated.\n";
    std::cout << "[*] Pulling dependencies and fixing dylib paths...\n";
    std::string dylibCmd =
        "dylibbundler -od -b -x \"" +
        (appBundle / "Contents/MacOS" / info.executableName).string() +
        "\" -d \"" + (appBundle / "Contents/Frameworks").string() + "\"";
    std::system(dylibCmd.c_str());
    std::cout << "\n[+] Success! Your macOS app is ready.\n";
  }

  else if (doEdit) {
    if (workPath.extension() != ".app") {
      std::cerr << "Error: For -edit, the -path must point directly to a .app "
                   "file.\n";
      return 1;
    }

    fs::path plistPath = workPath / "Contents/Info.plist";
    if (!fs::exists(plistPath)) {
      std::cerr << "Error: Info.plist not found in the provided .app bundle.\n";
      return 1;
    }

    std::cout << "\n=== App Editor ===\nEditing: " << workPath.filename()
              << "\n";
    std::string key, val;

    if (useGui) {
      key = askUserGUI("Which Plist key to edit? (e.g., CFBundleVersion)");
      val = askUserGUI("Enter new value for " + key);
    } else {
      key = askUser("Which Plist key to edit? (e.g., CFBundleVersion)");
      val = askUser("Enter new value for " + key);
    }

    if (!key.empty() && !val.empty()) {
      editPlist(plistPath, key, val);
      std::cout << "[+] Updated " << key << " successfully.\n";
    }
  } else {
    std::cout << "Usage:\n";
    std::cout << "  Create & Build: ./mac-app-creator -create -build -path "
                 "<path_to_cmake_project>\n";
    std::cout
        << "  Edit Existing:  ./mac-app-creator -edit -path <path_to.app>\n";
    std::cout << "  Add '-gui' to any command to use graphical popups instead "
                 "of terminal prompts.\n";
  }

  return 0;
}