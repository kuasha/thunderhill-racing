# Clone repository

    $ git clone --recursive https://github.com/kuasha/thunderhill-racing

# Setup Instructions

## Install Bazel

See [full instructions here](https://bazel.build/versions/master/docs/install.html#2-add-bazel-distribution-uri-as-a-package-source-one-time-setup).

    $ echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
    $ curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
    $ sudo apt-get update && sudo apt-get install bazel

## Install numpy

    $ sudo apt install python3-pip
    $ pip3 install numpy

## Configure TensorFlow

    $ cd tensorflow
    $ ./configure

## Run the example code

This will take a long time the first time you build the TensorFlow libraries. Subsequent builds are much faster.

    $ bazel run //RecorderNode:RecorderNode

## Optional: Building TensorFlow with GPU / CUDA

This guide explains [how to build TensorFlow with GPU / CUDA support](https://alliseesolutions.wordpress.com/2016/09/08/install-gpu-tensorflow-from-sources-w-ubuntu-16-04-and-cuda-8-0-rc/).

```
sudo apt-get install openjdk-8-jdk git python-dev python3-dev python-numpy python3-numpy build-essential python-pip python3-pip python-virtualenv swig python-wheel libcurl3-dev
```

If you have CUDA and CuDNN you can skip installation (I skipped)

Do `./configure` with CUDA enabled. (specify python - I have used paths from tutorial, I have native python3 without any env/conda).

Choose default CUDA settings. Here is full ./configure outoput:

```
./configure 
Please specify the location of python. [Default is /usr/bin/python]: /usr/bin/python3.5
Please specify optimization flags to use during compilation when bazel option "--config=opt" is specified [Default is -march=native]: 
Do you wish to use jemalloc as the malloc implementation? [Y/n] 
jemalloc enabled
Do you wish to build TensorFlow with Google Cloud Platform support? [y/N] 
No Google Cloud Platform support will be enabled for TensorFlow
Do you wish to build TensorFlow with Hadoop File System support? [y/N] 
No Hadoop File System support will be enabled for TensorFlow
Do you wish to build TensorFlow with the XLA just-in-time compiler (experimental)? [y/N] 
No XLA support will be enabled for TensorFlow
Found possible Python library paths:
  /opt/ros/kinetic/lib/python2.7/dist-packages
  /home/karol/projects/udacity/Racing/thunderhill/ros/devel/lib/python2.7/dist-packages
  /home/karol/caffe/BUILD/install/python/
  /usr/lib/python3/dist-packages
  /usr/local/lib/python3.5/dist-packages
  /home/karol/projects/jakub/optistat/src
Please input the desired Python library path to use.  Default is [/opt/ros/kinetic/lib/python2.7/dist-packages]
 /usr/local/lib/python3.5/dist-packages
Do you wish to build TensorFlow with OpenCL support? [y/N] 
No OpenCL support will be enabled for TensorFlow
Do you wish to build TensorFlow with CUDA support? [y/N] y
CUDA support will be enabled for TensorFlow
Please specify which gcc should be used by nvcc as the host compiler. [Default is /usr/bin/gcc]: 
Please specify the CUDA SDK version you want to use, e.g. 7.0. [Leave empty to use system default]: 
Please specify the location where CUDA  toolkit is installed. Refer to README.md for more details. [Default is /usr/local/cuda]: 
Please specify the Cudnn version you want to use. [Leave empty to use system default]: 
Please specify the location where cuDNN  library is installed. Refer to README.md for more details. [Default is /usr/local/cuda]: 
Please specify a list of comma-separated Cuda compute capabilities you want to build with.
You can find the compute capability of your device at: https://developer.nvidia.com/cuda-gpus.
Please note that each additional compute capability significantly increases your build time and binary size.
[Default is: "3.5,5.2"]: 
Extracting Bazel installation...
.......
INFO: Starting clean (this may take a while). Consider using --expunge_async if the clean takes more than several minutes.
.......
INFO: All external dependencies fetched successfully.
Configuration finished
```


### Bazel build with CUDA

```
bazel build -c opt --config=cuda //tensorflow/tools/pip_package:build_pip_package
```

Builds ok, but how to set config=cuda in our build? Tried that:

```
#does not work:
bazel run -c opt --config=cuda //HelloWorldSubscriber:HelloWorldSubscriber
```


### Build & Install Pip Package

Not tried yet.
