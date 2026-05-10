import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


ANDROID_ABIS = ('arm64-v8a', 'armeabi-v7a', 'x86', 'x86_64')
DEFAULT_ANDROID_API_LEVEL = 'android-31'


def run_command(cmd, cwd):
    print('Running command: %s' % ' '.join(str(item) for item in cmd), flush=True)
    subprocess.check_call([str(item) for item in cmd], cwd=str(cwd))


def resolve_executable(name):
    resolved = shutil.which(name)
    if resolved is None:
        raise RuntimeError('required executable not found in PATH: %s' % name)
    return resolved


def resolve_android_sdk_root(value):
    candidates = [
        value,
        os.environ.get('ANDROID_SDK_ROOT'),
        os.environ.get('ANDROID_HOME'),
    ]
    for candidate in candidates:
        if candidate:
            path = Path(candidate).resolve()
            if path.is_dir():
                return path
    raise RuntimeError('Android SDK not found. Set ANDROID_SDK_ROOT or pass --android-sdk-root.')


def resolve_android_jar(android_sdk_root, android_api_level):
    requested = android_sdk_root / 'platforms' / android_api_level / 'android.jar'
    if requested.is_file():
        return requested, android_api_level

    platforms = android_sdk_root / 'platforms'
    available = sorted(
        [path for path in platforms.glob('android-*') if (path / 'android.jar').is_file()],
        key=lambda path: int(path.name.split('-')[1]) if path.name.split('-')[1].isdigit() else -1)
    if available:
        fallback = available[-1]
        print('Requested %s was not found, using %s for bridge compilation.' %
              (android_api_level, fallback.name), flush=True)
        return fallback / 'android.jar', fallback.name

    raise RuntimeError('No Android platform android.jar found under %s' % platforms)


def resolve_ndk_root(android_sdk_root, value):
    candidates = [
        value,
        os.environ.get('ANDROID_NDK_ROOT'),
        os.environ.get('ANDROID_NDK_HOME'),
        os.environ.get('ANDROID_NDK'),
        android_sdk_root / 'ndk' / '25.1.8937393',
    ]
    ndk_dir = android_sdk_root / 'ndk'
    if ndk_dir.is_dir():
        versions = sorted([path for path in ndk_dir.iterdir() if path.is_dir()])
        candidates.extend(reversed(versions))

    for candidate in candidates:
        if candidate:
            path = Path(candidate).resolve()
            if (path / 'build' / 'cmake' / 'android.toolchain.cmake').is_file():
                return path
    raise RuntimeError('Android NDK not found. Set ANDROID_NDK_ROOT or pass --ndk-root.')


def resolve_ninja(root):
    path = shutil.which('ninja')
    if path is not None:
        return Path(path)
    local_ninja = root / 'tools' / ('ninja.exe' if os.name == 'nt' else 'ninja')
    if local_ninja.is_file():
        return local_ninja
    raise RuntimeError('Ninja not found in PATH or tools directory.')


def build_native(args, root, android_sdk_root):
    ndk_root = resolve_ndk_root(android_sdk_root, args.ndk_root)
    ninja = resolve_ninja(root)
    toolchain = ndk_root / 'build' / 'cmake' / 'android.toolchain.cmake'
    build_root = args.native_build_root.resolve()

    for abi in args.abis:
        build_dir = build_root / abi
        build_dir.mkdir(parents=True, exist_ok=True)

        cmake_cmd = [
            resolve_executable('cmake'),
            '-G', 'Ninja',
            '-S', root,
            '-B', build_dir,
            '-DCMAKE_MAKE_PROGRAM=%s' % ninja,
            '-DCMAKE_TOOLCHAIN_FILE=%s' % toolchain,
            '-DANDROID_ABI=%s' % abi,
            '-DCMAKE_BUILD_TYPE=Release',
            '-DANDROID_NATIVE_API_LEVEL=%s' % args.android_api_level,
            '-DBUILD_SYNCHRONI_SDK=OFF',
        ]
        if args.use_libftdi:
            cmake_cmd.append('-DUSE_LIBFTDI=ON')
        if args.cmake_find_root_path:
            cmake_cmd.append('-DCMAKE_FIND_ROOT_PATH=%s' % args.cmake_find_root_path)
        for define in args.cmake_define:
            cmake_cmd.append('-D%s' % define)

        run_command(cmake_cmd, root)
        run_command(
            [resolve_executable('cmake'), '--build', build_dir, '--parallel', str(args.jobs)], root)


