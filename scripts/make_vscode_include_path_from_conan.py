import os, sys, re, json

CONAN_PATH = os.path.expanduser("~/.conan/data").replace("\\", "/")

if sys.platform == "win32":
    CONAN_ROOT_PATH = "C:/.conan"
else:
    CONAN_ROOT_PATH = ""
    exit(1)

def get_conan_file(directory : str) -> str:
    for file in os.listdir(directory):
        if file == "conanfile.txt":
            return f"{directory}/{file}"

    return None

def get_conan_packages(file : str) -> list[str]:
    if not os.path.exists(file):
        exit(1)

    package_pattern = re.compile(r".*\/[0-9\.]*")
    packages = []

    with open(file, "r") as file:
        for line in file.readlines():
            if re.search(package_pattern, line):
                packages.append(line.replace("\n", ""))

    return packages

def find_pkg_include_path(package : str) -> str:
    if "@" in package:
        package = package.split("@")[0]
        
    package_name, package_version = package.split("/")
    
    package_dir = f"{CONAN_PATH}/{package_name}/{package_version}"

    for root, dirs, files in os.walk(package_dir):
        for dir in dirs:
            if dir == "include":
                package_include_dir = os.path.join(root, dir).replace("\\", "/")
                return package_include_dir

def find_missing_packages_include_path() -> list[str]:
    missing_packages_include_paths = []

    for root, dirs, files in os.walk(CONAN_ROOT_PATH):
        for dir in dirs:
            if dir == "include":
                new_include_path = os.path.join(root, dir).replace("\\", "/")
                missing_packages_include_paths.append(new_include_path)

    return missing_packages_include_paths
    
def main():
    if len(sys.path) < 2:
        exit(1)

    current_dir = os.getcwd()

    if not os.path.exists(current_dir):
        exit(1)

    conan_file = get_conan_file(current_dir)
    conan_packages = get_conan_packages(conan_file)
    conan_packages_include_paths = []

    for package in conan_packages:
        package_path = find_pkg_include_path(package)

        if package_path is not None:
            conan_packages_include_paths.append(package_path)

    missing_packages_include_paths = find_missing_packages_include_path()
    conan_packages_include_paths.extend(missing_packages_include_paths)

    vs_code_cpp_config_file = f"{os.getcwd()}/.vscode/c_cpp_properties.json"

    if not os.path.exists(vs_code_cpp_config_file):
        data = """{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${default}",
                "${workspaceFolder}/libOpenViewerUtils/include",
                "${workspaceFolder}/libOpenViewer/include",
                "${workspaceFolder}/libOpenViewerApp/include",
                "${workspaceFolder}/imgui/include"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE",
                "LOVU_BUILD_DLL"
            ],
            "windowsSdkVersion": "10.0.19041.0",
            "compilerPath": "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.29.30133/bin/Hostx64/x64/cl.exe",
            "intelliSenseMode": "windows-msvc-x64"
        }
    ],
    "version": 4
}"""
        jsonData = json.loads(data)

        includePath = jsonData["configurations"][0]["includePath"]
        includePath.extend(conan_packages_include_paths)
        jsonData["configurations"][0]["includePath"] = includePath

        with open(vs_code_cpp_config_file, "w") as file:
            json.dump(jsonData, file, indent=2)
    else:
        with open(vs_code_cpp_config_file, "r") as file:
            data = file.read()
            jsonData = json.loads(data)

        includePath = jsonData["configurations"][0]["includePath"]
        includePath.extend(conan_packages_include_paths)
        jsonData["configurations"][0]["includePath"] = includePath

        with open(vs_code_cpp_config_file, "w") as file:
            json.dump(jsonData, file, indent=2)

if __name__ == "__main__":
    main()