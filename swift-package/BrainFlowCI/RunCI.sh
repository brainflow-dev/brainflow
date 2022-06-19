
xcodebuild \
  -project BrainFlowCI.xcodeproj \
  -scheme BrainFlowCI \
  -sdk macosx12.3 \
  -destination 'platform=OS X' \
  test
