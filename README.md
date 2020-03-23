# Launch an AWS FPGA Developer AMI

## Setup Xilinx FPGA Runtime

```sh
git clone https://github.com/aws/aws-fpga.git
source aws-fpga/vitis_runtime_setup.sh
source aws-fpga/vitis_setup.sh
```

## Install Development Toolset 6

> ! to build XGBoost

```sh
sudo yum install -y centos-release-scl
sudo yum-config-manager --enable rhel-server-rhscl-7-rpms
sudo yum install -y devtoolset-6
```

## Install Docker

> ! To automate deployment, InAccel Coral is shipped in a docker container.

```sh
sudo yum install -y device-mapper-persistent-data lvm2 yum-utils
sudo yum-config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo
sudo yum install -y containerd.io docker-ce docker-ce-cli
sudo usermod -aG docker ${USER}
sudo systemctl enable docker
sudo systemctl start docker
```

> ! Log out and log back in so that your group membership is re-evaluated.

## Install Git 2

> ! https://github.com/dmlc/xgboost/issues/4684

```sh
sudo yum remove -y git
sudo yum install -y https://centos7.iuscommunity.org/ius-release.rpm
sudo yum install -y git2u-all
```

## Install InAccel & C++ Coral API

```sh
curl -sS https://setup.inaccel.com/repo | sh
sudo yum install -y inaccel coral-api
sudo systemctl restart docker
```

### Config InAccel Coral license (https://inaccel.com/license)
```sh
inaccel config license ...
```

### Start InAccel Coral

```sh
inaccel coral start --fpgas=xilinx
```

## Clone inaccel/xgboost

```sh
git clone --branch coral --recursive --single-branch https://github.com/inaccel/xgboost.git && cd xgboost
```

### Enable InAccel Coral integration

```sh
make patch clean all
```

## Fetch AWS-VU9P-F1 bitstream

```sh
inaccel bitstream install https://store.inaccel.com/artifactory/bitstreams/xilinx/aws-vu9p-f1/dynamic_5.0/com/inaccel/xgboost/0.1/2exact
```

# Run the benchmarks

## Fetch the datasets

```sh
curl https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/cifar10.bz2 --create-dirs -o data/cifar10.bz2
curl https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/cifar10.t.bz2 --create-dirs -o data/cifar10.t.bz2
curl https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/SVHN.bz2 --create-dirs -o data/SVHN.bz2
curl https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/multiclass/SVHN.t.bz2 --create-dirs -o data/SVHN.t.bz2
```

## Install Python 3 & required libraries

```sh
sudo yum install -y python3-pip

sudo pip3 install numpy pandas sklearn
```

## Examples

```sh
export PYTHONPATH=${PWD}/xgboost/python-package
```

Cifar10 & SVHN datasets:

```sh
python3 benchmarks.py -d Cifar10,SVHN -D 20
```

Synthetic Regression dataset:

```sh
python3 benchmarks.py -d SyntheticR -f 64 128 256 512 1024 2048
```

Synthetic Classification dataset:

```bash
python3 benchmarks.py -d SyntheticCl -f 64 128 256 512 1024 2048
```
