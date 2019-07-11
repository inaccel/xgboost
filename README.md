<a href="https://www.inaccel.com/">
<p align="center">
<img src="https://www.inaccel.com/wp-content/uploads/logo-horizontal1200px.png" width=60% height=60% align="middle" alt="InAccel"/>
</p>
</a>

# XGBoost Exact Updater IP core


This is an FPGA accelerated solution for the XGBoost algorithm. It can provide up to **30x** speedup compared to a single threaded execution and up to **5x** compared to an 8 threaded Intel Xeon CPU execution respectively.

## Specifications

|   Entries   |   Nodes    |
| :---------: | :--------: |
| up to 65536 | up to 2048 |

## Supported Platforms

|            Board            |
| :-------------------------: |
|   [AWS VU9P (F1 instances)](https://aws.amazon.com/ec2/instance-types/f1/)   |
| Any other Xilinx platform with at least the same amount of VU9P resources |

## Design Files

-   The XGBoost library is located in the library directory. Accelerator kernel files are located under the kernel directory.
-   The Makefiles will help you generate the _.so_ library file and accelerator _.xclbin_ files.

A listing of all the files in this repository is shown below:

    - library/
    	- xgboost/
            - XGBoost repository
        - inaccel/
            - patch/
                - Makefile.patch
                - gbtree.cc.patch
                - gbtree.h.patch
            - inaccel/
                - updater_fpga.cc
    - kernel/
        - Makefile
        - srcs/
            - xgboost_exact_0.cpp
            - xgboost_exact_1.cpp
        - build/
        - bitstream/
    - benchmark/

## Preparation

**!** Before invoking any of the Makefile targets make sure you have sourced Xilinx **XRT** setup script.  
**!** Make sure you have set **XILINX_SDX** environment variable pointing to the SDx installation directory.

As far as the **platform** (or board) is concerned, Makefile uses **AWS_PLATFORM** environment variable as the target platform for the kernels compilation. If you are running this on AWS make sure AWS_PLATFORM environment variable is present and points to the platform DSA files<sup>1</sup>. Otherwise you can set Makefile `PLATFORM` variable to point to your platform DSA files.

1.  To obtain the AWS platform DSA files make sure you have cloned the aws-fpga github repository

Download train letters train dataset to data directory. Navigate to data directory and execute the following commands:

``` bash
	wget https://s3.amazonaws.com/inaccel-demo/data/nist/letters_csv_train.dat
	wget https://s3.amazonaws.com/inaccel-demo/data/nist/letters_csv_test.dat
```
This repository contains submodules, so do not forget to:
``` bash
    git submodule update --init --recursive
```

## Compiling the kernels

To compile the kernels for hardware target you just need to go to the kernel directory and execute `make`.

## Creating an AFI

To create an AFI based on the generated bitstream go to the kernel directory and execute:
``` bash
    make BUCKET=<YOUR BUCKET> upload
```

**!** Before creating an AFI you have to [setup your AWS credentials](https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-configure.html). 

## Compiling the XGBoost library

###  Preparation
1.  [Install devtoolset-7](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-7/)
``` bash
    sudo yum install centos-release-scl
    sudo yum-config-manager --enable rhel-server-rhscl-7-rpms
    sudo yum install devtoolset-7
```
2.  Check the version
``` bash
    scl enable devtoolset-7 "g++ -v"
```
3. Install InAccel Coral-API
``` bash
    curl -sL https://jfrog.inaccel.com/artifactory/generic/packages/inaccel.repo | \
    sudo tee /etc/yum.repos.d/inaccel.repo
    sudo yum install coral-api
```
To compile the kernels for hardware target enter the library directory and execute `./build_lib.sh`.
The script first patches the XGBoost library source files and then compiles the library.

## Running the benchmarks with InAccel Coral

<a href="https://www.inaccel.com/coral-fpga-resource-manager/">
<p align="center">
<img src="https://www.inaccel.com/wp-content/uploads/coral_logo_big-1-e1561553344239.png" width=60% height=60% align="middle" alt="InAccel Coral"/>
</p>
</a>

The above example application spawns a single thread and can train a model using a single FPGA device which **is not viable for datacenter-scale needs**. Data scientists rely on frameworks like Scikit Learn and Apache Spark to create and test their machine learning pipelines.  
**InAccel Coral** FPGA resource manager is able to automatically **scale** and **schedule** any acceleration requests to a **cluster of FPGAs**, perform **load balancing** techniques, **reconfigure** the FPGA devices, perform **memory management** etc., yet providing a simple to use **high level API** in Java, CPP and Python.  
We have also ready-to-use **integrations** with broadly used open source frameworks like Apache Spark to seamlessly accelerate your pipelines.  
Finally, shaping cutting edge technology, Coral is fully compatible with **Kubernetes** and using InAccel's device plugin you can set up a Kubernetes cluster aware of hardware accelerated resources or take advantage of **Serverless architecture** and provide acclerated serverless solutions to your own customers.

* You can **create a free InAccel Coral license** [here](https://www.inaccel.com/license/).
* You can **download** InAccel Coral docker from [dockerhub](https://hub.docker.com/r/inaccel/coral).
* You can find **full documentation** as well as a **quick starting guide** in [InAccel Docs](https://docs.inaccel.com/latest/).

