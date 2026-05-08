# SimpleBLE Test

These are some random notes on how testing was set up. This will become more important once more components can be properly tested.


## Legacy CI Linux Test
```
  tests:
    runs-on: ubuntu-22.04
    steps:
    - name: Clone Repository
      uses: actions/checkout@v4
    - name: Install Dependencies
      env:
          DEBIAN_FRONTEND: noninteractive
      run: |
        sudo -H apt-get update -y
        sudo -H apt-get install -y dbus libdbus-1-dev python3-dev
    - name: Setup cmake
      uses: ./.github/actions/setup-cmake

    - name: Setup gtest
      uses: ./.github/actions/setup-gtest

    - name: Start DBus
      run: |
        echo "DBUS_SESSION_BUS_ADDRESS=$(dbus-daemon --config-file=/usr/share/dbus-1/session.conf --print-address --fork | cut -d, -f1)" >> $GITHUB_ENV

    - name: SimpleBLE Unit Tests
      run: |
        cmake -B $GITHUB_WORKSPACE/build_unit_simpleble -DCMAKE_BUILD_TYPE=Release -DGTEST_ROOT=$GITHUB_WORKSPACE/googletest/install -S $GITHUB_WORKSPACE/simpleble -DSIMPLEBLE_TEST=ON
        cmake --build $GITHUB_WORKSPACE/build_unit_simpleble --config Release --parallel 4
        $GITHUB_WORKSPACE/build_unit_simpleble/bin/simpleble_test
```

## Legacy CI MacOS Test