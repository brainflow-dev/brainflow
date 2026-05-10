import argparse
import shutil
import tempfile
import zipfile
from pathlib import Path


ANDROID_ABIS = ('arm64-v8a', 'armeabi-v7a', 'x86', 'x86_64')
REQUIRED_NATIVE_LIBS = ('libBoardController.so', 'libsimpleble-c.so')


def copy_file(src, dst):
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def copy_android_classes_jar(src, dst):
    dst.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(src, 'r') as src_zip:
        with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as dst_zip:
            for info in src_zip.infolist():
                name = info.filename
                if name == 'module-info.class' or name.endswith('/module-info.class'):
                    continue
                if name.startswith('META-INF/versions/'):
                    continue
                if name.startswith('com/sun/jna/platform/'):
                    continue
                dst_zip.writestr(info, src_zip.read(info.filename))


def write_manifest(path, package_name, min_sdk):
    path.write_text(
        '''<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android" package="{package_name}">
    <uses-sdk android:minSdkVersion="{min_sdk}" />
    <uses-permission android:name="android.permission.BLUETOOTH" android:maxSdkVersion="30" />
    <uses-permission android:name="android.permission.BLUETOOTH_ADMIN" android:maxSdkVersion="30" />
    <uses-permission android:name="android.permission.BLUETOOTH_SCAN" android:usesPermissionFlags="neverForLocation" />
    <uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />
</manifest>
'''.format(package_name=package_name, min_sdk=min_sdk),
        encoding='utf-8')


def add_directory_to_zip(zip_file, directory):
    for path in sorted(directory.rglob('*')):
        if path.is_file():
            zip_file.write(path, path.relative_to(directory).as_posix())


def parse_args():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description='Package BrainFlow Android AAR.')
    parser.add_argument('--classes-jar', type=Path,
                        default=root / 'java_package' / 'brainflow' / 'target' /
                        'brainflow-jar-with-dependencies.jar')
    parser.add_argument('--bridge-jar', type=Path,
                        default=root / 'tools' / 'simpleble-bridge.jar')
    parser.add_argument('--jni-libs', type=Path,
                        default=root / 'tools' / 'jniLibs')
    parser.add_argument('--output', type=Path,
                        default=root / 'tools' / 'brainflow-android.aar')
    parser.add_argument('--package-name', default='org.brainflow')
    parser.add_argument('--min-sdk', default='31')
    parser.add_argument('--allow-missing-abis', action='store_true',
                        help='Package only ABI folders present under jniLibs.')
    return parser.parse_args()


def main():
    args = parse_args()
    classes_jar = args.classes_jar.resolve()
    bridge_jar = args.bridge_jar.resolve()
    jni_libs = args.jni_libs.resolve()
    output = args.output.resolve()

    if not classes_jar.is_file():
        raise RuntimeError('classes jar not found: %s' % classes_jar)
    if not bridge_jar.is_file():
        raise RuntimeError('SimpleBLE bridge jar not found: %s' % bridge_jar)
    if not jni_libs.is_dir():
        raise RuntimeError('jniLibs directory not found: %s' % jni_libs)

    missing_abis = []
    packaged_abis = []
    for abi in ANDROID_ABIS:
        abi_dir = jni_libs / abi
        if not abi_dir.is_dir():
            missing_abis.append(abi)
            continue
        missing_libs = [lib for lib in REQUIRED_NATIVE_LIBS if not (abi_dir / lib).is_file()]
        if missing_libs:
            if args.allow_missing_abis:
                missing_abis.append(abi)
                continue
            raise RuntimeError('ABI %s is missing required libraries: %s' %
                               (abi, ', '.join(missing_libs)))
        packaged_abis.append(abi)

    if not packaged_abis:
        raise RuntimeError('no Android ABI directories found in %s' % jni_libs)
    if missing_abis and not args.allow_missing_abis:
        raise RuntimeError('missing Android ABI directories: %s' %
                           ', '.join(missing_abis))

    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp_dir_name:
        tmp_dir = Path(tmp_dir_name)
        write_manifest(tmp_dir / 'AndroidManifest.xml', args.package_name, args.min_sdk)
        copy_android_classes_jar(classes_jar, tmp_dir / 'classes.jar')
        copy_file(bridge_jar, tmp_dir / 'libs' / 'simpleble-bridge.jar')
        (tmp_dir / 'R.txt').write_text('', encoding='utf-8')

        for abi in packaged_abis:
            for native_lib in sorted((jni_libs / abi).glob('*.so')):
                copy_file(native_lib, tmp_dir / 'jni' / abi / native_lib.name)

        with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as zip_file:
            add_directory_to_zip(zip_file, tmp_dir)

    print('Packaged %s with ABIs: %s' % (output, ', '.join(packaged_abis)), flush=True)


if __name__ == '__main__':
    main()
