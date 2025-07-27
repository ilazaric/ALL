```
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install 3.1.70
./emsdk activate 3.1.70
# add to .bashrc
```

```
git clone https://github.com/raysan5/raylib
cd raylib
git checkout 5.5
```

```
emcmake cmake -B build -DPLATFORM=Web -DGRAPHICS=GRAPHICS_API_OPENGL_ES3 -DCMAKE_BUILD_TYPE=Release
```
