
export BRAINFLOW_VERSION=4.9.0
export GITHUB_WORKSPACE=/Users/scottmiller/git/brainflow-dev

mkdir -p $GITHUB_WORKSPACE/installed/macOS
cd $GITHUB_WORKSPACE/tools
cmake -G Ninja -DBRAINFLOW_VERSION=$BRAINFLOW_VERSION -DBUILD_BLUETOOTH=ON -DBUILD_TESTS=ON -DWARNINGS_AS_ERRORS=ON \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 -DCMAKE_INSTALL_PREFIX=$GITHUB_WORKSPACE/installed/macOS \
  -DCMAKE_BUILD_TYPE=Release ..
ninja
ninja install

cd $GITHUB_WORKSPACE/swift-package/BrainFlow

xcodebuild -create-xcframework -library $GITHUB_WORKSPACE/installed/macOS/lib/libBoardController.dylib \
  -headers $GITHUB_WORKSPACE/installed/macOS/inc -output BoardController.xcframework
xcodebuild -create-xcframework -library $GITHUB_WORKSPACE/installed/macOS/lib/libBrainBitLib.dylib  -output BrainBitLib.xcframework
xcodebuild -create-xcframework -library $GITHUB_WORKSPACE/installed/macOS/lib/libDataHandler.dylib  -output DataHandler.xcframework
xcodebuild -create-xcframework -library $GITHUB_WORKSPACE/installed/macOS/lib/libMLModule.dylib  -output MLModule.xcframework