def build_java(root):
    run_command(
        [resolve_executable('mvn'), '-q', '-f', root / 'java_package' / 'brainflow' / 'pom.xml',
         '-Pandroid', '-DskipTests', '-Dmaven.compiler.source=8', '-Dmaven.compiler.target=8',
         'clean', 'package'],
        root)


def build_simpleble_bridge(root, android_jar):
    source_root = root / 'third_party' / 'SimpleBLE' / 'simpledroidbridge' / 'src' / 'main' / 'java'
    source_files = sorted(source_root.rglob('*.java'))
    if not source_files:
        raise RuntimeError('No SimpleBLE Android bridge Java sources found under %s' % source_root)

    classes_dir = root / 'tools' / 'simpleble-bridge-classes'
    sources_file = root / 'tools' / 'simpleble-bridge-sources.txt'
    bridge_jar = root / 'tools' / 'simpleble-bridge.jar'

    if classes_dir.exists():
        shutil.rmtree(classes_dir)
    classes_dir.mkdir(parents=True)
    sources_file.write_text(
        '\n'.join(str(path) for path in source_files) + '\n', encoding='utf-8')

    run_command([
        resolve_executable('javac'), '-source', '8', '-target', '8', '-classpath', android_jar,
        '-d', classes_dir, '@%s' % sources_file
    ], root)
    run_command([resolve_executable('jar'), 'cf', bridge_jar, '-C', classes_dir, '.'], root)


def package_aar(args, root):
    cmd = [
        sys.executable,
        root / 'tools' / 'package_android_aar.py',
        '--output',
        args.output.resolve(),
        '--min-sdk',
        args.min_sdk,
    ]
    if args.allow_missing_abis:
        cmd.append('--allow-missing-abis')
    run_command(cmd, root)


def parse_args():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description='Build BrainFlow Android AAR.')
    parser.add_argument('--android-sdk-root', type=Path)
    parser.add_argument('--android-api-level', default=os.environ.get(
        'ANDROID_API_LEVEL', DEFAULT_ANDROID_API_LEVEL))
    parser.add_argument('--min-sdk', default='31')
    parser.add_argument('--output', type=Path, default=root / 'tools' / 'brainflow-android.aar')
    parser.add_argument('--allow-missing-abis', action='store_true')
    parser.add_argument('--build-native', action='store_true',
                        help='Build native Android libraries before packaging.')
    parser.add_argument('--abis', nargs='+', choices=ANDROID_ABIS, default=list(ANDROID_ABIS))
    parser.add_argument('--native-build-root', type=Path, default=root / 'build_android_aar')
    parser.add_argument('--ndk-root', type=Path)
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--use-libftdi', action='store_true')
    parser.add_argument('--cmake-find-root-path')
    parser.add_argument('--cmake-define', action='append', default=[],
                        help='Extra CMake definition without leading -D, for example FOO=ON.')
    return parser.parse_args()


def main():
    args = parse_args()
    root = Path(__file__).resolve().parents[1]
    android_sdk_root = resolve_android_sdk_root(args.android_sdk_root)
    android_jar, android_platform = resolve_android_jar(android_sdk_root, args.android_api_level)
    print('Using Android SDK: %s' % android_sdk_root, flush=True)
    print('Using Android platform for bridge: %s' % android_platform, flush=True)

    if args.build_native:
        build_native(args, root, android_sdk_root)
    build_java(root)
    build_simpleble_bridge(root, android_jar)
    package_aar(args, root)


if __name__ == '__main__':
    main()
