import os
import glob
import subprocess
import shutil
import sys

def clean_prev_build(build_directory):
    print(F"Cleaning directory {build_directory}")
    if os.path.exists(build_directory):
        shutil.rmtree(build_directory)
    os.makedirs(build_directory)

def configure_opencv(opencv_dir, slideio_dir, build_dir, python_dir):
    print("Start configuration")
    python_library = os.path.join(python_dir,"libs","python3.lib").replace("\\","\\\\")
    python_include = os.path.join(python_dir,"include").replace("\\","\\\\")
    python_exe = os.path.join(python_dir,"python.exe").replace("\\","\\\\")
    python_packages = os.path.join(python_dir, "Lib", "site-packages").replace("\\","\\\\")
    python_numpy = os.path.join(python_dir, "Lib", "site-packages","numpy", "core","include")
    build_dir_param = build_dir.replace("\\","\\\\")
    opencv_dir_param = opencv_dir.replace("\\","\\\\")
    slideio_dir_param = slideio_dir.replace("\\","/")
    cmake_props = {
        "OPENCV_EXTRA_MODULES_PATH":slideio_dir_param,
        "PYTHON3_LIBRARY": python_library,
        "PYTHON3_LIBRARY_DEBUG": python_library,
        "PYTHON3_INCLUDE_DIRS": python_include,
        "PYTHON3_EXECUTABLE": python_exe,
        "PYTHON3_PACKAGES_PATH":python_packages,
        "PYTHON3_NUMPY_INCLUDE_DIRS":python_numpy,
        "BUILD_opencv_python3":"ON",
        "HAVE_opencv_python3":"ON",
        "BUILD_opencv_python_bindings_generator":"ON",
        "BUILD_opencv_python_tests":"ON",
        "OPENCV_PYTHON_SKIP_DETECTION":"OFF",
        "BUILD_opencv_python2":"OFF",
        "BUILD_JAVA":"OFF",
        "BUILD_opencv_java_bindings_generator":"OFF",
        "BUILD_opencv_js":"OFF",
        "BUILD_TESTS":"ON",
        "BUILD_PERF_TESTS":"ON",
        "BUILD_opencv_ts":"ON",
        "INSTALL_TESTS":"ON",
        "BUILD_DOCS":"OFF",
        "BUILD_EXAMPLES":"OFF",
        "INSTALL_C_EXAMPLES":"OFF",
        "INSTALL_PYTHON_EXAMPLES":"ON",
        "OPENCV_FORCE_3RDPARTY_BUILD":"OFF",
        "BUILD_IPP_IW":"OFF",
        "BUILD_WITH_DYNAMIC_IPP":"OFF",
        "WITH_IPP":"OFF",
        "BUILD_ITT":"OFF",
        "WITH_ITT":"OFF",
        "BUILD_JASPER":"ON",
        "WITH_JASPER":"ON",
        "BUILD_JPEG":"ON",
        "WITH_JPEG":"ON",
        "WITH_TIFF":"ON",
        "BUILD_TIFF":"ON"
        }
    generator = '"Visual Studio 15 2017"'
    parameters = ""
    for pname, pvalue in cmake_props.items():
        parameters += F" -D{pname}=\"{pvalue}\""
    cmd = F"cmake.exe -G {generator} -A x64 -DBUILD_opencv_legacy=OFF {parameters} -S {opencv_dir_param} -B {build_dir_param}"
    print(F"Execute command {cmd}")
    subprocess.call(cmd)

def collect_profiles(profile_dir):
    profiles = []
    for root, dirs, files in os.walk(profile_dir):
        files = glob.glob(os.path.join(root,'*'))
        for f in files :
            profiles.append(os.path.abspath(f))
    return profiles

def process_conan_profile(profile, trg_dir, conan_file):
    command = ['conan','install',
        '-pr',profile,
        '-if',trg_dir,
        '-g', 'visual_studio_multi',
        conan_file]
    print(command)
    subprocess.check_call(command)

def configure_conan(slideio_dir, build_dir):
    # copy conan file to the location of vs project
    src_conan_file_path = os.path.join(slideio_dir, "slideio","conanfile.txt")
    trg_conan_dir_path = os.path.join(build_directory, "modules","slideio")
    trg_conan_file_path = os.path.join(trg_conan_dir_path, "conanfile.txt")
    shutil.copyfile(src_conan_file_path, trg_conan_file_path)
    # collect paths to conan profile files
    conan_profile_dir_path = os.path.join(slideio_dir,"slideio","conan")
    profiles = collect_profiles(conan_profile_dir_path)
    print("Process conan profiles.")
    for profile in profiles:
        print(F"Profile:{profile}")
        process_conan_profile(profile, trg_conan_dir_path, trg_conan_file_path)

if __name__ == "__main__":
    python_dir = r"C:\Users\Stas\AppData\Local\conda\conda\envs\cvbuild"
    if len(sys.argv)>1:
        python_dir = sys.argv[1]
    root_directory = os.path.dirname(os.getcwd())
    build_directory = os.path.join(root_directory, "build")
    opencv_directory = os.path.join(root_directory, "opencv")
    slideio_directory = os.path.join(root_directory, "opencv_slideio","modules")
    print("----------Installattion of slideio-----------------")
    print(F"Root directory: {root_directory}")
    print(F"Opencv directory: {opencv_directory}")
    print(F"Slideio directory: {slideio_directory}")
    clean_prev_build(build_directory)
    configure_opencv(opencv_directory, slideio_directory, build_directory, python_dir)
    configure_conan(slideio_directory, build_directory)