<a href="https://www.inaccel.com/">
<p align="center">
<img src="https://www.inaccel.com/wp-content/uploads/logo-horizontal1200px.png" width=60% height=60% align="middle" alt="InAccel"/>
</p>
</a>

# XGBoost Exact Updater IP core


This is an FPGA accelerated solution for the XGBoost algorithm. It can provide up to **30x** speedup compared to a single threaded 
execution and up to **5x** compared to an 8 threaded Intel Xeon CPU execution respectively.

## Specifications

The IP core that is provided accelerates Split Calculations for the Exact (Greedy) algorithm  for tree creation. A new tree method 
is also provided in the library, called _fpga\_exact_ that uses our updater and the pruner. To be able to accieve a speedup, some 
limitations were placed. The maximum training sample size is limited to 65536 entries, and the tree depth is also limited. At 
every tree level, the accelerator can calculate up to 2048 new nodes. That means the theoretical maximum depth is 11. 

|   Entries   |         Nodes        |
| :---------: | :------------------: |
| up to 65536 | up to 2048 per level |

## Supported Platforms

|            Board            |
| :-------------------------: |
|      [Xilinx Alveo U200](https://www.xilinx.com/products/boards-and-kits/alveo.html)      |
|      [Xilinx Alveo U250](https://www.xilinx.com/products/boards-and-kits/alveo.html)      |
|   [AWS VU9P (F1 instances)](https://aws.amazon.com/ec2/instance-types/f1/)   |
| Any other Xilinx platform with at least the same amount of VU9P resources |

## Design Files

-   The XGBoost library is located in the library directory.
-   Accelerator kernel files are located in the kernel directory.
-   A demo application to benchmark the accelerator is located in the benchmarks directory.
-   The Makefiles and scripts will help you generate the _.so_ library file and accelerator _.xclbin_ files.

A listing of all the files in this repository is shown below:

    - benchmarks/
        - data/
        - benchmarks.py
        - sdaccel.ini
    - kernel/
        - Makefile
        - srcs/
            - xgboost_exact_0.cpp
            - xgboost_exact_1.cpp
        - build/
        - bitstream/
            -bitstream.json
    - library/
        - xgboost/
            - XGBoost repository
        - src/
            - patch/
                - gbtree.cc.patch
                - gbtree.h.patch
                - Makefile.coral.patch
                - Makefile.patch
            - inaccel/
                - INcl.cc
                - INcl.h
                - runtime.cc
                - runtime.h
                - runtime-api.cc
                - runtime-api.h
                - updater_fpga.cc
                - updater_fpga_coral.cc
        - build_lib.sh
    - LICENSE
    - README.md

## Compiling the bitstream

Enter the _kernels/_ directory.

**!** Before invoking any of the Makefile targets make sure you have sourced Xilinx **XRT** setup script.  
**!** Make sure you have set **XILINX_SDX** environment variable pointing to the SDx installation directory.

As far as the **platform** (or board) is concerned, the makefile uses **AWS_PLATFORM** environment variable 
as the target platform for the kernels compilation. If you are running this on AWS make sure the environment 
variable *AWS_PLATFORM* is present and points to the platform DSA files. Otherwise you can set Makefile 
`PLATFORM` variable to point to your platform DSA files. To obtain the AWS platform DSA files make sure you 
have cloned the aws-fpga github repository.

To compile the kernels for hardware target you just need to execute `make`.

### Creating an AFI (AWS only)

**!** Before creating an AFI you have to [setup your AWS credentials](https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-configure.html). 

If you are in an Amazon AWS f1 instance, you will have to create an AFI to use the generated bitstream.
To create an AFI based on the generated bitstream execute:
``` bash
make BUCKET=<YOUR BUCKET> upload
```

## Deploying the AFI/Bitstream (Coral only)

To deploy your AFI/Bitstream you first have to:

1.  [Install the **InAccel Coral** FPGA resource manager](https://docs.inaccel.com/latest/inaccel/install/rpm/)

2.  [Setup your Environment](https://docs.inaccel.com/latest/tutorial/setup/)

Then, using the provided _bitstream.json_, execute `inaccel bitstream install bitstream/`.

To learn how to create your own _bitstream.json_, [read our guide](https://docs.inaccel.com/latest/tutorial/bitstreams/).

If you want to avoid having to compile a bitstream and/or create an AFI we provide prebuilt bitstreams for the following platforms:

-   AWS VU9P: 
-   Xilinx Alveo U200:
-   Xilinx Alveo U250:

To use the prebuilt bistreams simply execute `inaccel bitstream install <REPO PATH>`

## Compiling the XGBoost library

Enter the _library/_ directory.

###  Preparation

This repository contains submodules, so do not forget to execute:
``` bash
git submodule update --init --recursive
```

In our experience, the XGBoost library fails to build with g++ 4.8.5 (Red Hat default) and g++ 5.3.1 (devtoolset-4), 
so we used g++ 6.3.1 (devtoolset-6). If you use the Coral version, you can probably use any g++ >= 6. 
If you use the non-Coral version you have to use g++ 6.3.1 due to linking with the Xilinx Opencl library (libxilinxopencl.so).

-   [Install devtoolset-6](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-6/)
``` bash
sudo yum install centos-release-scl
sudo yum-config-manager --enable rhel-server-rhscl-7-rpms
sudo yum install devtoolset-6
```
-   Check the version
``` bash
scl enable devtoolset-6 "g++ -v"
```

A script is provided in the library directory that patches the XGBoost library, adding the new updater.

###  Standalone Version

The standalone version uses a single FPGA with two kernels, and does not need the **InAccel Coral** FPGA resource manager.
This version does not scale to more FPGAs or kernels.

**!** If you have already performed the Coral patch, reverse it with `./build_lib.sh reverse-coral`.

First patch the XGBoost library with the Standalone patch with `./build_lib.sh patch-standalone`.

Then build the library with `./build_lib.sh`.

###  Coral Version

The Coral version uses the InAccel [Coral FPGA resource manager](https://www.inaccel.com/coral-fpga-resource-manager/) to decouple the library and the FPGAs, thus allowing scalability to as many FPGAs as available.
In this version basic heuristics are performed to find the optimal number of requests (each request is a call to a kernel), which the resource manager schedules to all available FPGAs and kernels.

**!** If you have already performed the Standalone patch, reverse it with `./build_lib.sh reverse-standalone`.

The Coral version of the exact updater uses the InAccel Coral-API to comunicate with our FPGA resource manager

Install InAccel Coral-API
``` bash
curl -sL https://jfrog.inaccel.com/artifactory/generic/packages/inaccel.repo | \
sudo tee /etc/yum.repos.d/inaccel.repo
sudo yum install coral-api
```

First patch the XGBoost library with the Coral patch with `./build_lib.sh patch-coral`.

Then build the library with `./build_lib.sh`.

## Running the benchmarks

<a href="https://www.inaccel.com/coral-fpga-resource-manager/">
<p align="center">
<img src="https://www.inaccel.com/wp-content/uploads/coral_logo_big-1-e1561553344239.png" width=60% height=60% align="middle" alt="InAccel Coral"/>
</p>
</a>

Enter the _benchmarks/_ directory.

**!** To obtain the AWS platform DSA files make sure you have cloned the aws-fpga github repository

The benchmarks support the following datasets in libsvm format:

* Cifar10
* MNIST
* Higgs
* YearPredictionMSD
* Cover Type
* Airline

Download the datasets you are interrested in:

``` bash
#cifar10
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/cifar10.bz2
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/cifar10.t.bz2
#mnist
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/mnist.bz2
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/mnist.t.bz2
#HIGGS
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/binary/HIGGS.bz2
#YearPredictionMSD
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/regression/YearPredictionMSD.bz2
#Cover Type
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/binary/covtype.libsvm.binary.bz2
```

The Benchmarks are written in Python 3.6, and need the following libraries: numpy, pandas, sklearn.

To have access to Python 3.6, we used the rh-python36 Software Collection:

1.  [Install Python 3.6](https://www.softwarecollections.org/en/scls/rhscl/rh-python36/)
```bash
sudo yum install centos-release-scl
sudo yum-config-manager --enable rhel-server-rhscl-7-rpms
sudo yum install rh-python36
scl enable rh-python36 "python --version"
```
2.  Install the needed libraries
```bash
sudo scl enable rh-python36 bash
pip install numpy
pip install pandas
pip install sklearn
```

If you want to use the Coral version, to run the benchmarks the **InAccel Coral** FPGA resource manager must be installed and running.

* You can **create a free InAccel Coral license** [here](https://www.inaccel.com/license/).
* You can **download** InAccel Coral docker from [dockerhub](https://hub.docker.com/r/inaccel/coral).
* You can find **full documentation** as well as a **quick starting guide** in [InAccel Docs](https://docs.inaccel.com/latest/).

```bash
scl enable rh-python36 bash
```

If you want to use the Standalone version, the updater uses the environmental variable *BITSTREAM_PATH* to read the bitstream file. 
In this version the XGBoost library is dinamically linked to the Xilinx OpenCL library, so you have to add it to the LD_LIBRARY_PATH.
You also usually need to have admin rights to have access to the FPGA
```bash
sudo scl enable rh-python36 bash
export BITSTREAM_PATH=../kernel/bitstream/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$XILINX_XRT/lib
```

To run the benchmarks execute:
```bash
export PYTHONPATH=../library/xgboost/python-package
python benchmarks.py
```
Benchmark parameters:
```bash
usage: benchmarks.py [-h] [--num_rounds NUM_ROUNDS] [--datasets DATASETS]
                     [--verbosity VERBOSITY] [--nthreads NTHREADS]

optional arguments:
  -h, --help            show this help message and exit
  --num_rounds NUM_ROUNDS
                        Boosting rounds. (default: 5)
  --datasets DATASETS   Datasets to run. Must be included in the Default
                        (default:
                        Cifar10,MNIST,Higgs,YearPredictionMSD,Synthetic,Cover
                        Type,Airline)
  --verbosity VERBOSITY
                        XGBoost verbosity parameter. (default: 0)
  --nthreads NTHREADS   Number of threads to use. (default: None)

```

## Example results
Run on 19 June 2019

*  Coral Version

| Dataset           | updater | Time(s) | Accuracy |  RMSE  |  SpeedUp |
|-------------------|---------|---------|----------|--------|----------|
| Cifar10           |   cpu   | 408.419 |   0.413  |        |          |
|                   |   fpga  | 88.4338 |  0.4148  |        |  4.61835 |
| MNIST             |   cpu   | 23.1758 |   0.933  |        |          |
|                   |   fpga  | 22.0973 |  0.9333  |        |  1.04881 |
| Higgs             |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| YearPredictionMSD |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| Cover Type        |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| Airline           |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| SyntheticR        |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| SyntheticCl       |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |

*  Standalone Version

| Dataset           | updater | Time(s) | Accuracy |  RMSE  |  SpeedUp |
|-------------------|---------|---------|----------|--------|----------|
| Cifar10           |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| MNIST             |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| Higgs             |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| YearPredictionMSD |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| Cover Type        |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| Airline           |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| SyntheticR        |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
| SyntheticCl       |   cpu   | 1.00000 |  0.0000  | 0.0000 |          |
|                   |   fpga  | 1.00000 |  0.0000  | 0.0000 | 1.000000 |
