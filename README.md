# README #

The gdrift++ platform is a fully distributed system designed for performing approximate bayesian computation for parameter inference and model selection in population-genetic models. The gdrift++ platform is composed by a set of RESTful services that communicate each other by exchanging JavaScript Object Notation(JSON) documents via HTTP requests.

# Installation #

install dependecies
-------------------
```
git clone --recursive https://github.com/corvusoft/restbed.git
mkdir restbed/build
cd restbed/build
cmake [-DBUILD_TESTS=YES] [-DBUILD_EXAMPLES=YES] [-DBUILD_SSL=NO] [-DBUILD_SHARED=YES] [-DCMAKE_INSTALL_PREFIX=/output-directory] ..
make [-j CPU_CORES+1] install
make test
```

```
git clone https://github.com/mongodb/libbson.git
cd libbson/build
cmake ..
sudo make install
```

```
git clone https://github.com/mongodb/mongo-c-driver.git
cd mongo-c-driver
sh autogen.sh
./configure --disable-automatic-init-and-cleanup
make LDFLAGS=-pthread
sudo make install
```

```
curl -OL https://github.com/mongodb/mongo-cxx-driver/archive/r3.1.1.tar.gz 
tar -xzf r3.1.1.tar.gz
cd mongo-cxx-driver-r3.1.1/build
sudo cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
sudo make install
```

```
git clone https://github.com/robertosolargallardo/libgdrift.git
mkdir libgdrift/build
cd libgdrift/build
cmake ..
make
sudo make install
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.:/usr/local/lib
```
