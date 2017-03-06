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

## Build & run the example code

This will take a long time the first time you build the TensorFlow libraries.

    $ bazel run //HelloWorldSubscriber:HelloWorldSubscriber

