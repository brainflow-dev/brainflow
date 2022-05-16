
cd ../tools
python3 build.py
cd ../swift-package
xcodebuild -create-xcframework -library ../installed/lib/libBoardController.dylib \
  -headers ../installed/inc -output BoardController.xcframework
xcodebuild -create-xcframework -library ../installed/lib/libBrainBitLib.dylib  -output BrainBitLib.xcframework
xcodebuild -create-xcframework -library ../installed/lib/libDataHandler.dylib  -output DataHandler.xcframework
xcodebuild -create-xcframework -library ../installed/lib/libMLModule.dylib  -output MLModule.xcframework

