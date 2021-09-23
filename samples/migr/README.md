

The "migr" sample project
==============

This sample demonstrates running WebAssembly process migration of embedding WAMR:
- initialize runtime
- restore process from sample/basic at 500 of program counter
- export native functions to the WASM apps
- wasm function calls native function and pass arguments
- deinitialize runtime

Build this sample
==============
Execute the ```build.sh``` script then all binaries including wasm application files would be generated in 'out' directory.

```
$ ./build.sh
```

Run the sample
==========================

Execute the ```run.sh``` script in ```samples/migr``` folder.
```
$ ./run.sh
calling wasm_runtime_restore
restore
failed to link import func (env, main)
calling into WASM function: generate_float
end generate_float: 102009.921875
Native finished calling wasm function generate_float(), returned a float value: 102009.921875f
```




