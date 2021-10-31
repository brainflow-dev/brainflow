import argparse
import platform
import subprocess
import multiprocessing
import os
from shutil import rmtree


def run_command(cmd, cwd=None):
    print('Running command: %s' % (' '.join(cmd)))
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=cwd)
    while True:
        p.stdout.flush()
        line = p.stdout.read(1)
        if line:
            print(line.decode('utf-8', 'ignore'), end='')
        else:
            if p.poll() != None:
                break
    if p.returncode != 0:
        raise ValueError('Process finished with error code %d' % p.returncode)


class Generator:

    def __init__(self, priority):
        self.priority = priority

    def get_generator(self):
        raise NotImplementedError

    def get_arch(self):
        raise NotImplementedError

    def __lt__(self, other):
        return self.priority < other.priority

    def __gt__(self, other):
        return self.priority > other.priority

    def __le__(self, other):
        return self.priority <= other.priority

    def __ge__(self, other):
        return self.priority >= other.priority

class VS2019(Generator):

    def __init__(self):
        super(VS2019, self).__init__(10)

    def get_generator(self):
        return 'Visual Studio 16 2019'

    def get_arch(self):
        return 'x64'


class VS2017(Generator):

    def __init__(self):
        super(VS2017, self).__init__(5)

    def get_generator(self):
        return 'Visual Studio 15 2017 Win64'

    def get_arch(self):
        return None


def get_win_generators():
    result = list()
    try:
        output = subprocess.check_output(['C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe', '-property', 'displayName'])
        output = output.decode('utf-8', 'ignore')
        print(output)
        if '2019' in output:
            result.append(VS2019())
        if '2017' in output:
            result.append(VS2017())
    except BaseException:
        print('No Visual Studio Installations Found')
    return sorted(result, reverse=True)

def check_deps():
    try:
        subprocess.check_output(['cmake', '--version'])
    except BaseException as e:
        print('You need to install CMake first. Run: python -m pip install cmake')
        raise e

