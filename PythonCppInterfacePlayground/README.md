# PythonCppInterfacePlayground

An Initial Proof-of-concept Python wrapper for Polysync Core.  This is work in progress.  I am not sure yet how the images should be exposed in the python layer.  Please feel free to explore and test this example.

## Installation

```
$ cd PolysyncNode/
$ cmake .
$ make
$ cd ..
```
Test Run

1. Start Polysync Publisher (From Polysync CPP Example):
```
$~/polysync/PolySync-Core-CPP-Examples/PublishSubscribe/Publish/build$ ls
CMakeCache.txt  CMakeFiles  cmake_install.cmake  Makefile  ospl-info.log  polysync-publisher-cpp
$~/polysync/PolySync-Core-CPP-Examples/PublishSubscribe/Publish/build$ ./polysync-publisher-cpp 
2017-03-11 15:24:14.03s DEBUG  [polysync-publish-cpp-] - build version 2.0.10-1487272519
2017-03-11 15:24:15.03s DEBUG  [polysync-publish-cpp-281475644456168] - created participant - GUID: 0x0001000027CCFCE8 d(281475644456168)
2017-03-11 15:24:15.03s DEBUG  [polysync-publish-cpp-281475644456168] - message types visible to this node: 47
2017-03-11 15:24:15.03s DEBUG  [polysync-publish-cpp-281475644456168] - transition to state: INIT - timestamp: 1489263855942849
2017-03-11 15:24:15.03s DEBUG  [polysync-publish-cpp-281475644456168] - transition to state: OK - timestamp: 1489263855944163
```

2. Start testnode.py
```
$~/SDCND/thunderhill-racing/PythonCppInterfacePlayground$ python testnode.py 
2017-03-11 15:24:03.03s DEBUG  [polysync-python-test-client-cpp-] - build version 2.0.10-1487272519
2017-03-11 15:24:04.03s DEBUG  [polysync-python-test-client-cpp-281475387669553] - created participant - GUID: 0x00010000187EBC31 d(281475387669553)
2017-03-11 15:24:04.03s DEBUG  [polysync-python-test-client-cpp-281475387669553] - message types visible to this node: 47
2017-03-11 15:24:04.03s DEBUG  [polysync-python-test-client-cpp-281475387669553] - transition to state: INIT - timestamp: 1489263844853901
2017-03-11 15:24:04.03s DEBUG  [polysync-python-test-client-cpp-281475387669553] - transition to state: OK - timestamp: 1489263844858501
lat, lon: 45.515289 -122.654355
yaw: 0.0
heading: 0.0
velocity(x,y,z): 0.0 0.0 0.0
lat, lon: 45.515289 -122.654355
yaw: 0.0
heading: 0.0
velocity(x,y,z): 0.0 0.0 0.0
lat, lon: 45.515289 -122.654355
yaw: 0.0
heading: 0.0
velocity(x,y,z): 0.0 0.0 0.0
lat, lon: 45.515289 -122.654355
yaw: 0.0
heading: 0.0
velocity(x,y,z): 0.0 0.0 0.0
lat, lon: 45.515289 -122.654355
yaw: 0.0
heading: 0.0
velocity(x,y,z): 0.0 0.0 0.0
lat, lon: 45.515289 -122.654355
yaw: 0.0
heading: 0.0
velocity(x,y,z): 0.0 0.0 0.0
...
```


