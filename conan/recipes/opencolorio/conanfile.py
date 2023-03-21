from conan.tools.microsoft import is_msvc
from conans import ConanFile, CMake, tools
import functools
import os
import glob

required_conan_version = ">=1.45.0"


class OpenColorIOConan(ConanFile):
    name = "opencolorio"

    version = "2.1.0"

    description = "A color management framework for visual effects and animation."
    license = "BSD-3-Clause"
    homepage = "https://opencolorio.org/"
    url = "https://github.com/conan-io/conan-center-index"
    
    topics = ("colors", "visual", "effects", "animation")

    settings = "os", "arch", "compiler", "build_type"
    
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "use_sse": [True, False],
    }
    
    default_options = {
        "shared": True,
        "fPIC": True,
        "use_sse": True,
    }

    generators = "cmake", "cmake_find_package"

    @property
    def _source_subfolder(self):
        return "opencolorio_subfolder"

    @property
    def _build_subfolder(self):
        return "opencolorio_build"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
        if self.settings.arch not in ["x86", "x86_64"]:
            del self.options.use_sse

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def requirements(self):
        self.requires("expat/2.4.8")
        self.requires("openexr/3.1.5")
        self.requires("yaml-cpp/0.7.0")
        self.requires("pystring/1.1.3")

        # for tools only
        self.requires("lcms/2.13.1")

    def validate(self):
        if self.settings.compiler.get_safe("cppstd"):
            tools.check_min_cppstd(self, 11)

    def _patch_expat(self):
        # replace expat::expat with $EXPAT_LIBRARIES in the src/OpenColorIO/CMakeLists.txt
        files = glob.glob("{0}/**/src/OpenColorIO/CMakeLists.txt".format(self._source_subfolder))

        if files:
            cmake_file = files[0]

            with open(cmake_file, "r") as file:
                content = file.read()

            content = content.replace("expat::expat", "${EXPAT_LIBRARIES}")

            with open(cmake_file, "w") as file:
                file.write(content)

            print("Patched expat library")

    def _patch_includes(self):
        # Adds ${pystring_INCLUDE_DIR} and ${EXPAT_INCLUDE_DIR} in the src/OpenColorIO/CMakeLists.txt
        files = glob.glob("{0}/**/CMakeLists.txt".format(self._source_subfolder))

        if files:
            cmake_file = files[0]

            with open(cmake_file, "r") as file:
                content = file.read()

            content = content.replace("include(FindExtPackages)", 
"""
include(FindExtPackages)

message("PYSTRING INCLUDE DIRS : ${pystring_INCLUDE_DIRS}")
message("EXPAT INCLUDE DIRS : ${EXPAT_INCLUDE_DIRS}")

include_directories(
        ${pystring_INCLUDE_DIRS}
        ${EXPAT_INCLUDE_DIRS}
)
"""
)

            with open(cmake_file, "w") as file:
                file.write(content)

            print("Patched includes")

    def _patch_pystring(self):
        # change includes #include "pystring/pystring.h" to #include "pystring.h"
        files = [
            "{0}/OpenColorIO-2.1.2/src/OpenColorIO/Context.cpp".format(self._source_subfolder),
            "{0}/OpenColorIO-2.1.2/src/OpenColorIO/OCIOYaml.cpp".format(self._source_subfolder),
            "{0}/OpenColorIO-2.1.2/src/OpenColorIO/Op.cpp".format(self._source_subfolder),
            "{0}/OpenColorIO-2.1.2/src/OpenColorIO/PathUtils.cpp".format(self._source_subfolder),
            "{0}/OpenColorIO-2.1.2/src/OpenColorIO/fileformats/FileFormatCTF.cpp".format(self._source_subfolder),
            "{0}/OpenColorIO-2.1.2/src/OpenColorIO/fileformats/FileFormatICC.cpp".format(self._source_subfolder),
            "{0}/OpenColorIO-2.1.2/src/OpenColorIO/fileformats/FileFormatIridasLook.cpp".format(self._source_subfolder),
            "{0}/OpenColorIO-2.1.2/src/OpenColorIO/transforms/FileTransform.cpp".format(self._source_subfolder),
            "{0}/OpenColorIO-2.1.2/src/OpenColorIO/fileformats/FileFormatDiscreet1DL.cpp".format(self._source_subfolder),
        ]

        for file in files:
            with open(file, "r") as f:
                content = f.read()

            content = content.replace("#include \"pystring/pystring.h\"", "#include \"pystring.h\"")

            with open(file, "w") as f:
                f.write(content)

    def _patch_missing_strlen(self):
        file = "{0}/OpenColorIO-2.1.2/src/OpenColorIO/FileRules.cpp".format(self._source_subfolder)

        with open(file, "r") as f:
            content = f.read()

        content = content.replace("#include <map>", "#include <cstring>\n#include <map>")

        with open(file, "w") as f:
            f.write(content)

    def source(self):
        tools.get("https://github.com/AcademySoftwareFoundation/OpenColorIO/archive/refs/tags/v2.1.2.zip", destination=self._source_subfolder)

        self._patch_expat()
        self._patch_includes()
        self._patch_pystring()
        self._patch_missing_strlen()

    @functools.lru_cache(1)
    def _configure_cmake(self):
        cmake = CMake(self)

        cmake.definitions["CMAKE_MODULE_PATH"] = self.install_folder.replace("\\", "/")

        if tools.Version(self.version) >= "2.1.0":
            cmake.definitions["OCIO_BUILD_PYTHON"] = False
        else:
            cmake.definitions["OCIO_BUILD_SHARED"] = self.options.shared
            cmake.definitions["OCIO_BUILD_STATIC"] = not self.options.shared
            cmake.definitions["OCIO_BUILD_PYGLUE"] = False

            cmake.definitions["USE_EXTERNAL_YAML"] = True
            cmake.definitions["USE_EXTERNAL_TINYXML"] = True
            cmake.definitions["USE_EXTERNAL_LCMS"] = True

        cmake.definitions["OCIO_USE_SSE"] = self.options.get_safe("use_sse", False)

        # openexr 2.x provides Half library
        cmake.definitions["OCIO_USE_OPENEXR_HALF"] = True

        cmake.definitions["OCIO_BUILD_APPS"] = False
        cmake.definitions["OCIO_BUILD_DOCS"] = False
        cmake.definitions["OCIO_BUILD_TESTS"] = False
        cmake.definitions["OCIO_BUILD_GPU_TESTS"] = False
        cmake.definitions["OCIO_USE_BOOST_PTR"] = False

        # avoid downloading dependencies
        cmake.definitions["OCIO_INSTALL_EXT_PACKAGE"] = "NONE"

        if is_msvc(self) and not self.options.shared:
            # define any value because ifndef is used
            cmake.definitions["OpenColorIO_SKIP_IMPORTS"] = True

        ocio_folder = os.listdir(self._source_subfolder)[0]

        cmake.configure(source_folder="{0}/{1}".format(self._source_subfolder, ocio_folder), 
                        build_folder=self._build_subfolder)
        return cmake

    def build(self):
        cm = self._configure_cmake()
        cm.build()

    def package(self):
        cm = self._configure_cmake()
        cm.install()

        if not self.options.shared:
            self.copy("*", src=os.path.join(self.package_folder,
                      "lib", "static"), dst="lib")
            tools.rmdir(os.path.join(self.package_folder, "lib", "static"))

        tools.rmdir(os.path.join(self.package_folder, "cmake"))
        tools.rmdir(os.path.join(self.package_folder, "lib", "pkgconfig"))
        tools.rmdir(os.path.join(self.package_folder, "lib", "cmake"))
        tools.rmdir(os.path.join(self.package_folder, "share"))
        # nop for 2.x
        tools.remove_files_by_mask(self.package_folder, "OpenColorIOConfig*.cmake")

        tools.remove_files_by_mask(os.path.join(self.package_folder, "bin"), "*.pdb")

        self.copy("LICENSE", src=self._source_subfolder, dst="licenses")

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "OpenColorIO")
        self.cpp_info.set_property("cmake_target_name", "OpenColorIO::OpenColorIO")
        self.cpp_info.set_property("pkg_config_name", "OpenColorIO")

        self.cpp_info.libs = ["OpenColorIO"]

        if tools.Version(self.version) < "2.1.0":
            if not self.options.shared:
                self.cpp_info.defines.append("OpenColorIO_STATIC")

        if self.settings.os == "Macos":
            self.cpp_info.frameworks.extend(["Foundation", "IOKit", "ColorSync", "CoreGraphics"])

        if is_msvc(self) and not self.options.shared:
            self.cpp_info.defines.append("OpenColorIO_SKIP_IMPORTS")

        bin_path = os.path.join(self.package_folder, "bin")
        self.output.info("Appending PATH env var with: {}".format(bin_path))
        self.env_info.PATH.append(bin_path)

        # TODO: to remove in conan v2 once cmake_find_package_* & pkg_config generators removed
        self.cpp_info.names["cmake_find_package"] = "OpenColorIO"
        self.cpp_info.names["cmake_find_package_multi"] = "OpenColorIO"
        self.cpp_info.names["pkg_config"] = "OpenColorIO"

