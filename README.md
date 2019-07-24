<a href="https://www.inaccel.com/">
<p align="center">
<img src="https://www.inaccel.com/wp-content/uploads/logo-horizontal1200px.png" width=60% height=60% align="middle" alt="InAccel"/>
</p>
</a>

# XGBoost Exact Updater IP core


This is an FPGA accelerated solution for the XGBoost algorithm. It can provide up to **30x** speedup compared to a single threaded 
execution and up to **5x** compared to an 8 threaded Intel Xeon CPU execution respectively.

The acceleration is attained by exposing parallelism and reusing data in the _features_ dimension of the dataset. Due to this, 
to attain speedup, the dataset should have a large number of features.

Another limitation is that the accelerator needs to read the inputs in dense form, and does not perform well for very sparse datasets,
since it has to read and skip missing values.

## Specifications

The IP core that is provided accelerates Split Calculations for the Exact (Greedy) algorithm  for tree creation. A new tree method 
is also provided in the library, called _fpga\_exact_ that uses our updater and the pruner. To be able to achieve a speedup, some 
limitations were placed. The maximum training sample size is limited to 65536 entries, and the tree depth is also limited. At 
every tree level, the accelerator can calculate up to 2048 new nodes. That means the theoretical maximum depth is 11, but due to pruning, 
the actual number of nodes to be calculated is reduced. You should set the depth to the value you want and if the training fails with 
_"More than 2048 new nodes were requested. Please reduce max depth"_, set it to a lower value.

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
            - .keep
        - benchmarks.py
        - sdaccel.ini
    - kernel/
        - Makefile
        - srcs/
            - xgboost_exact_0.cpp
            - xgboost_exact_1.cpp
        - build/
            - .keep
        - bitstream/
            - bitstream.json
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
**!** In case of the following error [click this link](https://github.com/dmlc/xgboost/issues/4684).
```
fatal: reference is not a tree: e1c8056f6a0ee1c42fd00430b74176e67db66a9f
Unable to checkout 'e1c8056f6a0ee1c42fd00430b74176e67db66a9f' in submodule path 'rabit'
```


https://github.com/dmlc/xgboost/issues/4684

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

The Coral version uses the InAccel [Coral FPGA resource manager](https://www.inaccel.com/coral-fpga-resource-manager/) to decouple 
the library and the FPGAs, thus allowing scalability to as many FPGAs as available.

In this version you can change the number of requests (each request is a call to a kernel), which will be sent to the 
resource manager, depending on the number of FPGAs available. The bitstream we provide contains two kernels, 
so the optimal number of requests should be 2 or 4 depending on the dataset. For more FPGAs, scale it accordingly.

**!** If you have already performed the Standalone patch, reverse it with `./build_lib.sh reverse-standalone`.

The Coral version of the exact updater uses the InAccel Coral-API to comunicate with our FPGA resource manager.

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
* SVHN

Download the datasets you are interrested in:

``` bash
#Cifar10
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/cifar10.bz2
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/cifar10.t.bz2
#SVHN
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/SVHN.bz2
wget -P data/ https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/SVHN.t.bz2
```

For testing purposes, a synthetic regression dataset (SyntheticR) and a synthetic classification dataset (SyntheticCl) are also used.

The Benchmarks are written in Python 3.6, and need the following libraries: numpy, pandas, sklearn.

1.  Install Python 3.6
```bash
sudo yum install python36 python36-libs python36-devel python36-pip
python36 --version
```
2.  Install the needed libraries
```bash
sudo pip3.6 install numpy pandas sklearn
```

If you want to use the Coral version, to run the benchmarks the **InAccel Coral** FPGA resource manager must be installed and running.

* You can **create a free InAccel Coral license** [here](https://www.inaccel.com/license/).
* You can **download** InAccel Coral docker from [dockerhub](https://hub.docker.com/r/inaccel/coral).
* You can find **full documentation** as well as a **quick starting guide** in [InAccel Docs](https://docs.inaccel.com/latest/).

If you want to use the Standalone version, the updater uses the environmental variable *BITSTREAM* to read the bitstream file. 
In this version the XGBoost library is dinamically linked to the Xilinx OpenCL library, so you have to source the Xilinx XRT setup script.
You also usually need to have admin rights to have access to the FPGA.

```bash
sudo su
source /opt/xilinx/xrt/setup.sh
export BITSTREAM=../kernel/bitstream/xgboost_exact.hw.awsxclbin
```

To run the benchmarks execute:

```bash
export PYTHONPATH=../library/xgboost/python-package
python36 benchmarks.py
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
The reported times correspond to the default number of 5 training iterations.
These measurements were taken on 24 July 2019.

*  Coral Version

Cifar10 and SVHN Datasets with tree depth set to 20
```bash
python36 benchmarks.py -d Cifar10,SVHN -D 20
```

| Dataset | updater | Time(s)     | Accuracy | SpeedUp |
|---------|---------|-------------|----------|---------|
| Cifar10 | cpu     |  874.762506 | 0.401200 |         |
|         | fpga    |  168.310018 | 0.407100 | 5.20    |
| SVHN    | cpu     | 1051.139699 | 0.595037 |         |
|         | fpga    |  220.590023 | 0.594077 | 4.77    |

Synthetic Dataset Runs for different feature sizes
Synthetic Regression
```bash
python36 benchmarks.py -d SyntheticR -f 64 128 256 512 1024 2048
```

| features | updater | Time(s) | RMSE   | SpeedUp |
|----------|---------|---------|--------|---------|
|     64   |   cpu   |  1.2496 | 141.20 |         |
|          |   fpga  |  1.2888 | 191.15 | 0.97    |
|    128   |   cpu   |  2.8395 |  96.08 |         |
|          |   fpga  |  1.8535 | 122.48 | 1.53    |
|    256   |   cpu   |  5.6620 | 142.94 |         |
|          |   fpga  |  2.8624 | 202.42 | 1.98    |
|    512   |   cpu   | 10.9949 | 133.82 |         |
|          |   fpga  |  4.4565 | 183.40 | 2.47    |
|   1024   |   cpu   | 21.7405 | 136.49 |         |
|          |   fpga  |  7.6572 | 182.85 | 2.84    |
|   2048   |   cpu   | 43.6106 | 137.58 |         |
|          |   fpga  | 14.6405 | 190.68 | 2.98    |

Synthetic Multiclass Classification, 5 classes
```bash
python36 benchmarks.py -d SyntheticCl -f 64 128 256 512 1024 2048
```

| features | updater | Time(s) | Accuracy | SpeedUp |
|----------|---------|---------|----------|---------|
|     64   |   cpu   |   5.811 |  0.8589  |         |
|          |   fpga  |   5.072 |  0.8577  | 1.15    |
|    128   |   cpu   |  13.491 |  0.8529  |         |
|          |   fpga  |   6.102 |  0.8523  | 2.21    |
|    256   |   cpu   |  26.110 |  0.7634  |         |
|          |   fpga  |   8.869 |  0.7651  | 2.94    |
|    512   |   cpu   |  50.824 |  0.8068  |         |
|          |   fpga  |  13.487 |  0.8063  | 3.77    |
|   1024   |   cpu   | 100.869 |  0.7792  |         |
|          |   fpga  |  23.264 |  0.7805  | 4.34    |
|   2048   |   cpu   | 197.331 |  0.8428  |         |
|          |   fpga  |  42.774 |  0.8418  | 4.61    |

*  Standalone Version

Cifar10 and SVHN Datasets with tree depth set to 20
```bash
python36 benchmarks.py -d Cifar10,SVHN -D 20
```

| Dataset | updater | Time(s)     | Accuracy | SpeedUp |
|---------|---------|-------------|----------|---------|
| Cifar10 | cpu     |  845.308292 | 0.401200 |         |
|         | fpga    |  164.102713 | 0.407100 | 5.15    |
| SVHN    | cpu     | 1041.125136 | 0.595037 |         |
|         | fpga    |  216.056923 | 0.594077 | 4.82    |

Synthetic Dataset Runs for different feature sizes
Synthetic Regression
```bash
python36 benchmarks.py -d SyntheticR -f 64 128 256 512 1024 2048
```

| features | updater | Time(s) | RMSE   | SpeedUp |
|----------|---------|---------|--------|---------|
|     64   |   cpu   |  1.3279 |  92.52 |         |
|          |   fpga  |  3.3281 | 119.94 | 0.40    |
|    128   |   cpu   |  2.7711 |  96.08 |         |
|          |   fpga  |  3.7153 | 122.48 | 0.75    |
|    256   |   cpu   |  5.5509 | 142.93 |         |
|          |   fpga  |  4.5977 | 199.48 | 1.21    |
|    512   |   cpu   | 10.7886 | 133.82 |         |
|          |   fpga  |  6.1949 | 183.90 | 1.74    |
|   1024   |   cpu   | 21.4205 | 136.49 |         |
|          |   fpga  |  9.4584 | 185.55 | 2.26    |
|   2048   |   cpu   | 42.8734 | 137.58 |         |
|          |   fpga  | 15.5623 | 191.10 | 2.75    |

Synthetic Multiclass Classification, 5 classes
```bash
python36 benchmarks.py -d SyntheticCl -f 64 128 256 512 1024 2048
```

| features | updater | Time(s) | Accuracy | SpeedUp |
|----------|---------|---------|----------|---------|
|     64   |   cpu   |   8.237 |  0.8718  |         |
|          |   fpga  |   6.100 |  0.8721  | 1.35    |
|    128   |   cpu   |  14.480 |  0.8359  |         |
|          |   fpga  |   6.906 |  0.8340  | 2.10    |
|    256   |   cpu   |  25.519 |  0.8182  |         |
|          |   fpga  |   9.259 |  0.8174  | 2.76    |
|    512   |   cpu   |  49.714 |  0.8334  |         |
|          |   fpga  |  14.115 |  0.8346  | 3.52    |
|   1024   |   cpu   |  99.051 |  0.8565  |         |
|          |   fpga  |  23.554 |  0.8572  | 4.21    |
|   2048   |   cpu   | 196.171 |  0.8518  |         |
|          |   fpga  |  43.064 |  0.8508  | 4.56    |
