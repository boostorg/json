# Fuzzing

Boost json has support for fuzzing. Clang/libFuzzer is used.

## Building and running the fuzzers
Execute the fuzzing/fuzz.sh script. You need clang++ installed. The fuzzer script will start fuzzing for a limited time, interrupt it if you wish.

There are several fuzzers, to exercise different parts of the api, following the usage examples in the documentation.

## Running fuzzing
Either modify the fuzz.sh script, or run it to build the fuzzer and then issue:
```sh
mkdir -p out
./fuzzer_basic_parser out
```

## Rerunning old crashes
Given a test case testcase.json, build the fuzzer and execute it with the test file:
```sh
./fuzzer_basic_parser testcase.json
```