def prepare_args():
    parser = argparse.ArgumentParser()
    cur_folder = os.path.dirname(os.path.abspath(__file__))

    bluetooth_default = False
    ble_default = False
    if platform.system() == 'Windows':
        bluetooth_default = True
        parser.add_argument('--oymotion', dest='oymotion', action='store_true')
        parser.add_argument('--no-oymotion', dest='oymotion', action='store_false')
        parser.set_defaults(oymotion=False)
        parser.add_argument('--msvc-runtime', type=str, choices=['static', 'dynamic'], help='how to link MSVC runtime', required=False, default='static')
        generators = get_win_generators()
        if not generators:
            parser.add_argument('--generator', type=str, help='generator for CMake', required=True)
            parser.add_argument('--arch', type=str, help='arch for CMake', required=False)
        else:
            generator = generators[0]
            parser.add_argument('--generator', type=str, help='generator for CMake', required=False, default=generator.get_generator())
            if generator.get_arch() is not None:
                parser.add_argument('--arch', type=str, choices=['x64', 'Win32', 'ARM', 'ARM64'], help='arch for CMake', required=False, default=generator.get_arch())
            else:
                parser.add_argument('--arch', type=str, choices=['x64', 'Win32', 'ARM', 'ARM64'], help='arch for CMake', required=False)
        parser.add_argument('--cmake-system-version', type=str, help='system version for win', required=False, default='8.1')
    elif platform.system() == 'Darwin':
        macos_ver = platform.mac_ver()[0]
        versions = [int(x) for x in macos_ver.split('.')]
        if versions[0] >= 11:
            parser.add_argument('--cmake-osx-architectures', type=str, help='archs for osx', required=False, default='"arm64;x86_64"')
        else:
            parser.add_argument('--cmake-osx-architectures', type=str, help='archs for osx', required=False)
        parser.add_argument('--cmake-osx-deployment-target', type=str, help='min supported version of osx', required=False, default='10.15')
        parser.add_argument('--use-libftdi', action='store_true')
        try:
            output = subprocess.check_output(['ninja', '--version'])
            print(output)
            parser.add_argument('--generator', type=str, help='CMake generator', required=False, default='Ninja')
        except BaseException:
            parser.add_argument('--generator', type=str, help='CMake generator', required=False, default='')
            print('Ninja is a recommended generator for MacOS and is not found')
    else:
        parser.add_argument('--generator', type=str, help='CMake generator', required=False)
        parser.add_argument('--use-libftdi', action='store_true')
        parser.add_argument('--use-periphery', action='store_true')

    parser.add_argument('--build-dir', type=str, help='build folder', required=False, default=os.path.join(cur_folder, '..', 'build'))
    parser.add_argument('--cmake-install-prefix', type=str, help='installation folder, full path', required=False, default=os.path.join(cur_folder, '..', 'installed'))
    parser.add_argument('--use-openmp', action='store_true')
    parser.add_argument('--warnings-as-errors', action='store_true')
    parser.add_argument('--debug', action='store_true')
    parser.add_argument('--clear-build-dir', action='store_true')
    parser.add_argument('--num-jobs', type=int, help='num jobs to run in parallel', required=False, default=max(1, multiprocessing.cpu_count() // 2))
    parser.add_argument('--bluetooth', dest='bluetooth', action='store_true')
    parser.add_argument('--no-bluetooth', dest='bluetooth', action='store_false')
    parser.set_defaults(bluetooth=bluetooth_default)
    parser.add_argument('--ble', dest='ble', action='store_true')
    parser.add_argument('--no-ble', dest='ble', action='store_false')
    parser.set_defaults(ble=ble_default)
    args = parser.parse_args()
    return args

def config(args):
    if args.clear_build_dir:
        if os.path.exists(args.build_dir):
            rmtree(args.build_dir)
        if os.path.exists(args.cmake_install_prefix):
            rmtree(args.cmake_install_prefix)
    if not os.path.exists(args.build_dir):
        os.makedirs(args.build_dir)

    cur_folder = os.path.dirname(os.path.abspath(__file__))
    brainflow_root_folder = os.path.join(cur_folder, '..')
    cmd_config = list()
    cmd_config.append('cmake')
    cmd_config.append('-DCMAKE_INSTALL_PREFIX=%s' % args.cmake_install_prefix)
    if hasattr(args, 'cmake_system_version') and args.cmake_system_version:
        cmd_config.append('-DCMAKE_SYSTEM_VERSION=%s' % args.cmake_system_version)
    if hasattr(args, 'use_libftdi') and args.use_libftdi:
        cmd_config.append('-DUSE_LIBFTDI=ON')
    if hasattr(args, 'use_periphery') and args.use_periphery:
        cmd_config.append('-DUSE_PERIPHERY=ON')
    if args.warnings_as_errors:
        cmd_config.append('-DWARNINGS_AS_ERRORS=ON')
    if args.use_openmp:
        cmd_config.append('-DUSE_OPENMP=ON')
    if hasattr(args, 'oymotion') and args.oymotion:
        cmd_config.append('-DBUILD_OYMOTION_SDK=ON')
    if hasattr(args, 'cmake_osx_architecture') and args.cmake_osx_architecture:
        cmd_config.append('-DCMAKE_OSX_ARCHITECTURES=%s' % args.cmake_osx_architecture)
    if hasattr(args, 'cmake_osx_deployment_target') and args.cmake_osx_deployment_target:
        cmd_config.append('-DCMAKE_OSX_DEPLOYMENT_TARGET=%s' % args.cmake_osx_deployment_target)
    if hasattr(args, 'generator') and args.generator:
        cmd_config.extend(['-G', args.generator])
    if hasattr(args, 'arch') and args.arch:
        cmd_config.extend(['-A', args.arch])
    if hasattr(args, 'msvc_runtime'):
        cmd_config.append('-DMSVC_RUNTIME=%s' % (args.msvc_runtime))
    if platform.system() != 'Windows':
        if hasattr(args, 'debug') and args.debug:
            cmd_config.append('-DCMAKE_BUILD_TYPE=Debug')
        else:
            cmd_config.append('-DCMAKE_BUILD_TYPE=Release')
    if hasattr(args, 'bluetooth') and args.bluetooth:
        cmd_config.append('-DBUILD_BLUETOOTH=ON')
    if hasattr(args, 'ble') and args.ble:
        cmd_config.append('-DBUILD_BLE=ON')
    cmd_config.append(brainflow_root_folder)
    run_command(cmd_config, args.build_dir)

def build(args):
    if platform.system() == 'Windows':
        config = 'Release'
        if args.debug:
            config = 'Debug'
        cmd_build = ['cmake', '--build', '.', '--target', 'install', '--config', config, '-j', str(args.num_jobs), '--parallel', str(args.num_jobs)]
        run_command(cmd_build, cwd=args.build_dir)
    else:
        if hasattr(args, 'generator') and args.generator and args.generator.lower() == 'ninja':
            run_command(['ninja', '-j', str(args.num_jobs)], cwd=args.build_dir)
            run_command(['ninja', 'install'], cwd=args.build_dir)
        else:
            run_command(['make', '-j', str(args.num_jobs)], cwd=args.build_dir)
            run_command(['make', 'install'], cwd=args.build_dir)


def main():
    check_deps()
    args = prepare_args()
    config(args)
    build(args)


if __name__ == '__main__':
    main()
