g++ -g test.cpp src/archive.cpp src/asset.cpp -pthread -lcryptopp -I include -I ../cryptpp -L ../cryptpp -o main && \
g++ -g src/keygen.cpp -lcryptopp -I include -I ../cryptpp -L ../cryptpp -o keygen
