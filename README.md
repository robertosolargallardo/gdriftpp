# README #

The gdrift++ platform is a fully distributed system designed for performing approximate bayesian computation for parameter inference and model selection in population-genetic models. The gdrift++ platform is composed by a set of RESTful services that communicate each other by exchanging JavaScript Object Notation(JSON) documents via HTTP requests.

# Installation #

install dependecies
-------------------
restbed
```
git clone --recursive https://github.com/corvusoft/restbed.git
mkdir restbed/build
cd restbed/build
cmake [-DBUILD_TESTS=YES] [-DBUILD_EXAMPLES=YES] [-DBUILD_SSL=NO] [-DBUILD_SHARED=YES] [-DCMAKE_INSTALL_PREFIX=/output-directory] ..
make [-j CPU_CORES+1] install
make test
```
