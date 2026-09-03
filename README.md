# Grim Dawn Mod Dll Hook
This winmm.dll is a hook for Grim Dawn mods.  
It forwards all winmm.dll functions to the original one.

Used e.g. in [Riftgate Companion](https://github.com/elmodor/grimdawn-riftgatecompanion) and [Item Assistant Linux](https://github.com/elmodor/iagd).

## Building
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel $(nproc)
```

## Using
Copy build/winmm.dll to the x64 directory inside Grim Dawns install directory.

### Linux
If using the Linux compatibility mode, copy build/winmm.dll also to the compat directoy inside Grim Dawns install directory.  
Set `WINEDLLOVERRIDES="winmm=n,b" %command%` as launch option.

## winmm.def
To generate the winmm.def you need ´mingw-w64-tools´ and a windows winmm.dll. Then you can do:  
`gendef windows/system32/winmm.dll`
