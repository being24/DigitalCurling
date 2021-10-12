docker pull ubuntu
docker run -it ubuntu /bin/bash

# aptの更新
apt update
apt upgrade -y

# パッケージのインストール
apt install wget
apt install build-essential 
apt install libssl-dev
apt install git
apt install curl

curl https://getmic.ro | bash


# boostのインストール（1.77）
wget https://sourceforge.net/projects/boost/files/boost/1.77.0/boost_1_77_0.tar.gz
tar -xf ./boost_1_77_0.tar.gz
mv ./boost_1_77_0 /opt/
cd /opt/boost_1_77_0
sh ./bootstrap.sh 
./b2 install -j2 -j $(grep cpu.cores /proc/cpuinfo | sort -u | awk '{split($0, ary, ": "); print(ary[2] + 1)}' )


# boostのパスを設定
# export BOOST_ROOT=$(whereis boost | awk '{n=split($0, ary, ": "); print(ary[2])}')
export BOOST_ROOT="/opt/boost_1_77_0"

# cmakeのビルド(>= 3.19)
wget https://github.com/Kitware/CMake/releases/download/v3.21.3/cmake-3.21.3.tar.gz
tar xvf cmake-3.21.3.tar.gz
cd cmake-3.21.3
./configure
make
make install
export PATH="/usr/local/bin:$PATH"
cmake

# リポジトリのclone
git clone --recursive https://github.com/being24/DigitalCurling.git
git submodule update --init --recursive

# ブランチの切り替え
git checkout develop

# デジタルカーリングのビルド
cd ./DigitalCurling
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBOX2D_BUILD_TESTBED=OFF ..
cmake --build . --config RelWithDebInfo
